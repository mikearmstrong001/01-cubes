#pragma once

#include "mesh.h"
#include "misc.h"

class Level;
class World;
class KVStore;
class EntityDef;

class Entity : public CoreClass
{
	typedef CoreClass Super;

protected:
	Level* m_level;
	World* m_world;

	std::string m_attachName;
	std::string m_attachJoint;
	float m_attachOffsetMtx[16];

	int   m_flashTime;
	float m_pos[3];
	float m_wmtx[16];
	const AnimMeshDef *m_meshDef;
	AnimMeshInstance m_meshInstance;
	std::string m_name;

	const EntityDef *m_entityDef;

public:

	Entity();
	virtual ~Entity();

	virtual void Spawn( KVStore const &kv );
	virtual void Clear();

	void SetEntityDef( const EntityDef *edef ) { m_entityDef = edef; }
	const EntityDef *GetEntityDef( void ) { return m_entityDef; }
	World *GetWorld() { return m_world; }

	virtual void OnSetWorld( World *w );
	virtual void OnAddToLevel( Level *l );
	virtual void OnRemoveFromLevel( Level *l );

	virtual bool Use( Entity *by );
	virtual void Attack( Entity *by );

	virtual void UpdateFixed( float dt );
	virtual void UpdateDelta( float dt );

	virtual void UpdateAttachment();

	virtual void Render();
	virtual void RenderGUI();

	const std::string &GetName() const { return m_name; }
	void SetName( const char *name ) { m_name = name; }

	const float *GetPos() const { return m_pos; };

	const float *GetWorldMatrix() const { return m_wmtx; }
	void CalcAttachWorldMatrix( float wmtx[16], const char *name ) const;

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
