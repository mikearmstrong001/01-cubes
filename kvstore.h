#pragma once

#include <vector>
#include "misc.h"


class KVStore
{
protected:
	enum KVType
	{
		KVTString,
		KVTStore,
	};

	struct Value
	{
		KVType type;
		union
		{
			const char *str;
			const KVStore *store;
		};
	};

public:
	struct KeyValue
	{
		Guid k;
		Value v;
	};

protected:
	//std::map<Guid,Value> m_store;
	std::vector<KeyValue> m_store;
	static std::map<Guid,std::string> s_keymap;

	void AddKeyValueKeyValue( const char *k, const KVStore *v, bool copy = false );
	void AddKeyValueKeyValue( const Guid &g, const KVStore *v, bool copy = false );

	static void Parse( KVStore *kv, const char *&cursor );
	static void ParseArray( KVStore *kv, const char *&cursor );

public:

	KVStore();
	virtual ~KVStore();

	virtual bool Load( const char *filename );
	virtual void Parse( const char *name, const char *&cursor );
	virtual void Clear();

	void AddKeyValueString( const char *k, const char *v );
	void AddKeyValueString( const Guid &g, const char *v );


	const char *GetKeyValueString( const char *k, const char *def = "" ) const;
	float GetKeyValueFloat( const char *k, float def = 0.f ) const;
	void GetKeyValueFloatArray( float *out, int num, const char *k ) const;
	int GetKeyValueInt( const char *k, int def = 0 ) const;
	const KVStore *GetKeyValueKeyValue( const char *k ) const;

	int GetNumItems() const;
	const char *GetIndexValueString( int index, const char *def = "" ) const;
	float GetIndexValueFloat( int index, float def = 0.f ) const;
	void GetIndexValueFloatArray( float *out, int num, int index ) const;
	int GetIndexValueInt( int index, int def = 0 ) const;
	const KVStore *GetIndexValueKeyValue( int index ) const;

	bool HasKey( const char *k ) const;

	void Dump( std::string &output ) const;

	void MergeInto( KVStore const &other );
};
