#pragma once

#include "entity.h"

class Pickup : public Entity
{
	typedef Entity Super;
	float m_rotz;
	float m_rotRate;
	float m_bounceZ;
	float m_bounceZRate;
	float m_bounceZDist;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );

	void UpdateDelta( float dt );

	bool Use( Entity *by );

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
