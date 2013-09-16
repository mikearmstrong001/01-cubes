#include "workbench.h"
#include "workbench_menu.h"
#include "level.h"
#include "world.h"

static ClassCreator<Workbench> s_PlayerCreator( "Workbench" );
CoreType Workbench::s_Type( &Workbench::Super::s_Type );


void Workbench::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel( l );

	l->AddRender( this );
}

void Workbench::OnRemoveFromLevel( Level *l )
{
	l->RemoveRender( this );

	Super::OnRemoveFromLevel( l );
}

void Workbench::Spawn( KVStore const &kv )
{
	Super::Spawn( kv );
	m_active = false;
}

bool Workbench::Use( Entity *by )
{
	if ( !m_active )
	{
		m_world->PushMenu( new WorkbenchMenu( this, by ) );
	}
	m_active = true;
	return true;
}