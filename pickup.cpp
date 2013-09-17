#include "pickup.h"
#include "level.h"
#include "fpumath.h"
#include "misc.h"
#include "player.h"
#include "entitydef.h"
#include "inventory.h"

static ClassCreator<Pickup> s_PickupCreator( "Pickup" );
CoreType Pickup::s_Type( &Pickup::Super::s_Type );


void Pickup::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel( l );

	l->AddDelta( this );
	l->AddRender( this );
}

void Pickup::OnRemoveFromLevel( Level *l )
{
	l->RemoveRender( this );
	l->RemoveDelta( this );

	Super::OnRemoveFromLevel( l );
}

void Pickup::Spawn( KVStore const &kv )
{
	Super::Spawn( kv );

	kv.GetKeyValueFloatArray( m_pos, 3, "pos" );
	m_rotz = kv.GetKeyValueFloat( "rotz", 0.f );
	m_rotRate = kv.GetKeyValueFloat( "rotrate", 1.f );
	m_bounceZ = kv.GetKeyValueFloat( "bouncez" );
	m_bounceZRate = kv.GetKeyValueFloat( "bouncezrate", 2.f );
	m_bounceZDist = kv.GetKeyValueFloat( "bouncezdist", 0.5f );

	mtxRotateZ( m_wmtx, m_rotz );
	m_wmtx[12] = m_pos[0];
	m_wmtx[13] = m_pos[1];
	m_wmtx[14] = m_pos[2] + sinf( 0.f ) * m_bounceZDist;
}

void Pickup::UpdateDelta( float dt )
{
	m_rotz += m_rotRate * dt;
	m_bounceZ += m_bounceZRate * dt;

	mtxRotateZ( m_wmtx, m_rotz );
	m_wmtx[12] = m_pos[0];
	m_wmtx[13] = m_pos[1];
	m_wmtx[14] = m_pos[2] + sinf( m_bounceZ ) * m_bounceZDist;
}

bool Pickup::Use( Entity *by )
{
	Player *p = CoreCast<Player>(by);
	if ( p )
	{
		Inventory &inv = p->GetInventory();
		const KVStore *kv = m_entityDef->GetKeyValueKeyValue( "give" );
		for (int i=0; i<kv->GetNumItems(); i++)
		{
			const KVStore *item = kv->GetIndexValueKeyValue( i );
			const InventoryItemDef *itemDef = InventoryItemDefManager()->Get( item->GetKeyValueString( "item", "unknown" ) );
			float amount = item->GetKeyValueInt( "amount", 1.f );
			if ( itemDef )
			{
				inv.Give( itemDef, amount );
			}
		}
		m_level->RemoveEntity( this );
		return true;
	}
	else
	{
		return false;
	}
}
