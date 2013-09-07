#pragma once

#include <vector>

struct particleIniter_s
{
	float range[2][3];
};

struct particleGraph_s
{
	int numElements;
	std::vector< float > data;
};

struct particleDef_s
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
};

struct particleSystem_s
{
	int numParticles;
	int totalParticles;
	float spawnErr;
	float emitterLife;

	particleDef_s *def;

	std::vector< float > pos;
	std::vector< float > vel;
	std::vector< float > acc;
	std::vector< float > life;
	std::vector< float > startLife;
};

void initParticleDecl();

void initParticleSystem( particleSystem_s &ps, const char *filename );
bool updateParticleSystem( particleSystem_s &ps, float dt );
void renderParticleSystem( particleSystem_s &ps );

void reloadParticleSystems();

