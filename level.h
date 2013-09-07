#pragma once

#include <vector>
#include <map>
#include "voxel.h"
#include "mesh.h"
#include "misc.h"
#include "kvstore.h"


class Entity;
class World;

class Level
{
	World *m_world;

	map_s m_map;
	MapMesh m_mesh;

	typedef std::vector< Entity* > EntityContainer;
	EntityContainer m_entities;
	int m_curLevel;

	typedef std::vector< Entity* > UpdateContainer;
	UpdateContainer m_fixedUpdate;
	UpdateContainer m_deltaUpdate;
	UpdateContainer m_guiUpdate;
	UpdateContainer m_renderUpdate;

public:

	Level();
	~Level();

	void AddFixed( Entity* ent );
	void AddDelta( Entity* ent );
	void AddGui( Entity* ent );
	void AddRender( Entity* ent );

	void RemoveFixed( Entity* ent );
	void RemoveDelta( Entity* ent );
	void RemoveGui( Entity* ent );
	void RemoveRender( Entity* ent );

	bool Load( const char *filename );
	void Clear();

	void UpdateFixed( float dt );
	void UpdateDelta( float dt );
	void UpdateGUI();

	void Render();

	map_s* GetMap() { return &m_map; }

	Entity *FindEntity( const char *name );
};


class EntsDef
{
	std::vector<KVStore*> m_ents;
	std::string m_filename;
public:

	virtual bool Load( const char *filename );
	virtual void Reload();
	virtual void Clear();

	unsigned int GetNum() const
	{
		return m_ents.size();
	}

	KVStore const & Get( unsigned int i ) const
	{
		return *m_ents[i];
	}
};

class EntsDefMgr : public TypedDefMgr<EntsDef>
{
public:
};

EntsDefMgr *EntsDefManager();
