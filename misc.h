#pragma once

#include <assert.h>
#include <stdlib.h>
#include <map>
#include <string>

typedef unsigned int Guid;

void GenerateGUID( Guid &g, const char *str );
long int fsize(FILE* _file);
void* fload(const char* _filePath, unsigned int *len=NULL);
const char *skipws( const char *txt );
char *skipws( char *txt );

bool ParseString( std::string &i );
bool ParseInt( int &i );
bool ParseFloat( float &f );

bool ParseToken( std::string &token, const char *&s );

bool isoneof( char c, const char *possible );
const char *firstof( const char *s, const char *possible );
const char *firstnotof( const char *s, const char *possible );


struct CoreType
{
	bool m_registered;
	int m_from, m_to;
	CoreType *m_parent, *m_sibling, *m_child;
	CoreType *m_next;
	static CoreType *s_root;
	static CoreType *s_base;

	explicit CoreType( CoreType *parent ) : m_registered(false), m_from(-1), m_to(-1), m_parent(parent), m_sibling(NULL), m_child(NULL)
	{
		m_next = s_root;
		s_root = this;
	}

	void Register()
	{
		if ( m_registered )
			return;
		m_registered = true;
		if ( m_parent )
		{
			m_parent->Register();
			m_sibling = m_parent->m_child;
			m_parent->m_child = this;
		}
	}

	static void InitClassTypes();
};

inline bool IsTypeOf( CoreType const *a, CoreType const *b )
{
	return a->m_from >= b->m_from && a->m_to <= b->m_to;
}

template <class A, class B>
inline A *CoreCast( B *ptr )
{
	if ( &IsTypeOf( &B::s_Type, &A::s_Type ) )
		return (A*)ptr;
	return NULL;
}


class CoreClass
{
public:

	virtual ~CoreClass() {}

	void *operator new( size_t s )
	{
		void *mem = malloc(s);
		if ( mem )
			memset( mem, 0, s );
		return mem;
	}

	void operator delete( void *ptr )
	{
		free( ptr );
	}

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return NULL; }
};

class CoreClassCreator
{
	CoreClassCreator *m_next;
	static CoreClassCreator *s_root;
public:

	CoreClassCreator()
	{
		m_next = s_root;
		s_root = this;
	}

	virtual bool CanCreate( std::string const &name ) = 0;
	virtual CoreClass *Create() = 0;

	static  CoreClass *Create( std::string const &name )
	{
		CoreClassCreator *c = s_root;
		while ( c )
		{
			if ( c->CanCreate( name ) )
			{
				return c->Create();
			}
			c = c->m_next;
		}
		return NULL;
	}
};

template <class T>
class ClassCreator : public CoreClassCreator
{
	std::string m_name;

public:

	ClassCreator( const char *name ) : m_name(name)
	{
	}

	bool CanCreate( std::string const &name )
	{
		return name == m_name;
	}

	CoreClass *Create()
	{
		return new T;
	}
};

class DefMgr
{
	DefMgr *m_next;
	static DefMgr *s_root;

public:

	DefMgr();
	virtual ~DefMgr();

	virtual void Reload();
	virtual void Clear();
};

template <class T, bool LOADGROUP=false >
class TypedDefMgr : public DefMgr
{
protected:
	std::map< Guid, T* > m_defs;
public:

	const T* Get( const char *name, bool createDef = true )
	{
		Guid g;
		GenerateGUID( g, name );
		std::map<Guid,T*>::const_iterator f = m_defs.find( g );
		if ( f == m_defs.end() )
		{
			if ( !createDef )
			{
				return NULL;
			}
			else
			{
				T* def = new T;
				def->Load( name );
				m_defs[g] = def;
				return def;
			}
		} 
		else
		{
			return f->second;
		}
	}

	virtual void Reload()
	{
		std::map<Guid,T*>::iterator c = m_defs.begin();
		while ( c != m_defs.end() )
		{
			T* def = c->second;
			def->Reload();
			c++;
		}
	}

	virtual void Clear()
	{
		std::map<Guid,T*>::iterator c = m_defs.begin();
		while ( c != m_defs.end() )
		{
			T* def = c->second;
			def->Clear();
			delete def;
			c++;
		}
		m_defs.clear();
	}

	bool LoadGroup( const char *filename )
	{
		if ( LOADGROUP )
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
						T *def = new T;
						def->Parse( cursor );
						Guid g;
						GenerateGUID( g, name.c_str() );
						m_defs[g] = def;
					}
				}
				free( ents );
			}
			return true;
		}
		else
		{
			return false;
		}
	}

};

template <class T, int S>
class ChunkArray
{
	struct chunk
	{
		T items[S];
		int used;
		chunk *next;
	};

	chunk *root;
public:
	
	void Init()
	{
		root = NULL;
	}

	void Add( T item )
	{
		chunk *c = root;
		while ( c )
		{
			if ( c->used < S )
			{
				c->items[c->used++] = item;
				return;
			}
			c = c->next;
		}
		chunk *nc = new chunk;
		nc->used = 1;
		nc->next = root;
		nc->items[0] = item;
	}

	void Remove( T item )
	{
		chunk *c = root;
		while ( c )
		{
			for (int i=0; i<c->used; i++)
			{
				if ( c->items[i] == item )
				{
					c->used--;
					c->items[i] = c->items[c->used];
					return;
				}
			}
			c = c->next;
		}
	}
};