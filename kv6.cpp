#include "kv6.h"
#include "voxel.h"

struct kv6block {
	uint32_t color; // 0xLLRRGGBB
	uint16_t zpos;
	uint8_t visfaces;
	uint8_t lighting; // not sure what value corresponds to what direction here
};

struct kv6hdr {
	char magic_kvxl[4]; // "Kvxl"
	uint32_t xsiz, ysiz, zsiz;
	float xpivot, ypivot, zpivot;
	
	uint32_t blklen;
	kv6block blks[1];
};

struct kv6pal
{
	char magic_spal[4]; // "SPal"
	char palette[256][3]; // 256 R,G,B entries, all 0 <= x <= 63 as per VGA
};

void load_kv6(map_s *map, uint8_t *v, int len)
{
	kv6hdr *hdr = (kv6hdr *)v;
	uint32_t *xoffset = (uint32_t*)&hdr->blks[hdr->blklen];
	uint16_t *xyoffset = (uint16_t*)&xoffset[hdr->xsiz];
	kv6pal *pal = (kv6pal *)&xyoffset[hdr->xsiz*hdr->ysiz];
	init( map, hdr->xsiz, hdr->ysiz, hdr->zsiz );
	map->ofs[0] = hdr->xpivot;
	map->ofs[1] = hdr->ypivot;
	map->ofs[2] = hdr->zsiz - hdr->zpivot - 1;
	kv6block *block = hdr->blks;
	for ( int x=0; x<hdr->xsiz; x++ )
	{
		for ( int y=0; y<hdr->ysiz; y++ )
		{
			int num = xyoffset[x*hdr->ysiz+y];
			for ( int n=0; n<num; n++ )
			{
				setgeom( map, x, y, hdr->zsiz-block->zpos-1, 1 );
				setcolor( map, x, y, hdr->zsiz-block->zpos-1, block->color );
				block++;
			}
		}
	}
}

