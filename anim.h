#pragma once

#include <vector>

enum AnimKeyIndex
{
	AKI_OFSX = 0,
	AKI_OFSY,
	AKI_OFSZ,
	AKI_ROTX,
	AKI_ROTY,
	AKI_ROTZ,
	AKI_VIS,
	AKI_COUNT,
};

struct AnimKey
{
	float time;
	float value;
};

struct AnimChannel
{
	std::vector< AnimKey > keys[AKI_COUNT];
};

struct AnimTag
{
	float time;
	int   id;
};

struct Anim
{
	std::string name;
	float time;
	int looping;
	std::vector< AnimChannel > channels;
	std::vector< AnimTag > tags;
};


float animSample( const Anim &anim, int meshIndex, int channelIndex, float t, float def );
int   animTags( const Anim &anim, float prevT, float curT );
