#include "misc.h"

CoreClassCreator *CoreClassCreator::s_root = NULL;

CoreType *CoreType::s_root = NULL;
CoreType *CoreType::s_base = NULL;

CoreType CoreClass::s_Type( NULL );


static int CalcCoreTypeRanges( CoreType *root, int start )
{
	int kidscount = 1;
	CoreType *kid = root->m_child;
	root->m_from = start;
	start++;
	while ( kid )
	{
		int count = CalcCoreTypeRanges( kid, start );
		kidscount += count;
		start += count;
		kid = kid->m_sibling;
	}
	root->m_to = root->m_from + kidscount;
	return kidscount;
}



void CoreType::InitClassTypes()
{
	CoreType *c = s_root;
	while ( c )
	{
		c->Register();
		if ( c->m_parent == NULL )
			s_base = c;
		c = c->m_next;
	}
	CalcCoreTypeRanges( s_base, 0 );
}

void GenerateGUID( Guid &g, const char *str )
{
	g = 0;
	while ( *str )
	{
		g = (g*33) + (*str) + 7;
		str++;
	}
}

long int fsize(FILE* _file)
{
	long int pos = ftell(_file);
	fseek(_file, 0L, SEEK_END);
	long int size = ftell(_file);
	fseek(_file, pos, SEEK_SET);
	return size;
}

void* fload(const char* _filePath, unsigned int *len)
{
	FILE* file = fopen(_filePath, "rb");
	if (NULL != file)
	{
		unsigned int size = (unsigned int)fsize(file);
		char* mem = (char*)malloc( size+1 );
		fread(mem, 1, size, file);
		fclose(file);
		mem[size] = 0;
		if ( len )
			*len = size;
		return mem;
	}

	return NULL;
}

bool isoneof( char c, const char *possible )
{
	while ( *possible )
	{
		if ( *possible == c )
			return true;
		possible++;
	}
	return false;
}

const char *firstof( const char *s, const char *possible )
{
	const char *cursor = s;
	while ( *cursor )
	{
		if ( isoneof( *cursor, possible ) )
			return cursor;
		cursor++;
	}
	return NULL;
}

const char *firstnotof( const char *s, const char *possible )
{
	const char *cursor = s;
	while ( *cursor )
	{
		if ( !isoneof( *cursor, possible ) )
			return cursor;
		cursor++;
	}
	return NULL;
}

bool ParseToken( std::string &token, const char *&s )
{
	token.clear();
	do
	{
		s = skipws( s );
		if ( *s == 0 )
		{
			return false;
		} else 
		if ( s[0] == '/' && s[1] == '/' )
		{
			while ( *s && *s != '\n' )
				s++;
			continue;
		} else
		if ( s[0] == '/' && s[1] == '*' )
		{
			while ( *s && !(s[0] == '*' && s[1] == '/') )
				s++;
			if ( *s )
				s+=2;
			continue;
		} else
		if ( s[0] == '"' )
		{
			s++;
			while ( *s )
			{
				if ( s[0] == '"' && s[1] == '"' )
				{
					token.push_back( '"' );
					s+=2;
					continue;
				}
				if ( s[0] == '"' )
					break;
				token.push_back( s[0] );
				s++;
			}
			s++;
			return true;
		} else
		if ( *s == '-' && isoneof( s[1], "0123456789" ) )
		{
			while ( *s && !isspace( *s ) )
			{
				token.push_back( *s );
				s++;
			}
			return true;
		} else
		if ( isoneof( *s, "{}[]()+-*/&^%$£<>" ) )
		{
			token.push_back( *s );
			s++;
			return true;
		} else
		{
			while ( *s && !isspace( *s ) )
			{
				token.push_back( *s );
				s++;
			}
			return true;
		}
	} while ( true );
	return false;
}


const char *skipws( const char *txt )
{
	while ( *txt && isspace(*txt) )
		txt++;
	return txt;
}

char *skipws( char *txt )
{
	while ( *txt && isspace(*txt) )
		txt++;
	return txt;
}

bool ParseString( std::string &i, const char *&cursor )
{
	return ParseToken( i, cursor );
}

bool ParseInt( int &i, const char *&cursor )
{
	std::string tok;
	if ( ParseToken( tok, cursor ) )
	{
		i = atoi( tok.c_str() );
		return true;
	}
	else
	{
		return false;
	}
}

bool ParseFloat( float &f, const char *&cursor )
{
	std::string tok;
	if ( ParseToken( tok, cursor ) )
	{
		f = atof( tok.c_str() );
		return true;
	}
	else
	{
		return false;
	}
}

DefMgr *DefMgr::s_root = NULL;

DefMgr::DefMgr()
{
	m_next = s_root;
	s_root = this;
}

DefMgr::~DefMgr()
{
}

void DefMgr::Reload()
{
}

void DefMgr::Clear()
{
}
