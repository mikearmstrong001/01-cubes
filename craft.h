#pragma once

#include "misc.h"
#include "kvstore.h"
#include <string>
#include <vector>

class CraftRecipeDef : public KVStore
{
public:

	virtual void Reload();
	virtual void Clear();
};


class CraftRecipeDefMgr : public TypedDefMgr<CraftRecipeDef,true>
{

public:

};

CraftRecipeDefMgr *CraftRecipeDefManager();
