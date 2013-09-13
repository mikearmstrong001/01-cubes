#pragma once

#include "actor.h"

class Animal : public Actor
{

public:

	void UpdateFixed( float dt );
	void UpdateDelta( float dt );

	void Render();

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
