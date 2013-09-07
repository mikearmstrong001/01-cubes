#pragma once

#include "entity.h"

class Spawner : public Entity
{
	typedef Entity Super;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );
};
