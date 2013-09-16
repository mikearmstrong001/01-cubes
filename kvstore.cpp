#include "kvstore.h"
#include <string>

std::map<Guid,std::string> KVStore::s_keymap;

static KVStore::KeyValue &AddItem( Guid const &g, std::vector<KVStore::KeyValue> &keys )
{
	for (unsigned int i=0; i<keys.size(); i++)
	{
		if ( keys[i].k == g )
			return keys[i];
	}
	keys.push_back( KVStore::KeyValue() );
	return keys.back();
}

static std::vector<KVStore::KeyValue>::iterator Find( std::vector<KVStore::KeyValue> &keys, const Guid &g )
{
	std::vector<KVStore::KeyValue>::iterator b = keys.begin();
	while ( b != keys.end() )
	{
		if ( b->k == g )
			return b;
		b++;
	}
	return keys.end();
}

static std::vector<KVStore::KeyValue>::const_iterator Find( std::vector<KVStore::KeyValue> const &keys, const Guid &g )
{
	std::vector<KVStore::KeyValue>::const_iterator b = keys.begin();
	while ( b != keys.end() )
	{
		if ( b->k == g )
			return b;
		b++;
	}
	return keys.end();
}

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
		if ( value == "[" )
		{
			KVStore *store = new KVStore;
			ParseArray( store, cursor );
			kv->AddKeyValueKeyValue( key.c_str(), store );
		}
		else
		{
			kv->AddKeyValueString( key.c_str(), value.c_str() );
		}
	}
}

void KVStore::ParseArray( KVStore *kv, const char *&cursor )
{
	int index = 0;
	std::string value;
	while ( ParseToken( value, cursor ) && value != "]" )
	{
		if ( value == "{" )
		{
			KVStore *store = new KVStore;
			Parse( store, cursor );
			kv->AddKeyValueKeyValue( Guid(index), store );
		}
		else
		if ( value == "[" )
		{
			KVStore *store = new KVStore;
			ParseArray( store, cursor );
			kv->AddKeyValueKeyValue( Guid(index), store );
		}
		else
		{
			kv->AddKeyValueString( Guid(index), value.c_str() );
		}
		index++;
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
	std::vector<KeyValue>::iterator b = m_store.begin();
	while ( b != m_store.end() )
	{
		Value &v = b->v;
		
		if ( v.type == KVTString )
			free( (void*)v.str );
		if ( v.type == KVTStore )
			delete v.store;

		b++;
	}
	m_store.clear();
}

void KVStore::AddKeyValueKeyValue( const char *k, const KVStore *v, bool copy )
{
	Guid g;
	GenerateGUID( g, k );
	s_keymap[g] = k;
	AddKeyValueKeyValue( g, v, copy );
}

void KVStore::AddKeyValueKeyValue( const Guid &g, const KVStore *v, bool copy )
{
	if ( copy )
	{
		KVStore *newv = new KVStore;
		for (unsigned int i=0; i<v->m_store.size(); i++)
		{
			const KeyValue &kv = v->m_store[i];
			if ( kv.v.type == KVTString )
				newv->AddKeyValueString( kv.k, kv.v.str );
			else if ( kv.v.type == KVTStore )
				newv->AddKeyValueKeyValue( kv.k, kv.v.store, true );
		}
		v = newv;
	}

	KeyValue &val = AddItem( g, m_store );
	val.k = g;
	val.v.type = KVTStore;
	val.v.store = v;
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
	KeyValue &val = AddItem( g, m_store );
	val.k = g;
	val.v.type = KVTString;
	val.v.str = strdup( v );
}

const char *KVStore::GetKeyValueString( const char *k, const char *def ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::vector<KeyValue>::const_iterator b = Find(m_store,g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->v;
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
	std::vector<KeyValue>::const_iterator b = Find(m_store,g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->v;
	if ( v.type == KVTString )
		return (float)atof(v.str);

	return def;
}

int KVStore::GetKeyValueInt( const char *k, int def ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::vector<KeyValue>::const_iterator b = Find(m_store,g);
	if ( b == m_store.cend() )
		return def;
	
	Value const &v = b->v;
	if ( v.type == KVTString )
		return (int)floor(atof(v.str));

	return def;
}

void KVStore::GetKeyValueFloatArray( float *out, int num, const char *k ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::vector<KeyValue>::const_iterator b = Find(m_store,g);
	memset( out, 0, sizeof(float)*num );
	if ( b == m_store.cend() )
		return;
	
	Value const &v = b->v;
	if ( v.type == KVTString )
	{
		const char *cursor = v.str;
		for (int i=0; i<num; i++)
		{
			std::string tok;
			if ( !ParseToken( tok, cursor ) )
				break;
			out[i] = (float)atof( tok.c_str() );
		}
	}
}

int KVStore::GetNumItems() const
{
	return (int)m_store.size();
}

const char *KVStore::GetIndexValueString( int index, const char *def ) const
{
	if ( index < 0 || index >= (int)m_store.size() )
		return def;

	KeyValue const &kv = m_store[index];
	if ( kv.v.type == KVTString )
		return kv.v.str;
	return def;
}

float KVStore::GetIndexValueFloat( int index, float def ) const
{
	if ( index < 0 || index >= (int)m_store.size() )
		return def;

	KeyValue const &kv = m_store[index];
	if ( kv.v.type == KVTString )
		return (float)atof(kv.v.str);
	return def;
}

void KVStore::GetIndexValueFloatArray( float *out, int num, int index ) const
{
	memset( out, 0, sizeof(float)*num);
	if ( index < 0 || index >= (int)m_store.size() )
		return;

	KeyValue const &kv = m_store[index];
	Value const &v = kv.v;
	if ( v.type == KVTString )
	{
		const char *cursor = v.str;
		for (int i=0; i<num; i++)
		{
			std::string tok;
			if ( !ParseToken( tok, cursor ) )
				break;
			out[i] = (float)atof( tok.c_str() );
		}
	}
}

int KVStore::GetIndexValueInt( int index, int def ) const
{
	if ( index < 0 || index >= (int)m_store.size() )
		return def;

	KeyValue const &kv = m_store[index];
	if ( kv.v.type == KVTString )
		return (int)floor(atof(kv.v.str));
	return def;
}

const KVStore *KVStore::GetIndexValueKeyValue( int index ) const
{
	static KVStore empty;
	if ( index < 0 || index >= (int)m_store.size() )
		return &empty;

	KeyValue const &kv = m_store[index];
	if ( kv.v.type == KVTStore )
		return kv.v.store;
	return &empty;
}

bool KVStore::HasKey( const char *k ) const
{
	Guid g;
	GenerateGUID( g, k );
	std::vector<KeyValue>::const_iterator b = Find(m_store,g);
	if ( b == m_store.cend() )
		return false;

	return true;
}

void KVStore::Dump( std::string &output ) const
{
	std::vector<KeyValue>::const_iterator b = m_store.begin();
	while ( b != m_store.cend() )
	{
		std::string key = s_keymap[ b->k ];

		output.append( "\t\"" );
		output.append( key );
		output.append( "\" \"" );
		output.append( b->v.str );
		output.append( "\"\n" );
		b++;
	}

}


void KVStore::MergeInto( KVStore const &other )
{
	std::vector<KeyValue>::const_iterator b = other.m_store.begin();
	while ( b != other.m_store.cend() )
	{
		std::vector<KeyValue>::const_iterator f = Find(m_store,b->k);
		if ( f == m_store.cend() )
		{
			Value const &v = b->v;
			if ( v.type == KVTString )
				AddKeyValueString( b->k, v.str );
			if ( v.type == KVTStore )
				AddKeyValueKeyValue( b->k, v.store, true );
		}
		b++;
	}
}
