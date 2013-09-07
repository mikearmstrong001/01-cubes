#pragma once

#include <vector>

class Level;
class Entity;

class World
{
	std::vector< Level* > m_levels;
	int m_curLevel;

public:

	World();
	~World();

	bool Load( const char *filename );
	void Clear();

	void UpdateFixed( float dt );
	void UpdateDelta( float dt );
	void UpdateGUI();

	void Render();

	Entity *FindEntityInCurrentLevel( const char *name );
};
