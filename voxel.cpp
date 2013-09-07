#include "voxel.h"
#include <malloc.h>
#include <memory.h>
#include "fpumath.h"

void init(map_s *m, int x, int y, int z)
{
	m->ofs[0] = 0.f;
	m->ofs[1] = 0.f;
	m->ofs[2] = 0.f;
	m->dim[0] = x;
	m->dim[1] = y;
	m->dim[2] = z;
	m->solid = (unsigned char*)malloc(x*y*z);
	m->colour = (uint32_t*)malloc(x*y*z*sizeof(uint32_t));
	m->entities = (ChunkArray<void*,16>*)malloc(x*y*sizeof(ChunkArray<void*,16>));
	memset( m->solid, 0, x*y*z );
	memset( m->colour, 0, x*y*z*sizeof(uint32_t) );
	memset( m->entities, 0, x*y*sizeof(ChunkArray<void*,16>) );
}

void setgeom(map_s *m, int x, int y, int z, unsigned char t)
{
	if ( x >= m->dim[0] || y >= m->dim[1] || z >= m->dim[2] )
		return;
	m->solid[(x*m->dim[2]*m->dim[1])+(y*m->dim[2]) + z] = t;
}

// need to convert for endianness here if we read 32-bits at a time
void setcolor(map_s *m, int x, int y, int z, uint32_t c)
{
	if ( x >= m->dim[0] || y >= m->dim[1] || z >= m->dim[2] )
		return;
	m->colour[(x*m->dim[2]*m->dim[1])+(y*m->dim[2]) + z] = (0xff<<24)|(((c>>0)&0xff)<<16)|(((c>>8)&0xff)<<8)|(((c>>16)&0xff)<<0);
}

void setcolor(map_s *m, int x, int y, int z, uint32_t r,uint32_t g,uint32_t b)
{
	if ( x >= m->dim[0] || y >= m->dim[1] || z >= m->dim[2] )
		return;
	m->colour[(x*m->dim[2]*m->dim[1])+(y*m->dim[2]) + z] = (0xff<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|(((b)&0xff)<<0);
}

bool issolid( map_s *m, int x, int y, int z )
{
	if ( x < 0 || y < 0 || z < 0 )
		return false;
	if ( x >= m->dim[0] || y >= m->dim[1] || z >= m->dim[2] )
		return false;
	return m->solid[(x*m->dim[2]*m->dim[1])+(y*m->dim[2]) + z] != 0;
}

void unlinkFromMap( map_s *map, void *p, float pos[3] )
{
	int x = (int)(pos[0]);
	int y = (int)(pos[1]);
	if ( x < 0 || x >= map->dim[0] )
		return;
	if ( y < 0 || y >= map->dim[1] )
		return;
	int index = (x*map->dim[1])+y;
	map->entities[index].Remove( p );
}

void linkToMap( map_s *map, void *p, float pos[3] )
{
	int x = (int)(pos[0]);
	int y = (int)(pos[1]);
	if ( x < 0 || x >= map->dim[0] )
		return;
	if ( y < 0 || y >= map->dim[1] )
		return;
	int index = (x*map->dim[1])+y;
	map->entities[index].Add( p );
}

bool IntersectRay(HitInfo_s &hi, float f[3], float d[3], float maxT, map_s *map, unsigned int mask )
{
    // Set up 3D DDA for ray
    float NextCrossingT[3], DeltaT[3];
    int Step[3], Pos[3], OOB[3];
    for (int axis = 0; axis < 3; ++axis) {
        // Compute current voxel for axis
        Pos[axis] = (int)(f[axis]);
        if (d[axis] >= 0) 
		{
            // Handle ray with positive direction for voxel stepping
            NextCrossingT[axis] = ((Pos[axis]+1.f) - f[axis]) / d[axis];
            DeltaT[axis] = 1.f / d[axis];
            Step[axis] = 1;
			OOB[axis] = map->dim[axis];
        }
        else {
            // Handle ray with negative direction for voxel stepping
            NextCrossingT[axis] = ((Pos[axis], axis) - f[axis]) / d[axis];
            DeltaT[axis] = -1.f / d[axis];
            Step[axis] = -1;
			OOB[axis] = -1;
        }
    }

    for (;;) 
	{
		int index = Pos[0] * map->dim[1] + Pos[1];
#if 0
		entitystate_s *cur = map->entities[index];
		while ( cur )
		{
			if ( cur->collisionMask & mask )
			{
				if ( (Pos[2] >= cur->pos[2]) && (Pos[2] <= (cur->pos[2]+cur->collisionHeight) ) )
				{
					hi.es = cur;
					hi.loc[0] = Pos[0];
					hi.loc[1] = Pos[1];
					hi.loc[2] = Pos[2];
					return true;
				}
			}
			cur = cur->voxelNext;
		}
#endif

		if ( issolid( map, Pos[0], Pos[1], Pos[2] ) )
		{
			hi.loc[0] = Pos[0];
			hi.loc[1] = Pos[1];
			hi.loc[2] = Pos[2];
			hi.es = NULL;
			return true;
		}

		// Find _stepAxis_ for stepping to next voxel
        int bits = ((NextCrossingT[0] < NextCrossingT[1]) << 2) +
                   ((NextCrossingT[0] < NextCrossingT[2]) << 1) +
                   ((NextCrossingT[1] < NextCrossingT[2]));
        const int cmpToAxis[8] = { 2, 1, 2, 1, 2, 2, 0, 0 };
        int stepAxis = cmpToAxis[bits];
		if (maxT < NextCrossingT[stepAxis])
			break;        
		Pos[stepAxis] += Step[stepAxis];
		if (Pos[stepAxis] == OOB[stepAxis])
			break;
        NextCrossingT[stepAxis] += DeltaT[stepAxis];
    }
	return false;
}

bool IntersectRay(HitInfo_s &hi, float f[3], float d[3], map_s *map, unsigned int mask  )
{
	float nd[3];
	vec3Norm( nd, d );
	return IntersectRay( hi, f, nd, sqrtf(vec3Dot(d,d)), map, mask );
}

bool IntersectFromTo(HitInfo_s &hi, float f[3], float t[3], map_s *map, unsigned int mask )
{
	float d[3], nd[3];
	vec3Sub( d, t, f );
	vec3Norm( nd, d );
	return IntersectRay( hi, f, nd, sqrtf(vec3Dot(d,d)), map, mask );
}

void move( float pos[3], float vel[3], map_s *map )
{
	// xy
	int attempts = 3;
	while ( attempts-- )
	{
		int nx = (int)(pos[0]+vel[0]);
		int ny = (int)(pos[1]+vel[1]);
		if ( issolid( map, nx, ny, (int)(pos[2] + 1.f) ) )
		{
			if ( nx != (int)pos[0] )
				vel[0] = 0.f;
			if ( ny != (int)pos[1] )
				vel[1] = 0.f;
		}
		else
		{
			break;
		}
	}

	pos[0] += vel[0];
	pos[1] += vel[1];
	if ( issolid( map, (int)pos[0], (int)pos[1], (int)pos[2] ) )
	{
		pos[2] += 1.f;
	}	

	if ( !issolid( map, (int)pos[0], (int)pos[1], (int)(pos[2] + vel[2]) ) )
	{
		pos[2] += vel[2];
	}
}
