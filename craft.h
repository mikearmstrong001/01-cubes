#pragma once

#include "misc.h"
#include <string>
#include <vector>

struct CraftIngredient
{
	std::string item;
	float quatity;
};

class CraftRecipeDef
{
	std::vector<CraftIngredient> m_requirements;
	std::string m_item;

public:

	virtual bool Load( const char *filename );
	virtual void Reload();
	virtual void Clear();
};


class CraftRecipeDefMgr : public TypedDefMgr<CraftRecipeDef,true>
{

public:

};

CraftRecipeDefMgr *CraftRecipeDefManager();
