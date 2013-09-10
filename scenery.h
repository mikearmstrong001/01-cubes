#pragma once

#include "entity.h"

class Scenery : public Entity
{
	typedef Entity Super;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
