#include "inventory.h"
#include "misc.h"


void InventoryItemDef::Reload()
{
}

void InventoryItemDef::Clear()
{
}




InventoryItemDefMgr *InventoryItemDefManager()
{
	static InventoryItemDefMgr s_mgr;
	return &s_mgr;
}


void Inventory::Give( const InventoryItemDef *def, float quantity )
{
	for (unsigned int i=0; i<m_items.size(); i++)
	{
		if ( m_items[i].def == def )
		{
			m_items[i].quantity += quantity;
			break;
		}
	}
	m_items.push_back( InventoryItem() );
	m_items.back().def = def;
	m_items.back().quantity = quantity;
}

bool Inventory::CanTake( const InventoryItemDef *def, float quantity ) const
{
	for (unsigned int i=0; i<m_items.size(); i++)
	{
		if ( m_items[i].def == def )
		{
			return m_items[i].quantity >= quantity;
		}
	}
	return false;
}

void Inventory::Take( const InventoryItemDef *def, float quantity )
{
	for (unsigned int i=0; i<m_items.size(); i++)
	{
		if ( m_items[i].def == def )
		{
			m_items[i].quantity = Max( 0.f, m_items[i].quantity-quantity );
			if ( m_items[i].quantity <= 0.f )
			{
				m_items.erase( m_items.begin() + i );
			}
			break;
		}
	}
}

InventoryItem const &Inventory::GetItem( int index )
{
	return m_items[index];
}
