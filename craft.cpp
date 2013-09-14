#include "craft.h"

void CraftRecipeDef::Reload()
{
}

void CraftRecipeDef::Clear()
{
}


CraftRecipeDefMgr *CraftRecipeDefManager()
{
	static CraftRecipeDefMgr s_mgr;
	return &s_mgr;
}
