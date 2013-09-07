#pragma once

#include <stdint.h>

void load_vxl_memory(struct map_s *map, uint8_t *v);
void load_vxl_file(struct map_s *map, const char*filename);
