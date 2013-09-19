#include "particle.h"
#include "fpumath.h"
#include "misc.h"
#include <map>
#include <string>
#include <bgfx.h>
#include "level.h"

static bgfx::VertexDecl s_PosColorDecl;

void initParticleDecl()
{
	// Create vertex stream declaration.
	s_PosColorDecl.begin();
	s_PosColorDecl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
	s_PosColorDecl.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
	s_PosColorDecl.end();
}

static PosColorVertex s_cubeVertices[8] =
{
	{ 0.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{ 0.0f,  0.0f,  1.0f, 0xff00ff00 },
	{ 1.0f,  0.0f,  1.0f, 0xff00ffff },
	{ 0.0f,  1.0f,  0.0f, 0xffff0000 },
	{ 1.0f,  1.0f,  0.0f, 0xffff00ff },
	{ 0.0f,  0.0f,  0.0f, 0xffffff00 },
	{ 1.0f,  0.0f,  0.0f, 0xffffffff },
};

static const uint16_t s_cubeIndices[36] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};


static bool loadGraph( particleGraph_s &graph, int numElements, FILE *f )
{
	char line[2048];
	fgets( line, sizeof(line), f );
	const char *cursor = line;
	int num;
	ParseInt( num, cursor );

	graph.numElements = numElements;
	graph.data.reserve( num * (numElements+1) );

	for (int i=0; i<num; i++)
	{
		float v;
		ParseFloat( v, cursor );
		graph.data.push_back( v );
		for (int j=0; j<numElements; j++)
		{
			ParseFloat( v, cursor );
			graph.data.push_back( v );
		}
	}
	return true;
}

static void sampleGraph( float *o, const particleGraph_s &graph, float t )
{
	unsigned int index = 0;
	unsigned int entries = graph.data.size() / (graph.numElements+1);
	while ( index < entries && t > graph.data[ index*(graph.numElements+1) ] )
		index++;
	if ( index == entries )
	{
		memcpy( o, &graph.data[(index-1)*(graph.numElements+1)+1], sizeof(float) * graph.numElements );
	}
	else
	if ( index == 0 )
	{
		memcpy( o, &graph.data[(0)*(graph.numElements+1)+1], sizeof(float) * graph.numElements );
	}
	else
	{
		const float *cur = &graph.data[ (index-1)*(graph.numElements+1) ];
		const float *next = &graph.data[ (index)*(graph.numElements+1) ];
		float dt = (t-cur[0]) / (next[0] - cur[0]);
		float omdt = 1.f - dt;
		for (int i=0; i<graph.numElements; i++)
			o[i] = (omdt*cur[i+1]) + (dt*next[i+1]);
	}
}

static bool loadRange( particleIniter_s &range, FILE *f )
{
	char line[1024];
	fgets( line, sizeof(line), f );
	int res = sscanf( line, "{ %f %f } { %f %f } { %f %f }\n", &range.range[0][0], &range.range[1][0], 
		&range.range[0][1], &range.range[1][1], 
		&range.range[0][2], &range.range[1][2] );

	return res > 0;
}

bool ParticleDef::Load( const char *filename )
{
	bool ok = true;
	FILE *f = fopen( filename, "rb" );
	if ( f )
	{
		ok &= (fscanf( f, "%d %d %f\n", &maxParticles, &totalParticles, &spawnRate ) == 3);

		ok &= loadRange( posInit, f );
		ok &= loadRange( velInit, f );
		ok &= loadRange( accInit, f );
		ok &= loadRange( lifeInit, f );
		ok &= loadRange( emitterLifeInit, f );
		ok &= loadGraph( sizeByLife, 1, f ); 
		ok &= loadGraph( orbitRadiusByLife, 1, f ); 
		ok &= loadGraph( orbitAngleByLife, 1, f ); 
	}

	return ok;
}

void ParticleDef::Reload()
{
}

void ParticleDef::Clear()
{
}

static float particleRand( float mn, float mx )
{
	float f = rand() / (float)RAND_MAX;
	return mn + f * (mx - mn);
}

static inline float saturate( float d )
{
	return d < 0.f ? 0.f : d > 1.f ? 1.f : d;
}

static void initFromRange( float *dst, const particleIniter_s &range, int numEle )
{
	for (int i=0; i<numEle; i++)
	{
		dst[i] = particleRand( range.range[0][i], range.range[1][i] );
	}
}

void initParticleSystem( particleSystem_s &ps, const char *filename )
{
	ps.def = ParticleDefManager()->Get(filename);
	ps.spawnErr = 0.f;
	ps.numParticles = 0;
	ps.totalParticles = 0;
	initFromRange( &ps.emitterLife, ps.def->emitterLifeInit, 1 );
	ps.pos.resize( ps.def->maxParticles*3 );
	ps.vel.resize( ps.def->maxParticles*3 );
	ps.acc.resize( ps.def->maxParticles*3 );
	ps.life.resize( ps.def->maxParticles*1 );
	ps.startLife.resize( ps.def->maxParticles*1 );
}


static bool updateParticleSystem( particleSystem_s &ps, float dt )
{
	if ( ps.def == NULL )
		return false;

	ps.spawnErr += dt * ps.def->spawnRate;

	if ( ps.emitterLife > 0.f )
	{
		while ( ps.spawnErr >= 1.f )
		{
			ps.spawnErr -= 1.f;
			if ( ps.numParticles < ps.def->maxParticles && (ps.totalParticles < ps.def->totalParticles || ps.def->totalParticles == 0 ) )
			{
				initFromRange( &ps.pos[ps.numParticles*3], ps.def->posInit, 3 );
				initFromRange( &ps.vel[ps.numParticles*3], ps.def->velInit, 3 );
				initFromRange( &ps.acc[ps.numParticles*3], ps.def->accInit, 3 );
				initFromRange( &ps.life[ps.numParticles*1], ps.def->lifeInit, 1 );
				ps.startLife[ps.numParticles*1] = ps.life[ps.numParticles*1];
				ps.numParticles++;
				ps.totalParticles++;
			}
		}
		ps.emitterLife -= dt;
	}

	for (int i=0; i<ps.numParticles;)
	{
		ps.life[i] -= dt;
		if ( ps.life[i] > 0.f )
		{
			vec3Mad( &ps.pos[i*3], &ps.vel[i*3], dt );
			vec3Mad( &ps.vel[i*3], &ps.acc[i*3], dt );
			i++;
		}
		else
		{
			ps.numParticles--;
			memcpy( &ps.pos[i*3], &ps.pos[ps.numParticles*3], sizeof(float)*3 );
			memcpy( &ps.vel[i*3], &ps.vel[ps.numParticles*3], sizeof(float)*3 );
			memcpy( &ps.acc[i*3], &ps.acc[ps.numParticles*3], sizeof(float)*3 );
			memcpy( &ps.life[i*1], &ps.life[ps.numParticles*1], sizeof(float)*1 );
			memcpy( &ps.startLife[i*1], &ps.startLife[ps.numParticles*1], sizeof(float)*1 );
		}
	}
	return (ps.numParticles > 0) || (ps.totalParticles == 0);
}

void renderParticleSystem( particleSystem_s &ps )
{
	if ( ps.def == NULL )
		return;

	if ( bgfx::checkAvailTransientBuffers( ps.numParticles*8, s_PosColorDecl, ps.numParticles*36 ) )
	{
		bgfx::TransientIndexBuffer tib;
		bgfx::allocTransientIndexBuffer( &tib, ps.numParticles * 36 );

		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer( &tvb, ps.numParticles * 8, s_PosColorDecl );

		uint16_t *indices = (uint16_t*)tib.data;
		PosColorVertex *verts = (PosColorVertex *)tvb.data;

		for ( int i=0; i<ps.numParticles; i++ )
		{
			const float *pos = &ps.pos[i*3];
			const float *vel = &ps.vel[i*3];
			float dt = saturate( 1.f - (ps.life[i*1] / ps.startLife[i*1]) );
			float size, orbitRadius, orbitAngle;
			sampleGraph( &size, ps.def->sizeByLife, dt );
			sampleGraph( &orbitAngle, ps.def->orbitAngleByLife, dt );
			sampleGraph( &orbitRadius, ps.def->orbitRadiusByLife, dt );

			float orbitPos[3];
			float velMagSq = vec3Dot( vel, vel );
			if ( orbitRadius > 0.f &&  velMagSq > 0.f )
			{
				float rmtx[16];
				float nvel[3];
				vec3Norm( nvel, vel );
				mtxRotateAxisAngle( rmtx, nvel[0], nvel[1], nvel[2], orbitAngle );

				float perp[3];
				vec3Perp( perp, nvel );
				perp[0] *= orbitRadius;
				perp[1] *= orbitRadius;
				perp[2] *= orbitRadius;

				vec3MulMtx( orbitPos, perp, rmtx );

				// rotate around vel axis not up !
				orbitPos[0] += pos[0];//sinf( orbitAngle );
				orbitPos[1] += pos[1];//cosf( orbitAngle );
				orbitPos[2] += pos[2];
				pos = orbitPos;
			}

			for (int j=0; j<8; j++)
			{
				verts->m_x = pos[0] + s_cubeVertices[j].m_x * size;
				verts->m_y = pos[1] + s_cubeVertices[j].m_y * size;
				verts->m_z = pos[2] + s_cubeVertices[j].m_z * size;
				verts->m_abgr = s_cubeVertices[j].m_abgr;
				verts++;
			}
			int baseVert = i * 8;
			for (int j=0; j<36; j++)
			{
				*indices++ = (uint16_t)(s_cubeIndices[j] + baseVert);
			}
		}

		bgfx::setVertexBuffer(&tvb);
		bgfx::setIndexBuffer(&tib);

		// Set render states.
		bgfx::setState(0
				| BGFX_STATE_RGB_WRITE
				| BGFX_STATE_ALPHA_WRITE
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_CULL_CW
				| BGFX_STATE_MSAA
				);

		// Submit primitive for rendering to view 0.
		bgfx::submit(0);
	}
}


ParticleDefMgr *ParticleDefManager()
{
	static ParticleDefMgr s_mgr;
	return &s_mgr;
}


static ClassCreator<ParticleEntity> s_PlayerCreator( "Particle" );
CoreType ParticleEntity::s_Type( &ParticleEntity::Super::s_Type );


void ParticleEntity::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel( l );

	l->AddRender( this );
	l->AddDelta( this );
}

void ParticleEntity::OnRemoveFromLevel( Level *l )
{
	l->RemoveDelta( this );
	l->RemoveRender( this );

	Super::OnRemoveFromLevel( l );
}

void ParticleEntity::Spawn( KVStore const &kv )
{
	Super::Spawn( kv );

	initParticleSystem( m_psys, kv.GetKeyValueString( "particle" ) );
}

void ParticleEntity::UpdateDelta( float dt )
{
	Super::UpdateDelta(dt);

	if ( !updateParticleSystem( m_psys, dt ) )
	{
		m_level->RemoveEntity( this );
	}
}

void ParticleEntity::Render()
{
	bgfx::setTransform(m_wmtx);
	renderParticleSystem( m_psys );
}
