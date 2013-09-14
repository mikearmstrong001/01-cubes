#include "entitydef.h"

void EntityDef::Reload()
{
	Clear();
	Super::Load( m_filename.c_str() );
}


EntityDefMgr *EntityDefManager()
{
	static EntityDefMgr s_def;
	return &s_def;
}
