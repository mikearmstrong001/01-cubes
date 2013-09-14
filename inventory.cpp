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
