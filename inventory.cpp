#include "inventory.h"


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
