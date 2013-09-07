#include "world.h"
#include "level.h"
#include "entitydef.h"

World::World() : m_curLevel(-1)
{
}

World::~World()
{
	Clear();
}

bool World::Load( const char * filename )
{
	EntityDef const *def = EntityDefManager()->Get( filename );

	Level *l = new Level;
	l->Load( def->GetKeyValueString( "level" ) );
	m_levels.push_back( l );
	m_curLevel = 0;

	return true;
}

void World::Clear()
{
	for (unsigned int i=0; i<m_levels.size(); i++)
	{
		m_levels[i]->Clear();
		delete m_levels[i];
	}
	m_levels.clear();
	m_curLevel = -1;
}


void World::UpdateFixed( float dt )
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->UpdateFixed( dt );
}

void World::UpdateDelta( float dt )
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->UpdateDelta( dt );
}

void World::UpdateGUI()
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->UpdateGUI();
}


void World::Render()
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->Render();
}

Entity *World::FindEntityInCurrentLevel( const char *name )
{
	if ( m_curLevel == -1 )
		return NULL;

	return m_levels[m_curLevel]->FindEntity( name );
}
