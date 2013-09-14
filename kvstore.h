#pragma once

#include <map>
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

	std::map<Guid,Value> m_store;
	static std::map<Guid,std::string> s_keymap;

	void AddKeyValueKeyValue( const char *k, const KVStore *v );
	void AddKeyValueKeyValue( const Guid &g, const KVStore *v );

	static void Parse( KVStore *kv, const char *&cursor );

public:

	KVStore();
	virtual ~KVStore();

	virtual bool Load( const char *filename );
	virtual void Parse( const char *&cursor );
	virtual void Clear();

	void AddKeyValueString( const char *k, const char *v );
	void AddKeyValueString( const Guid &g, const char *v );


	const char *GetKeyValueString( const char *k, const char *def = "" ) const;
	float GetKeyValueFloat( const char *k, float def = 0.f ) const;
	void GetKeyValueFloatArray( float *out, int num, const char *k ) const;
	int GetKeyValueInt( const char *k, int def = 0 ) const;
	const KVStore *GetKeyValueKeyValue( const char *k ) const;

	bool HasKey( const char *k ) const;

	void Dump( std::string &output ) const;

	void MergeInto( KVStore const &other );
};
