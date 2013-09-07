#pragma once

#include "entity.h"

class Scenery : public Entity
{
	typedef Entity Super;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );
};
