#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdint.h>
#include "common.h"
#include "geom.h"

enum TEX_SAMPLE_MODE {
  NEAREST = 0,
  BILINEAR
};

typedef struct {
  uint32_t width, height;
  size_t len;
  uint8_t *buffer;
} wg_texture_t;

wg_texture_t* get_empty_texture(uint32_t width, uint32_t height);

void delete_texture(wg_texture_t **tex);

uint32_t get_pixel(const wg_texture_t *tex, uint32_t x, uint32_t y);

void set_chessboard_texture(wg_texture_t *tex, int u, int v, uint32_t c1, uint32_t c2);

wg_color_t sampler_nearest(const wg_texture_t *tex, float x, float y);

wg_color_t sampler_bilinear(const wg_texture_t *tex, float x, float y);

wg_color_t (*load_sampler(enum TEX_SAMPLE_MODE mode))(const wg_texture_t *tex, float x, float y);

wg_color_t gamma_trans(const wg_color_t *c, float pow);

#endif