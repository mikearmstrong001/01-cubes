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
	virtual void Parse( const char *&cursor );

};

class EntityDefMgr : public TypedDefMgr<EntityDef,true>
{

public:

	//virtual bool LoadGroup( const char *filename );
};

EntityDefMgr *EntityDefManager();

