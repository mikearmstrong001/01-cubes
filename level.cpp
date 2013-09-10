#include "level.h"
#include "entity.h"
#include <algorithm>
#include "entitydef.h"
#include "vxl.h"
#include "kvstore.h"
#include "fpumath.h"

struct sceneryobject_s
{
	float x, y, z;
};

static bool load_scenery( std::vector< sceneryobject_s > &scenery, const char *filename )
{
	scenery.clear();
	FILE *f = fopen( filename, "rb" );
	if ( f )
	{
		char line[2048];
		while ( fgets( line, sizeof(line), f ) )
		{
			float x, y, z;
			if ( sscanf( line, "%f %f %f\n", &x, &y, &z ) == 3 )
			{
				sceneryobject_s so;
				so.x = x;
				so.y = y;
				so.z = z;
				scenery.push_back( so );
			}
		}
		fclose( f );
	}
	return true;
}





Level::Level()
{
}

Level::~Level()
{
	Clear();
}

void Level::AddFixed( Entity* ent )
{
	if ( std::find( m_fixedUpdate.cbegin(), m_fixedUpdate.cend(), ent ) != m_fixedUpdate.cend() )
		return;
	m_fixedUpdate.push_back( ent );
	std::sort( m_fixedUpdate.begin(), m_fixedUpdate.end() );
}

void Level::AddDelta( Entity* ent )
{
	if ( std::find( m_deltaUpdate.cbegin(), m_deltaUpdate.cend(), ent ) != m_deltaUpdate.cend() )
		return;
	m_deltaUpdate.push_back( ent );
	std::sort( m_deltaUpdate.begin(), m_deltaUpdate.end() );
}

void Level::AddGui( Entity* ent )
{
	if ( std::find( m_guiUpdate.cbegin(), m_guiUpdate.cend(), ent ) != m_guiUpdate.cend() )
		return;
	m_guiUpdate.push_back( ent );
	std::sort( m_guiUpdate.begin(), m_guiUpdate.end() );
}

void Level::AddRender( Entity* ent )
{
	if ( std::find( m_renderUpdate.cbegin(), m_renderUpdate.cend(), ent ) != m_renderUpdate.cend() )
		return;
	m_renderUpdate.push_back( ent );
	std::sort( m_renderUpdate.begin(), m_renderUpdate.end() );
}

void Level::RemoveFixed( Entity* ent )
{
	UpdateContainer::iterator f = std::find( m_fixedUpdate.begin(), m_fixedUpdate.end(), ent );

	if ( f == m_fixedUpdate.end() )
		return;

	m_fixedUpdate.erase( f );
}

void Level::RemoveDelta( Entity* ent )
{
	UpdateContainer::iterator f = std::find( m_deltaUpdate.begin(), m_deltaUpdate.end(), ent );

	if ( f == m_deltaUpdate.end() )
		return;

	m_deltaUpdate.erase( f );
}

void Level::RemoveGui( Entity* ent )
{
	UpdateContainer::iterator f = std::find( m_guiUpdate.begin(), m_guiUpdate.end(), ent );

	if ( f == m_guiUpdate.end() )
		return;

	m_guiUpdate.erase( f );
}

void Level::RemoveRender( Entity* ent )
{
	UpdateContainer::iterator f = std::find( m_renderUpdate.begin(), m_renderUpdate.end(), ent );

	if ( f == m_renderUpdate.end() )
		return;

	m_renderUpdate.erase( f );
}


bool Level::Load( const char * filename )
{
	EntityDef const *def = EntityDefManager()->Get( filename );

	load_vxl_file( &m_map, def->GetKeyValueString( "vxl" ) );
	std::vector< sceneryobject_s > scenery;
	load_scenery( scenery, def->GetKeyValueString( "scenery" ) );

	const LevelEntsDef *ents = LevelEntsDefManager()->Get( def->GetKeyValueString( "ents" ) );

	for ( unsigned int i=0; i<ents->GetNum(); i++ )
	{
		KVStore const &ent = ents->Get( i );

		Entity *s = (Entity *)CoreClassCreator::Create( ent.GetKeyValueString( "classname" ) );
		s->Spawn( ent );
		s->OnAddToLevel( this );
		m_entities.push_back( s );
	}

	initMapMesh( m_mesh );
	//makeMapMesh( m_mesh, &m_map, 1.f );
#if 0
	FILE *f = fopen( "level1.ents", "wb" );
	{
		Entity *s = (Entity *)CoreClassCreator::Create( "Player" );
		KVStore kv;
		kv.AddKeyValueString( "classname", "Player" );
		kv.AddKeyValueString( "mesh", "dwarf.am" );
		kv.AddKeyValueString( "pos", "128 128 64" );
		kv.AddKeyValueString( "name", "player0" );
		s->Spawn( kv );
		s->OnAddToLevel( this );
		m_entities.push_back( s );

		std::string o;
		kv.Dump( o );
		fprintf( f, "{\n%s}\n", o.c_str() );
	}

	for (unsigned int i=0; i<50; i++)
	{
		Entity *s = (Entity *)CoreClassCreator::Create( "Zombie" );
		KVStore kv;
		kv.AddKeyValueString( "classname", "Zombie" );
		kv.AddKeyValueString( "mesh", "zombie.am" );
		char temp[512];
		sprintf( temp, "%d %d %d", rand()%512, rand()%512, 64 );
		kv.AddKeyValueString( "pos", temp );
		s->Spawn( kv );
		s->OnAddToLevel( this );
		m_entities.push_back( s );

		std::string o;
		kv.Dump( o );
		fprintf( f, "{\n%s}\n", o.c_str() );
	}

	for (unsigned int i=0; i<scenery.size(); i++)
	{
		Entity *s = (Entity *)CoreClassCreator::Create( "Scenery" );
		KVStore kv;
		kv.AddKeyValueString( "classname", "Scenery" );
		kv.AddKeyValueString( "mesh", "pine.am" );
		char temp[512];
		sprintf( temp, "%f %f %f", scenery[i].x, scenery[i].y, scenery[i].z );
		kv.AddKeyValueString( "pos", temp );
		s->Spawn( kv );
		s->OnAddToLevel( this );
		m_entities.push_back( s );

		std::string o;
		kv.Dump( o );
		fprintf( f, "{\n%s}\n", o.c_str() );
	}
	fclose( f );
#endif

	return true;
}

void Level::Clear()
{
	EntityContainer::iterator b = m_entities.begin();
	while ( b != m_entities.end() )
	{
		Entity *ent = *b;
		ent->OnRemoveFromLevel( this );
		b++;
	}

	b = m_entities.begin();
	while ( b != m_entities.end() )
	{
		Entity *ent = *b;
		ent->Clear();
		delete ent;
		b = m_entities.erase( b );
	}

	m_fixedUpdate.clear();
	m_deltaUpdate.clear();
	m_guiUpdate.clear();
	m_renderUpdate.clear();
}

void Level::UpdateBegin()
{
}

void Level::UpdateFixed( float dt )
{
	UpdateContainer::iterator b = m_fixedUpdate.begin();
	while ( b != m_fixedUpdate.end() )
	{
		Entity *ent = *b;
		ent->UpdateFixed( dt );
		b++;
	}
}

void Level::UpdateDelta( float dt )
{
	UpdateContainer::iterator b = m_deltaUpdate.begin();
	while ( b != m_deltaUpdate.end() )
	{
		Entity *ent = *b;
		ent->UpdateDelta( dt );
		b++;
	}
}

void Level::UpdateGUI()
{
	UpdateContainer::iterator b = m_guiUpdate.begin();
	while ( b != m_guiUpdate.end() )
	{
		Entity *ent = *b;
		ent->UpdateGUI();
		b++;
	}
}

void Level::UpdateEnd()
{
	RemoveContainer::iterator b = m_toRemove.begin();
	while ( b != m_toRemove.end() )
	{
		Entity *ent = *b;
		ent->OnRemoveFromLevel( this );
		EntityContainer::iterator f = std::find( m_entities.begin(), m_entities.end(), ent );
		if ( f != m_entities.end() )
		{
			m_entities.erase( f );
		}
		b++;
	}
	m_toRemove.clear();
}

void Level::Render()
{
	makeMapMesh( m_mesh, &m_map, 1.f );
	renderMesh( m_mesh );

	UpdateContainer::iterator b = m_renderUpdate.begin();
	while ( b != m_renderUpdate.end() )
	{
		Entity *ent = *b;
		ent->Render();
		b++;
	}
}

void Level::AddEntity( Entity *ent )
{
	m_entities.push_back( ent );
	ent->OnAddToLevel( this );
}

void Level::RemoveEntity( Entity *ent )
{
	m_toRemove.push_back( ent );
}

Entity *Level::FindEntity( const char *name )
{
	for ( unsigned int i=0; i<m_entities.size(); i++)
	{
		if ( strcmp( m_entities[i]->GetName().c_str(), name ) == 0 )
		{
			return m_entities[i];
		}
	}
	return NULL;
}

void Level::FindEntities( std::vector<Entity*> &ents, const float pos[3], float radius, CoreType const &type )
{
	for ( unsigned int i=0; i<m_entities.size(); i++)
	{
		Entity *e = m_entities[i];
		if ( IsTypeOf( e->Type(), &type ) )
		{
			const float *epos = e->GetPos();
			float distSq = vec3DistSq( pos, epos );
			if ( distSq < (radius *radius ) )
			{
				ents.push_back( e );
			}
		}
	}
}


static void ParseEnt( KVStore &kv, const char *&cursor )
{
	std::string token;
	ParseToken( token, cursor );
	while ( token != "}" )
	{
		std::string key;
		std::swap( key, token );
		std::string value;
		ParseToken( value, cursor );
		kv.AddKeyValueString( key.c_str(), value.c_str() );

		ParseToken( token, cursor );
	}

	const char *templatename = kv.GetKeyValueString( "template", NULL );
	if ( templatename )
	{
		const EntityDef *edef = EntityDefManager()->Get( templatename, false );
		kv.MergeInto( *edef );
	}
}

bool LevelEntsDef::Load( const char *filename )
{
	m_filename = filename;
	char* ents = (char*)fload(m_filename.c_str());
	if ( ents )
	{
		const char *cursor = ents;
		while ( cursor && *cursor )
		{
			std::string token;
			if ( !ParseToken( token, cursor ) )
				break;
			if ( token == "{" )
			{
				m_ents.push_back( new KVStore );
				ParseEnt( *m_ents.back(), cursor );
			}
		}
	}
	return true;
}

void LevelEntsDef::Reload()
{
	Clear();
	Load( m_filename.c_str() );
}

void LevelEntsDef::Clear()
{
	for (unsigned int i=0; i<m_ents.size(); i++)
		delete m_ents[i];
	m_ents.clear();
}


LevelEntsDefMgr *LevelEntsDefManager()
{
	static LevelEntsDefMgr s_mgr;
	return &s_mgr;
}
