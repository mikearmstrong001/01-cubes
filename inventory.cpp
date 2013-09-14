#include "inventory.h"




InventoryItemDefMgr *InventoryItemDefManager()
{
	static InventoryItemDefMgr s_mgr;
	return &s_mgr;
}
