#pragma once

#include "actor.h"

class Zombie : public Actor
{
	typedef Actor Super;
	float m_noProgress;
	float m_stateTime;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void UpdateDelta( float dt );
};
