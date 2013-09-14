#pragma once
#include <string>
#include <vector>
#include "kvstore.h"
#include "misc.h"

class InventoryItemDef : public KVStore
{
	std::string m_guiName;
public:

	virtual void Reload();
	virtual void Clear();
};


class InventoryItemDefMgr : public TypedDefMgr<InventoryItemDef,true>
{

public:

};

InventoryItemDefMgr *InventoryItemDefManager();

// ------------------------

class InventoryItem
{
	std::string m_type;
	float quatity;
};

class Inventory
{
	std::vector< InventoryItem > m_items;
public:
};
