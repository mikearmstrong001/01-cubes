#pragma once

#include "actor.h"

class Player : public Actor
{
	typedef Actor Super;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );

	//void UpdateFixed( float dt );
	void UpdateDelta( float dt );
	
	void RenderGUI();

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
