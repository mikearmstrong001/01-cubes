#pragma once

#include <stdint.h>
#include "misc.h"

struct map_s
{
	float ofs[3];
	int dim[3];
	unsigned char *solid;
	uint32_t *colour;
	ChunkArray<void *,16> *entities;
};

struct HitInfo_s
{
	void *es;
	int loc[3];
};

void init(map_s *m, int x, int y, int z);
void setgeom(map_s *m, int x, int y, int z, unsigned char t);
void setcolor(map_s *m, int x, int y, int z, uint32_t c);
void setcolor(map_s *m, int x, int y, int z, uint32_t r,uint32_t g,uint32_t b);
bool issolid( map_s *m, int x, int y, int z );

void linkToMap( map_s *map, void *p, float pos[3] );
void unlinkFromMap( map_s *map, void *p, float pos[3] );

void move( float pos[3], float vel[3], map_s *map );

