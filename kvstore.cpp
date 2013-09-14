#include "kvstore.h"
#include <string>

std::map<Guid,std::string> KVStore::s_keymap;

void KVStore::Parse( KVStore *kv, const char *&cursor )
{
	std::string token;
	while ( ParseToken( token, cursor ) && token != "}" )
	{
		std::string key;
		std::swap( key, token );
		std::string value;
		ParseToken( value, cursor );
		if ( value == "{" )
		{
			KVStore *store = new KVStore;
			Parse( store, cursor );
			kv->AddKeyValueKeyValue( key.c_str(), store );
		}
		else
		{
			kv->AddKeyValueString( key.c_str(), value.c_str() );
		}
	}
}


KVStore::KVStore()
{
}

KVStore::~KVStore()
{
	Clear();
}

bool KVStore::Load( const char *filename )
{
	char* f = (char*)fload(filename);
	if ( f )
	{
		const char *cursor = f;
		Parse( cursor );
		return true;
	}
	else
	{
		return false;
	}
}

void KVStore::Parse( const char *&cursor )
{
	Parse( this, cursor );
}

void KVStore::Clear()
{
	std::map<Guid,Value>::iterator b = m_store.begin();
	while ( b != m_store.end() )
	{
		Value &v = b->second;
		
		if ( v.type == KVTString )
			free( (void*)v.str );
		if ( v.type == KVTStore )
			delete v.store;

		b++;
	}
	m_store.clear();
}

void KVStore::AddKeyValueKeyValue( const char *k, const KVStore *v )
{
	Guid g;
	GenerateGUID( g, k );
	s_keymap[g] = k;
	AddKeyValueKeyValue( g, v );
}

void KVStore::AddKeyValueKeyValue( const Guid &g, const KVStore *v )
{
	Value &val = m_store[g];
	val.type = KVTStore;
	val.store = v;
}

void KVStore::AddKeyValueString( const char *k, const char *v )
{
	Guid g;
	GenerateGUID( g, k );
	s_keymap[g] = k;
	AddKeyValueString( g, v );
}

void KVStore::AddKeyValueString( const Guid &g, const char *v )
{
	Value &val = m_store[g];
	val.type = KVTString;
	val.str = strdup( v );
}

const char *KVStore::GetKeyValueString( const char *k, const char *def ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::map<Guid,Value>::const_iterator b = m_store.find(g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->second;
	if ( v.type == KVTString )
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
	if ( v.type == KVTString )
		return (float)atof(v.str);

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
	if ( v.type == KVTString )
		return (int)floor(atof(v.str));

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
	if ( v.type == KVTString )
	{
		const char *cursor = v.str;
		for (int i=0; i<num; i++)
		{
			std::string tok;
			if ( !ParseToken( tok, cursor ) )
				break;
			out[i] = atof( tok.c_str() );
		}
	}
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


void KVStore::MergeInto( KVStore const &other )
{
	std::map<Guid,Value>::const_iterator b = other.m_store.begin();
	while ( b != other.m_store.cend() )
	{
		std::map<Guid,Value>::const_iterator f = m_store.find(b->first);
		if ( f == m_store.cend() )
		{
			Value const &v = b->second;
			if ( v.type == KVTString )
				AddKeyValueString( b->first, v.str );
		}
		b++;
	}
}
