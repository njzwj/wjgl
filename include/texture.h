#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdint.h>
#include "common.h"

typedef struct {
  uint32_t width, height;
  size_t len;
  uint8_t *buffer;
} wg_texture_t;

wg_texture_t* get_empty_texture(uint32_t width, uint32_t height);

void delete_texture(wg_texture_t **tex);

uint32_t get_pixel(wg_texture_t *const tex, uint32_t x, uint32_t y);

void set_checkboard_texture(wg_texture_t *tex, int u, int v, uint32_t c1, uint32_t c2);



#endif