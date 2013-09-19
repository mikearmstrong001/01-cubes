#pragma once
#include <string>
#include <vector>
#include "kvstore.h"
#include "misc.h"

class InventoryItemDef : public KVStore
{
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

struct InventoryItem
{
	const InventoryItemDef *def;
	float quantity;
};

class Inventory
{
	std::vector< InventoryItem > m_items;
public:

	void Give( const InventoryItemDef *def, float quantity );
	bool CanTake( const InventoryItemDef *def, float quantity ) const;
	void Take( const InventoryItemDef *def, float quantity );
	int  GetNumItems() const { return (int)m_items.size(); }
	InventoryItem const &GetItem( int index );
};
