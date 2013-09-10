#include "entity.h"
#include "misc.h"
#include "kvstore.h"
#include "fpumath.h"

CoreType Entity::s_Type( &Entity::Super::s_Type );


Entity::Entity() : m_level(NULL), m_world(NULL), m_meshDef(NULL)
{
}

Entity::~Entity()
{
	assert( m_level == NULL );
}

void Entity::Spawn( KVStore const &kv )
{
	if ( kv.HasKey( "mesh" ) )
	{
		m_meshDef = AnimMeshDefManager()->Get( kv.GetKeyValueString( "mesh" ) );
		init_animmeshinstance( m_meshInstance, *m_meshDef->GetMesh() );
		//load_animmesh( m_mesh, kv.GetKeyValueString( "mesh" ), 1.f/8.f );
	}
	SetName( kv.GetKeyValueString( "name" ) );

	mtxIdentity( m_wmtx );
	kv.GetKeyValueFloatArray( &m_wmtx[12], 3, "pos" );
	kv.GetKeyValueFloatArray( m_pos, 3, "pos" );
}

void Entity::Clear()
{
}

void Entity::OnSetWorld( World *w )
{
	assert( m_world == NULL );
	m_world = w;
}

void Entity::OnAddToLevel( Level *l )
{
	assert( m_level == NULL );
	m_level = l;
}

void Entity::OnRemoveFromLevel( Level *l )
{
	assert( m_level == l );
	m_level = NULL;
}

bool Entity::Use( Entity *by )
{
	return false;
}

void Entity::Attack( Entity *by )
{
}

void Entity::UpdateFixed( float dt )
{
}

void Entity::UpdateDelta( float dt )
{
}

void Entity::UpdateGUI()
{
}

void Entity::Render()
{
	renderMesh( *m_meshDef->GetMesh(), m_meshInstance, m_wmtx );
}
