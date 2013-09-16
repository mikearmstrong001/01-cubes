#pragma once

#include "menu.h"

class Entity;

class WorkbenchMenu : public Menu
{
	Entity *m_item;
	Entity *m_user;
public:

	WorkbenchMenu( Entity *i, Entity *u );

	void Render();
};
