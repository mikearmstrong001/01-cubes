#pragma once
#include <string>
#include <vector>
#include "misc.h"

class InventoryItemDef
{
	std::string m_guiName;
public:

	virtual bool Load( const char *filename );
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
