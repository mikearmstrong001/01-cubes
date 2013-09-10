#include "scenery.h"
#include "level.h"

static ClassCreator<Scenery> s_PlayerCreator( "Scenery" );
CoreType Scenery::s_Type( &Scenery::Super::s_Type );


void Scenery::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel( l );

	l->AddRender( this );
}

void Scenery::OnRemoveFromLevel( Level *l )
{
	l->RemoveRender( this );

	Super::OnRemoveFromLevel( l );
}

void Scenery::Spawn( KVStore const &kv )
{
	Super::Spawn( kv );
}
