#pragma once

#include "actor.h"

class Animal : public Actor
{

public:

	void UpdateFixed( float dt );
	void UpdateDelta( float dt );
	void UpdateGUI();

	void Render();
};
