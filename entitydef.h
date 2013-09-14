#pragma once

#include "kvstore.h"
#include "misc.h"


class EntityDef : public KVStore
{
	typedef KVStore Super;
	std::string m_filename;
public:

	virtual void Reload();

};

class EntityDefMgr : public TypedDefMgr<EntityDef,true>
{

public:

};

EntityDefMgr *EntityDefManager();

