#pragma once

#include "entity.h"

class Workbench : public Entity
{
	typedef Entity Super;
	bool m_active;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );

	bool Use( Entity *by );

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
