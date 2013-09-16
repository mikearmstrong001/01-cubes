#pragma once

#include <vector>
#include <stack>

class Level;
class Entity;
class Menu;

class World
{
	std::vector< Level* > m_levels;
	std::stack< Menu* > m_menus;
	int m_curLevel;

public:

	World();
	~World();

	bool Load( const char *filename );
	void Clear();

	void UpdateBegin();
	void UpdateFixed( float dt );
	void UpdateDelta( float dt );
	void UpdateEnd();

	void Render();
	void RenderGUI();

	void PushMenu( Menu *m );
	void PopMenu();

	Entity *FindEntityInCurrentLevel( const char *name );
};
