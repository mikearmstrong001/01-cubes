#include "mesh.h"
#include "fpumath.h"
#include "voxel.h"
#include "misc.h"
#include "kv6.h"
#include "cub.h"

static bgfx::VertexDecl s_PosColorDecl;

void initMeshDecl()
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

static int s_cubeNormal[6][3] =
{
	{  0,  0,  1 },
	{  0,  0, -1 },
	{ -1,  0,  0 },
	{  1,  0,  0 },
	{  0,  1,  0 },
	{  0, -1,  0 },
};


void makeSubset( MeshSubSet *subset, std::vector<PosColorVertex> const &verts, std::vector<unsigned short> const &indices )
{
	subset->memVerts = bgfx::alloc( verts.size() * sizeof(PosColorVertex) );
	subset->memIndices = bgfx::alloc( indices.size() * sizeof(unsigned short) );

	memcpy( subset->memVerts->data, &verts[0], verts.size() * sizeof(PosColorVertex) );
	memcpy( subset->memIndices->data, &indices[0], indices.size() * sizeof(unsigned short) );

	subset->vbh = bgfx::createVertexBuffer(subset->memVerts, s_PosColorDecl);
	subset->ibh = bgfx::createIndexBuffer(subset->memIndices);
}

void makeMesh( Mesh *mesh, map_s *map, float scale, int *from, int *to )
{
	int zero[3] = { 0, 0, 0 };
	int *f = from ? from : zero;
	int *t = to ? to : map->dim;

	for (unsigned int i=0; i<mesh->m_subsets.size(); i++)
	{
		bgfx::destroyIndexBuffer( mesh->m_subsets[i].ibh );
		bgfx::destroyVertexBuffer( mesh->m_subsets[i].vbh );
	}
	mesh->m_subsets.clear();

	std::vector<PosColorVertex> verts;
	std::vector<unsigned short> indices;
	for (int x=f[0]; x < t[0]; ++x)
	{
		for (int y=f[1]; y < t[1]; ++y) 
		{
			for (int z=f[2]; z < t[2]; ++z) 
			{
				int index = (x*map->dim[2]*map->dim[1])+(y*map->dim[2]) + z;
				if ( !map->solid[index] )
					continue;

				if ( verts.size() > 20000 )
				{
					MeshSubSet subset;
					makeSubset( &subset, verts, indices );
					mesh->m_subsets.push_back( subset );
					verts.clear();
					indices.clear();
				}

				int baseVertex = verts.size();

				for (int v=0; v<(sizeof(s_cubeVertices)/sizeof(s_cubeVertices[0])); v++)
				{
					PosColorVertex vtx = s_cubeVertices[v];
					vtx.m_x += (x - map->ofs[0]);
					vtx.m_y += (y - map->ofs[1]);
					vtx.m_z += (z - map->ofs[2]);
					vtx.m_x *= scale;
					vtx.m_y *= scale;
					vtx.m_z *= scale;
					vtx.m_abgr = map->colour[index];
					verts.push_back( vtx );
				}
				for (int v=0; v<(sizeof(s_cubeIndices)/sizeof(s_cubeIndices[0])); v+=3)
				{
					int i0 = s_cubeIndices[v+0];
					int i1 = s_cubeIndices[v+1];
					int i2 = s_cubeIndices[v+2];
				//	if ( (v/6) > 1 )
				//		continue;
					if ( issolid( map, x+s_cubeNormal[v/6][0], y+s_cubeNormal[v/6][1], z+s_cubeNormal[v/6][2] ) )
						continue;
					indices.push_back( (unsigned short)(i0 + baseVertex) );
					indices.push_back( (unsigned short)(i1 + baseVertex) );
					indices.push_back( (unsigned short)(i2 + baseVertex) );
				}
			}
		}
	}
	if ( indices.size() )
	{
		MeshSubSet subset;
		makeSubset( &subset, verts, indices );
		mesh->m_subsets.push_back( subset );
	}
}

void renderMesh( Mesh &mesh )
{
	for (uint32_t s=0; s<mesh.m_subsets.size(); s++)
	{
		// Set vertex and index buffer.
		bgfx::setVertexBuffer(mesh.m_subsets[s].vbh);
		bgfx::setIndexBuffer(mesh.m_subsets[s].ibh);

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

void initMapMesh( MapMesh &mapMesh )
{
	for (int x=0; x<512; x+=(MAPXDIM))
		for (int y=0; y<512; y+=(MAPYDIM))
			for (int z=0; z<64; z+=(MAPZDIM))
				mapMesh.dirty[x/MAPXDIM][y/MAPYDIM][z/MAPZDIM] = true;
}

void makeMapMesh( MapMesh &mapMesh, map_s *map, float scale )
{
	for (int x=0; x<512; x+=(MAPXDIM))
	{
		for (int y=0; y<512; y+=(MAPYDIM))
		{
			for (int z=0; z<64; z+=(MAPZDIM))
			{
				if ( !mapMesh.dirty[x/MAPXDIM][y/MAPYDIM][z/MAPZDIM] )
					continue;
				int f[3] = {x,y,z};
				int t[3] = {x+MAPXDIM,y+MAPYDIM,z+MAPZDIM};
				makeMesh( &mapMesh.meshes[x/MAPXDIM][y/MAPYDIM][z/MAPZDIM], map, scale, f, t );
				mapMesh.dirty[x/MAPXDIM][y/MAPYDIM][z/MAPZDIM] = false;
			}
		}
	}
}

void renderMesh( MapMesh &mapMesh )
{
	for (int x=0; x<512; x+=(MAPXDIM))
	{
		for (int y=0; y<512; y+=(MAPYDIM))
		{
			for (int z=0; z<64; z+=(MAPZDIM))
			{
				renderMesh( mapMesh.meshes[x/MAPXDIM][y/MAPYDIM][z/MAPZDIM] );
			}
		}
	}
}

void clearMap( map_s *m, MapMesh &mapMesh, int f[3], int t[3] )
{
	for (int x=f[0]; x<=t[0]; x++)
	{
		for (int y=f[1]; y<=t[1]; y++)
		{
			for (int z=f[2]; z<=t[2]; z++)
			{
				setgeom( m, x, y, z, 0 );
				mapMesh.dirty[x/MAPXDIM][y/MAPYDIM][z/MAPZDIM] = true;
			}
		}
	}
}



void addMesh( AnimMesh &amesh, Mesh *mesh, const char *name, float ox, float oy, float oz, float rx, float ry, float rz )
{
	AnimSubMesh am;
#if 0
	mtxIdentity( am.animmtx );
	am.animmtx[12] = ox;
	am.animmtx[13] = oy;
	am.animmtx[14] = oz;
	am.animrot[0] = rx;
	am.animrot[1] = ry;
	am.animrot[2] = rz;
	am.animofs[0] = ox;
	am.animofs[1] = oy;
	am.animofs[2] = oz;
#endif
	am.baserot[0] = rx;
	am.baserot[1] = ry;
	am.baserot[2] = rz;
	am.baseofs[0] = ox;
	am.baseofs[1] = oy;
	am.baseofs[2] = oz;
	am.mesh = mesh;
	am.name = name;
	amesh.m_submeshs.push_back( am );
}

void init_animmeshinstance( AnimMeshInstance &inst, const AnimMesh &amesh )
{
	inst.m_submeshPose.resize( amesh.m_submeshs.size() );
	for (unsigned int i=0; i<inst.m_submeshPose.size(); i++)
	{
		AnimSubMeshJoint &aj = inst.m_submeshPose[i];
		const AnimSubMesh &am = amesh.m_submeshs[i];
		mtxIdentity( aj.animmtx );
		aj.animmtx[12] = am.baseofs[0];
		aj.animmtx[13] = am.baseofs[1];
		aj.animmtx[14] = am.baseofs[2];
		aj.animrot[0] = am.baserot[0];
		aj.animrot[1] = am.baserot[1];
		aj.animrot[2] = am.baserot[2];
		aj.animofs[0] = am.baseofs[0];
		aj.animofs[1] = am.baseofs[1];
		aj.animofs[2] = am.baseofs[2];
	}
}


void renderMesh( const AnimMesh &amesh, const AnimMeshInstance &inst, const float pos[3], float rotz )
{
	float wmtx[16];
	mtxRotateZ(wmtx, rotz );
	wmtx[12] = pos[0];
	wmtx[13] = pos[1];
	wmtx[14] = pos[2];
	for (unsigned int i=0; i<amesh.m_submeshs.size(); i++)
	{
		const AnimSubMesh &am = amesh.m_submeshs[i];
		if ( am.mesh == NULL )
			continue;
		const AnimSubMeshJoint &aj = inst.m_submeshPose[i];
		float fmtx[16];
		mtxMul(fmtx,aj.animmtx,wmtx);
		// Set model matrix for rendering.
		bgfx::setTransform(fmtx);

		renderMesh( *am.mesh );
	}
}

void renderMesh( const AnimMesh &amesh, const AnimMeshInstance &inst, const float wmtx[16] )
{
	for (unsigned int i=0; i<amesh.m_submeshs.size(); i++)
	{
		const AnimSubMesh &am = amesh.m_submeshs[i];
		if ( am.mesh == NULL )
			continue;
		const AnimSubMeshJoint &aj = inst.m_submeshPose[i];
		float fmtx[16];
		mtxMul(fmtx,aj.animmtx,wmtx);
		// Set model matrix for rendering.
		bgfx::setTransform(fmtx);

		renderMesh( *am.mesh );
	}
}

void calcAttachMtx( float fmtx[16], AnimMeshInstance &inst, int index, float ofs[3], float rot[3], float pos[3], float rotz )
{
	float wmtx[16];
	mtxRotateZ(wmtx, rotz );
	wmtx[12] = pos[0];
	wmtx[13] = pos[1];
	wmtx[14] = pos[2];

	float amtx[16];
	mtxRotateXYZ(amtx, rot[0], rot[1], rot[2] );
	amtx[12] = ofs[0];
	amtx[13] = ofs[1];
	amtx[14] = ofs[2];

	AnimSubMeshJoint &aj = inst.m_submeshPose[index];
	float omtx[16];
	mtxMul(omtx,amtx,aj.animmtx);
	mtxMul(fmtx,omtx,wmtx);
}

void calcAttachMtx( float fmtx[16], AnimMeshInstance &inst, int index, float ofs[3], float rot[3], float wmtx[3] )
{
	float amtx[16];
	mtxRotateXYZ(amtx, rot[0], rot[1], rot[2] );
	amtx[12] = ofs[0];
	amtx[13] = ofs[1];
	amtx[14] = ofs[2];

	AnimSubMeshJoint &aj = inst.m_submeshPose[index];
	float omtx[16];
	mtxMul(omtx,amtx,aj.animmtx);
	mtxMul(fmtx,omtx,wmtx);
}

void calcAttachPos( float fpos[16], AnimMeshInstance &inst, int index, float ofs[3], float rot[3], float pos[3], float rotz )
{
	float wmtx[16];
	mtxRotateZ(wmtx, rotz );
	wmtx[12] = pos[0];
	wmtx[13] = pos[1];
	wmtx[14] = pos[2];

	float amtx[16];
	mtxRotateXYZ(amtx, rot[0], rot[1], rot[2] );
	amtx[12] = ofs[0];
	amtx[13] = ofs[1];
	amtx[14] = ofs[2];

	AnimSubMeshJoint &aj = inst.m_submeshPose[index];
	float omtx[16];
	mtxMul(omtx,amtx,aj.animmtx);
	float fmtx[16];
	mtxMul(fmtx,omtx,wmtx);
	fpos[0] = fmtx[12];
	fpos[1] = fmtx[13];
	fpos[2] = fmtx[14];
}


static int toChannelIndex( const std::string &ch )
{
	if ( ch == "tx" )
		return AKI_OFSX;
	if ( ch == "ty" )
		return AKI_OFSY;
	if ( ch == "tz" )
		return AKI_OFSZ;
	if ( ch == "rx" )
		return AKI_ROTX;
	if ( ch == "ry" )
		return AKI_ROTY;
	if ( ch == "rz" )
		return AKI_ROTZ;
	return -1;
}

static int findmesh( AnimMesh &mesh, const char *name )
{
	for (unsigned int i=0; i<mesh.m_submeshs.size(); i++)
	{
		AnimSubMesh &sm = mesh.m_submeshs[i];
		if ( sm.name == name )
			return i;
	}
	return -1;
}

void load_kv6( Mesh &mesh, const char *filename, float scale )
{
	unsigned int len;
	void *mem = fload( filename, &len );
	map_s vox;
	load_kv6( &vox, (uint8_t*)mem, (int)len );
	makeMesh( &mesh, &vox, scale );
}

void load_cub( Mesh &mesh, const char *filename, float px, float py, float pz, float scale )
{
	unsigned int len;
	void *mem = fload( filename, &len );
	map_s vox;
	load_cub( &vox, (uint8_t*)mem, (int)len );
	vox.ofs[0] = px * vox.dim[0];
	vox.ofs[1] = py * vox.dim[1];
	vox.ofs[2] = pz * vox.dim[2];
	makeMesh( &mesh, &vox, scale );
}

void load_animmesh( AnimMesh &mesh, const char *filename, float globalScale )
{
	static std::map< std::string, Mesh * > g_meshes;

	mesh.m_submeshs.clear();
	mesh.m_anims.clear();
	mesh.m_anims.push_back( Anim() );
	mesh.m_attachments.clear();
	mesh.m_skeleton.clear();
	char curline[2048];
	FILE *f = fopen( filename, "rb" );
	while ( fgets( curline, sizeof(curline)-1, f ) )
	{
		char *line = skipws( curline );
		char file[256];
		char name[32];
		char meshname[128];
		float animTime;
		int   looping;
		float px, py, pz;
		float ox, oy, oz, rx, ry, rz;
		float scale;
		if ( strstr( line, "startskeleton" ) )
		{
			while ( fgets( curline, sizeof(curline)-1, f ) && strstr( line, "endskeleton" ) == NULL )
			{
				char *line = skipws( curline );
				char child[128], parent[128];
				if ( sscanf( line, "%s %s", &child, &parent ) == 2 )
				{
					AnimSkeleton as;
					as.child = findmesh( mesh, child );
					as.parent = findmesh( mesh, parent );
					mesh.m_skeleton.push_back(as);
				}
			}
		}
		else
		if ( strstr( line, "startmesh" ) )
		{
			while ( fgets( curline, sizeof(curline)-1, f ) && strstr( curline, "endmesh" ) == NULL )
			{
				char *line = skipws( curline );
				if ( sscanf( line, "cub %s %s %f %f %f %f %f %f %f %f %f %f\n", meshname, file, &px, &py, &pz, &ox, &oy, &oz, &rx, &ry, &rz, &scale ) == 12 )
				{
					char uid[256];
					sprintf( uid, "%s:%f%f%f%f", file, px,py,pz,scale );
					if ( g_meshes.find(uid) == g_meshes.end() )
					{
						Mesh *m = new Mesh;
						load_cub( *m ,file, px, py, pz, scale * globalScale );
						g_meshes[uid] = m;
					}
					addMesh( mesh, g_meshes[uid], meshname, ox * globalScale, oy * globalScale, oz * globalScale, rx, ry, rz );
				} else
				if ( sscanf( line, "kv6 %s %s %f %f %f %f %f %f %f\n", meshname, file, &ox, &oy, &oz, &rx, &ry, &rz, &scale ) == 9 )
				{
					char uid[256];
					sprintf( uid, "%s:%f", file, scale );
					if ( g_meshes.find(uid) == g_meshes.end() )
					{
						Mesh *m = new Mesh;
						load_kv6( *m ,file, scale * globalScale );
						g_meshes[uid] = m;
					}
					addMesh( mesh, g_meshes[uid], meshname, ox * globalScale, oy * globalScale, oz * globalScale, 0.f, 0.f, 0.f );
				} else
				if ( sscanf( line, "dummymesh %s %f %f %f %f %f %f %f\n", meshname, &ox, &oy, &oz, &rx, &ry, &rz ) == 7 )
				{
					addMesh( mesh, NULL, meshname, ox * globalScale, oy * globalScale, oz * globalScale, rx, ry, rz );
				}
			}
		} else
		if ( sscanf( line, "attach %s %s %f %f %f %f %f %f", name, meshname, &ox, &oy, &oz, &rx, &ry, &rz ) == 8 )
		{
			mesh.m_attachments.push_back( AnimMeshAttach() );
			AnimMeshAttach &a = mesh.m_attachments[ mesh.m_attachments.size()-1 ];
			a.name = name;
			a.idx = findmesh( mesh, meshname );
			a.ofs[0] = ox;
			a.ofs[1] = oy;
			a.ofs[2] = oz;
			a.rot[0] = rx;
			a.rot[1] = ry;
			a.rot[2] = rz;
		} else
		if ( sscanf( line, "startanim %s %f %d", name, &animTime, &looping ) == 3 )
		{
			mesh.m_anims.push_back( Anim() );
			Anim &anim = mesh.m_anims[mesh.m_anims.size()-1];
			anim.name = name;
			anim.time = animTime;
			anim.looping = looping;
			anim.channels.resize( mesh.m_submeshs.size() );
			while ( fgets( curline, sizeof(curline)-1, f ) )
			{
				char *line = skipws( curline );
				if ( strstr( line, "endanim" ) != 0 )
				{
					break;
				} else
				if ( strstr( line, "ch" ) != 0 )
				{
					strtok( line, " \t" );
					std::string meshName, chName;
					if ( ParseString( meshName ) && ParseString( chName ) )
					{
						int meshIndex = findmesh( mesh, meshName.c_str() );
						int chType = toChannelIndex( chName.c_str() );
						AnimChannel &ch = anim.channels[meshIndex];
						float keyTime, keyValue;
						while ( ParseFloat( keyTime ) && ParseFloat( keyValue ) )
						{
							AnimKey ak;
							ak.time = keyTime;
							ak.value = keyValue;
							if (chType < 3) 
								ak.value *= globalScale;
							ch.keys[chType].push_back( ak );
						}
					}
				}
			}
		}
	}
	fclose( f );

	{
		mesh.m_anims[0].looping = 0;
		mesh.m_anims[0].time = 1.f;
		mesh.m_anims[0].channels.resize(mesh.m_submeshs.size());
		for (unsigned int index=0; index<mesh.m_submeshs.size();index++)
		{
			mesh.m_anims[0].channels[index].keys[AKI_OFSX].resize(1);
			mesh.m_anims[0].channels[index].keys[AKI_OFSX][0].time = 0.f;
			mesh.m_anims[0].channels[index].keys[AKI_OFSX][0].value = mesh.m_submeshs[index].baseofs[0];
			mesh.m_anims[0].channels[index].keys[AKI_OFSY].resize(1);
			mesh.m_anims[0].channels[index].keys[AKI_OFSY][0].time = 0.f;
			mesh.m_anims[0].channels[index].keys[AKI_OFSY][0].value = mesh.m_submeshs[index].baseofs[1];
			mesh.m_anims[0].channels[index].keys[AKI_OFSZ].resize(1);
			mesh.m_anims[0].channels[index].keys[AKI_OFSZ][0].time = 0.f;
			mesh.m_anims[0].channels[index].keys[AKI_OFSZ][0].value = mesh.m_submeshs[index].baseofs[2];
			mesh.m_anims[0].channels[index].keys[AKI_ROTX].resize(1);
			mesh.m_anims[0].channels[index].keys[AKI_ROTX][0].time = 0.f;
			mesh.m_anims[0].channels[index].keys[AKI_ROTX][0].value = mesh.m_submeshs[index].baserot[0];
			mesh.m_anims[0].channels[index].keys[AKI_ROTY].resize(1);
			mesh.m_anims[0].channels[index].keys[AKI_ROTY][0].time = 0.f;
			mesh.m_anims[0].channels[index].keys[AKI_ROTY][0].value = mesh.m_submeshs[index].baserot[1];
			mesh.m_anims[0].channels[index].keys[AKI_ROTZ].resize(1);
			mesh.m_anims[0].channels[index].keys[AKI_ROTZ][0].time = 0.f;
			mesh.m_anims[0].channels[index].keys[AKI_ROTZ][0].value = mesh.m_submeshs[index].baserot[2];
		}
	}
}

bool AnimMeshDef::Load( const char *filename )
{
	m_filename = filename;
	load_animmesh( m_mesh, filename, 1.f/8.f );
	return true;
}

void AnimMeshDef::Reload()
{
	Load( m_filename.c_str() );
}

void AnimMeshDef::Clear()
{
}



AnimMeshDefMgr *AnimMeshDefManager()
{
	static AnimMeshDefMgr s_def;
	return &s_def;
}
