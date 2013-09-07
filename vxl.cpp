#include "voxel.h"
#include "misc.h"

void load_vxl_memory(map_s *map, uint8_t *v)
{
	init( map, 512, 512, 64 );
	uint8_t *base = v;
	int x,y,z;
	for (y=0; y < 512; ++y)
	{
		for (x=0; x < 512; ++x) 
		{
			for (z=0; z < 64; ++z) 
			{
				setgeom(map, x,y,64-z-1,1);
			}

			z = 0;

			for(;;) 
			{
				uint8_t *color;
				int i;
				int number_4byte_chunks = v[0];
				int top_color_start = v[1];
				int top_color_end   = v[2]; // inclusive
				int bottom_color_start;
				int bottom_color_end; // exclusive
				int len_top;
				int len_bottom;

				for(i=z; i < top_color_start; i++)
				{
					setgeom(map,x,y,64-i-1,0);
				}

				color = (v+4);
				for(z=top_color_start; z <= top_color_end; z++)
				{
					color[3] = 255;
					setcolor(map, x,y,64-z-1,(color[0]*color[3])>>8,(color[1]*color[3])>>8,(color[2]*color[3])>>8);
					color+=4;
				}

				len_bottom = top_color_end - top_color_start + 1;

				// check for end of data marker
				if (number_4byte_chunks == 0) 
				{
					// infer ACTUAL number of 4-byte chunks from the length of the color data
					v += 4 * (len_bottom + 1);
					break;
				}

				// infer the number of bottom colors in next span from chunk length
				len_top = (number_4byte_chunks-1) - len_bottom;

				// now skip the v pointer past the data to the beginning of the next span
				v += v[0]*4;

				bottom_color_end   = v[3]; // aka air start
				bottom_color_start = bottom_color_end - len_top;

				for(z=bottom_color_start; z < bottom_color_end; ++z) 
				{
	//				setcolor(map, x,y,z,color[0],color[1],color[2]);
					setcolor(map, x,y,64-z-1,(color[0]*color[3])>>8,(color[1]*color[3])>>8,(color[2]*color[3])>>8);
					color+=4;
				}
			}
		}
	}
	//assert(v-base == len);
}

void load_vxl_file(map_s *map, const char *filename)
{
	void *vxl = fload( filename );
	load_vxl_memory(map, (uint8_t*)vxl);
	free(vxl);
}
