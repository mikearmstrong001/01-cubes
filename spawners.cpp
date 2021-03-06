#include "spawners.h"
#include "level.h"

static ClassCreator<Spawner> s_SpawnerCreator( "Spawner" );
CoreType Spawner::s_Type( &Spawner::Super::s_Type );


void Spawner::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel( l );

	l->AddRender( this );
}

void Spawner::OnRemoveFromLevel( Level *l )
{
	l->RemoveRender( this );

	Super::OnRemoveFromLevel( l );
}

void Spawner::Spawn( KVStore const &kv )
{
	Super::Spawn( kv );
}
