#include "cub.h"
#include "voxel.h"

struct cubhdr
{
	int xsiz, ysiz, zsiz;
	unsigned char rgb[3];
};

void load_cub(map_s *map, uint8_t *v, int /*len*/)
{
	cubhdr *hdr = (cubhdr *)v;
	init( map, hdr->xsiz, hdr->ysiz, hdr->zsiz );
	map->ofs[0] = 0.f;
	map->ofs[1] = 0.f;
	map->ofs[2] = 0.f;
	unsigned char *rgb = hdr->rgb;
	for ( int z=0; z<hdr->zsiz; z++ )
	{
		for ( int y=0; y<hdr->ysiz; y++ )
		{
			for ( int x=0; x<hdr->xsiz; x++ )
			{
				if ( rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 0 )
				{
					setgeom( map, x, y, z, 0 );
				}
				else
				{
					setgeom( map, x, y, z, 1 );
					setcolor( map, x, y, z, rgb[2], rgb[1], rgb[0] );
				}
				rgb+=3;
			}
		}
	}
}

