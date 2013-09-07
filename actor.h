#pragma once

#include "entity.h"

class Actor : public Entity
{
	typedef Entity Super;

protected:

	virtual void PreDeltaUpdate( float dt );
	virtual void PostDeltaUpdate( float dt );
	virtual bool AnimUpdate( const char *animName, float dt );

	bool m_onGround;
	float m_vel[3];
	float m_pos[3];
	float m_rotz;
	float m_moveSpeed;
	float m_animDir;
	float m_animTime;
	int   m_stateIndex;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );

	const float *GetPos() const { return m_pos; };
	const float  GetRotZ() const { return m_rotz; };
};
