#include "entitydef.h"

bool EntityDef::Load( const char *filename )
{
	m_filename = filename;
	return Super::Load( filename );
}

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
