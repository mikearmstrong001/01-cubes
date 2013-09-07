#pragma once

#include "kvstore.h"
#include "misc.h"


class EntityDef : public KVStore
{
	typedef KVStore Super;
	std::string m_filename;
public:

	virtual bool Load( const char *filename );
	virtual void Reload();
};

class EntityDefMgr : public TypedDefMgr<EntityDef>
{
public:
};

EntityDefMgr *EntityDefManager();
