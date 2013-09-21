#pragma once

#include <vector>
#include "misc.h"
#include "entity.h"

struct particleIniter_s
{
	float range[2][3];
};

struct particleGraph_s
{
	int numElements;
	std::vector< float > data;
};

struct ParticleDef
{
	int maxParticles;
	int totalParticles;
	float spawnRate;

	particleIniter_s posInit;
	particleIniter_s velInit;
	particleIniter_s accInit;
	particleIniter_s lifeInit;
	particleIniter_s emitterLifeInit;

	particleGraph_s  sizeByLife;
	particleGraph_s  orbitRadiusByLife;
	particleGraph_s  orbitAngleByLife;

//public:

	bool Load( const char *filename );
	void Reload();
	void Clear();

};

struct particleSystem_s
{
	int numParticles;
	int totalParticles;
	float spawnErr;
	float emitterLife;

	const ParticleDef *def;

	std::vector< float > pos;
	std::vector< float > vel;
	std::vector< float > acc;
	std::vector< float > life;
	std::vector< float > startLife;
};

void initParticleDecl();

class ParticleDefMgr : public TypedDefMgr<ParticleDef>
{

public:

};

ParticleDefMgr *ParticleDefManager();



class ParticleEntity : public Entity
{
	typedef Entity Super;

	particleSystem_s m_psys;

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );
	void UpdateDelta( float dt );
	void Render();

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};

class HitpointEntity : public Entity
{
	typedef Entity Super;

	float m_time;
	std::string m_text;
	float m_vel[3];

public:

	void OnAddToLevel( Level *l );
	void OnRemoveFromLevel( Level *l );

	void Spawn( KVStore const &kv );
	void UpdateDelta( float dt );
	void Render();

	static CoreType s_Type;
	virtual CoreType const *Type() const { return &s_Type; }
	virtual CoreType const *SuperType() const { return &Super::s_Type; }
};
