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
	float m_rotz;
	float m_moveSpeed;
	float m_animDir;
	float m_animTime;
	int   m_stateIndex;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );

	const float  GetRotZ() const { return m_rotz; };

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
