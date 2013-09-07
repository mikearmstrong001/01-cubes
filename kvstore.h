#pragma once

#include <map>
#include "misc.h"


class KVStore
{
	enum KVType
	{
		KVString,
		KVFloat,
		KVFloatArray,
		KVInt,
	};

	struct FloatArray
	{
		int count;
		float f[1];
	};

	struct Value
	{
		KVType type;
		union
		{
			const char *str;
			float f;
			int i;
			FloatArray *fa;
		};
	};

	std::map<Guid,Value> m_store;
	static std::map<Guid,std::string> s_keymap;

public:

	KVStore();
	virtual ~KVStore();

	virtual bool Load( const char *filename );
	virtual void Clear();

	void AddKeyValueString( const char *k, const char *v );
	void AddKeyValueFloat( const char *k, float v );
	void AddKeyValueFloatArray( const char *k, float *v, int num );
	void AddKeyValueInt( const char *k, int v );

	const char *GetKeyValueString( const char *k, const char *def = "" ) const;
	float GetKeyValueFloat( const char *k, float def = 0.f ) const;
	void GetKeyValueFloatArray( float *out, int num, const char *k ) const;
	int GetKeyValueInt( const char *k, int def = 0 ) const;

	bool HasKey( const char *k ) const;

	void Dump( std::string &output ) const;
};
