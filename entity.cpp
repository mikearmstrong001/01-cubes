#include "entity.h"
#include "misc.h"
#include "kvstore.h"
#include "fpumath.h"
#include "level.h"
#include "bgfx.h"

CoreType Entity::s_Type( &Entity::Super::s_Type );

extern bgfx::UniformHandle u_flash;
extern bgfx::TextureHandle g_whiteTexture;
extern bgfx::UniformHandle u_tex;

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

	m_attachName = kv.GetKeyValueString( "attach_name" );
	m_attachJoint = kv.GetKeyValueString( "attach_joint" );

	mtxIdentity( m_wmtx );
	mtxIdentity( m_attachOffsetMtx );
	kv.GetKeyValueFloatArray( &m_wmtx[12], 3, "pos" );
	kv.GetKeyValueFloatArray( m_pos, 3, "pos" );

	m_flashTime = 0.f;
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

void Entity::FinishUsing( Entity *by )
{
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

#include "gui.h"
void Entity::UpdateAttachment()
{
	if ( m_attachName.empty() )
		return;
	const Entity *attach = m_level->FindEntity( m_attachName.c_str() );
	if ( attach == NULL )
		return;
	float attachWMtx[16];
	attach->CalcAttachWorldMatrix( attachWMtx, m_attachJoint.c_str() );
	mtxMul(m_wmtx,m_attachOffsetMtx,attachWMtx);
}

void Entity::Render()
{
	int levelTime = m_level->GetTimeMS();
	float flash = 0.f;
	if ( m_flashTime > levelTime )
	{
		flash = 1.f;
	}
	bgfx::setUniform( u_flash, &flash, 1 );
	bgfx::setTexture(0, u_tex, g_whiteTexture);
	renderMesh( *m_meshDef->GetMesh(), m_meshInstance, m_wmtx );
}

void Entity::RenderGUI()
{
}

void Entity::CalcAttachWorldMatrix( float wmtx[16], const char *name ) const
{
	int index = findmeshattach( *m_meshDef->GetMesh(), name );
	if ( index == -1 )
	{
		memcpy( wmtx, m_wmtx, 16*sizeof(float) );
		return;
	}
	const AnimMeshAttach &attach = m_meshDef->GetMesh()->m_attachments[index];
	calcAttachMtx( wmtx, m_meshInstance, attach.idx, attach.ofs, attach.rot, m_wmtx );
	//const AnimSubMeshJoint &aj = m_meshInstance.m_submeshPose[index];
	//mtxMul(wmtx,aj.animmtx,m_wmtx);
}
