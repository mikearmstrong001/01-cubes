#include "craft.h"


CraftRecipeDefMgr *CraftRecipeDefManager()
{
	static CraftRecipeDefMgr s_mgr;
	return &s_mgr;
}
