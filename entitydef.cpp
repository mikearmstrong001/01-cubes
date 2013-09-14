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

void EntityDef::Parse( const char *&cursor )
{
	std::string token;
	ParseToken( token, cursor );
	while ( token != "}" )
	{
		std::string key;
		std::swap( key, token );
		std::string value;
		ParseToken( value, cursor );
		AddKeyValueString( key.c_str(), value.c_str() );

		ParseToken( token, cursor );
	}
}

#if 0
bool EntityDefMgr::LoadGroup( const char *filename )
{
	char* ents = (char*)fload(filename);
	if ( ents )
	{
		const char *cursor = ents;
		while ( cursor && *cursor )
		{
			std::string name, token;
			if ( !ParseToken( name, cursor ) )
				break;
			if ( !ParseToken( token, cursor ) )
				break;
			if ( token == "{" )
			{
				EntityDef *def = new EntityDef;
				ParseEnt( *def, cursor );
				Guid g;
				GenerateGUID( g, name.c_str() );
				this->m_defs[g] = def;
			}
		}
		free( ents );
	}
	return true;
}
#endif

EntityDefMgr *EntityDefManager()
{
	static EntityDefMgr s_def;
	return &s_def;
}
