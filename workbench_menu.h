#pragma once

#include "menu.h"

class Entity;

class WorkbenchMenu : public Menu
{
	Entity *m_item;
	Entity *m_user;
	int m_cursor;
public:

	WorkbenchMenu( Entity *i, Entity *u );

	void Update();
	void Render();
};
