#include "kvstore.h"
#include <string>

std::map<Guid,std::string> KVStore::s_keymap;

KVStore::KVStore()
{
}

KVStore::~KVStore()
{
	Clear();
}

bool KVStore::Load( const char *filename )
{
	FILE *f = fopen( filename, "rb" );
	if ( f )
	{
		char line[4096];
		while ( fgets( line, sizeof(line), f ) )
		{
			std::string str = line;

			std::string::size_type startk = str.find_first_not_of( " \t\n\r" );
			if ( startk == std::string::npos )
				continue;

			std::string::size_type endk = str.find_first_of( " \t\n\r", startk+1 );
			if ( endk == std::string::npos )
				continue;

			std::string::size_type startv = str.find_first_not_of( " \t\n\r", endk+1 );
			if ( startv == std::string::npos )
				continue;

			std::string::size_type endv = str.find_first_of( " \t\n\r", startv+1 );

			std::string key = str.substr( startk, endk-startk );
			std::string val = str.substr( startv, endv!=std::string::npos ? endv-startv : endv );

			AddKeyValueString( key.c_str(), val.c_str() );
		}

		return true;
	}
	else
	{
		return false;
	}
}

void KVStore::Clear()
{
	std::map<Guid,Value>::iterator b = m_store.begin();
	while ( b != m_store.end() )
	{
		Value &v = b->second;
		
		if ( v.type == KVString )
			free( (void*)v.str );

		b++;
	}
	m_store.clear();
}


void KVStore::AddKeyValueString( const char *k, const char *v )
{
	Guid g;
	GenerateGUID( g, k );
	s_keymap[g] = k;
	Value &val = m_store[g];
	val.type = KVString;
	val.str = strdup( v );
}

void KVStore::AddKeyValueFloat( const char *k, float v )
{
	Guid g;
	GenerateGUID( g, k );
	s_keymap[g] = k;
	Value &val = m_store[g];
	val.type = KVFloat;
	val.f = v;
}

void KVStore::AddKeyValueInt( const char *k, int v )
{
	Guid g;
	GenerateGUID( g, k );
	s_keymap[g] = k;
	Value &val = m_store[g];
	val.type = KVInt;
	val.i = v;
}


const char *KVStore::GetKeyValueString( const char *k, const char *def ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::map<Guid,Value>::const_iterator b = m_store.find(g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->second;
	/*if ( v.type == KVFloat )
	{
		char buf[1024];
		sprintf( buf, "%f", v.f );
		return buf;
	}
	if ( v.type == KVInt )
	{
		char buf[1024];
		sprintf( buf, "%d", v.i );
		return buf;
	}*/
	if ( v.type == KVString )
	{
		return v.str;
	}

	return def;
}

float KVStore::GetKeyValueFloat( const char *k, float def ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::map<Guid,Value>::const_iterator b = m_store.find(g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->second;
	if ( v.type == KVInt )
		return (float)v.i;
	if ( v.type == KVString )
		return (float)atof(v.str);
	if ( v.type == KVFloat )
		return v.f;

	return def;
}

int KVStore::GetKeyValueInt( const char *k, int def ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::map<Guid,Value>::const_iterator b = m_store.find(g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->second;
	if ( v.type == KVFloat )
		return (int)floorf(v.f);
	if ( v.type == KVString )
		return (int)floor(atof(v.str));
	if ( v.type == KVInt )
		return v.i;

	return def;
}

void KVStore::GetKeyValueFloatArray( float *out, int num, const char *k ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::map<Guid,Value>::const_iterator b = m_store.find(g);
	memset( out, 0, sizeof(float)*num );
	if ( b == m_store.cend() )
		return;
	
	Value const &v = b->second;
	if ( v.type == KVInt )
		out[0] = (float)v.i;
	if ( v.type == KVString )
	{
		char temp[1024];
		strcpy( temp, v.str );
		char *tok = strtok( temp, " \t\n\r" );
		out[0] = (float)atof(tok);
		for (int i=1; i<num; i++)
		{
			ParseFloat( out[i] );
		}
	}
	if ( v.type == KVFloat )
		out[0] = v.f;
}



bool KVStore::HasKey( const char *k ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::map<Guid,Value>::const_iterator b = m_store.find(g);
	if ( b == m_store.cend() )
		return false;

	return true;
}

void KVStore::Dump( std::string &output ) const
{
	std::map<Guid,Value>::const_iterator b = m_store.begin();
	while ( b != m_store.cend() )
	{
		std::string key = s_keymap[ b->first ];

		output.append( "\t\"" );
		output.append( key );
		output.append( "\" \"" );
		output.append( b->second.str );
		output.append( "\"\n" );
		b++;
	}

}
