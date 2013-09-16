#include "world.h"
#include "level.h"
#include "entitydef.h"
#include "menu.h"

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
	l->SetWorld( this );
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

void World::UpdateBegin()
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->UpdateBegin();
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

void World::UpdateEnd()
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->UpdateEnd();
}

void World::Render()
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->Render();
}

void World::RenderGUI()
{
	if ( m_curLevel == -1 )
		return;

	m_levels[m_curLevel]->RenderGUI();

	if ( m_menus.size() )
	{
		m_menus.top()->Render();
	}
}

Entity *World::FindEntityInCurrentLevel( const char *name )
{
	if ( m_curLevel == -1 )
		return NULL;

	return m_levels[m_curLevel]->FindEntity( name );
}

void World::PushMenu( Menu *m )
{
	m_menus.push( m );
}

void World::PopMenu()
{
	Menu *m = m_menus.top();
	m_menus.pop();
	delete m;
}


