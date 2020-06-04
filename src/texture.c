#include "texture.h"
#include <stdlib.h>
#include <math.h>

/**
 * @description: Returns an empty texture buffer. 
 * @param {width} The width of texture.
 * @param {height} The height of texture
 * @return: Pointer to a new texture buffer.
 */
wg_texture_t* get_empty_texture(uint32_t width, uint32_t height) {
  wg_texture_t* tex = (wg_texture_t*)malloc(sizeof(wg_texture_t));
  tex->width = width;
  tex->height = height;
  tex->len = (size_t)(width * height * 4);
  tex->buffer = (uint8_t*)malloc(tex->len);
  return tex;
}

/**
 * @description: Deletes a texture buffer. 
 * @param {ptex} A pointer to the texture pointer. 
 * @return: 
 */
void delete_texture(wg_texture_t **ptex) {
  wg_texture_t *tex = *ptex;
  free(tex->buffer);
  free(ptex);
}
/**
 * @description: Get pixel color of texture buffer.
 * @param {const wg_texture_t *tex} Texture.
 * @param {x, y} Target pixel.
 * @return: 
 */
uint32_t get_pixel(const wg_texture_t *tex, uint32_t x, uint32_t y) {
  Assert(x >= 0 && x < tex->width && y >= 0 && y < tex->height, "Coord must be in bounds of texture.");
  uint32_t *buf = (uint32_t*)tex->buffer;
  return buf[x + y * tex->width];
}

/**
 * @description: Set texture to chessboard
 * @param {wg_texture_t *tex} Output texture.
 * @param {u, v} Block number in x, y
 * @param {c1, c2} Block color. 
 * @return: 
 */
void set_chessboard_texture(wg_texture_t *tex, int u, int v, uint32_t c1, uint32_t c2) {
  uint32_t *buf = (uint32_t*)tex->buffer;
  for (int y = 0; y < tex->height; y ++) {
    for (int x = 0; x < tex->width; x ++) {
      if ((y / v + x / u) & 1) {
        buf[y * tex->width + x] = c1;
      } else {
        buf[y * tex->width + x] = c2;
      }
    }
  }
}

#define CLIP(x, l, r) ( \
  (x) < (l) ? (l) : \
              (x) > (r) ? (r) : \
                          (x) \
)

#define C_MUL_ADD(c0, c1, m) do {\
  c0.r += c1.r * (m);  \
  c0.g += c1.g * (m);  \
  c0.b += c1.b * (m);  \
} while(0)

/**
 * @description: Nearest texture sampler
 * @param {const wg_texture_t *tex} Texture
 * @param {flaot x, y} Position
 * @return: 
 */
wg_color_t sampler_nearest(const wg_texture_t *tex, float x, float y) {
  x = CLIP(x, 0.f, 1.f);
  y = CLIP(y, 0.f, 1.f);
  uint32_t ux = floorf(x * tex->width);
  uint32_t uy = floorf(y * tex->height);
  ux = ux < tex->width ? ux : tex->width - 1;
  uy = uy < tex->height ? uy : tex->height - 1;
  uint32_t c = get_pixel(tex, ux, uy);
  return color_cvt_uint2float(c);
}

/**
 * @description: Bilinear texture sampler
 * @param {const wg_texture_t *tex} Texture
 * @param {flaot x, y} Position
 * @return: 
 */
wg_color_t sampler_bilinear(const wg_texture_t *tex, float x, float y) {
  x = CLIP(x, 0.f, 1.f) * (tex->width - 1);
  y = CLIP(y, 0.f, 1.f) * (tex->height - 1);
  uint32_t x0 = floorf(x);
  uint32_t y0 = floorf(y);
  float u = x0 + 1. - x;
  float v = y0 + 1. - y;
  wg_color_t res = (wg_color_t){0., 0., 0.};
  wg_color_t c;
  c = color_cvt_uint2float(get_pixel(tex, x0, y0));
  C_MUL_ADD(res, c, u * v);
  c = color_cvt_uint2float(get_pixel(tex, x0 + 1, y0));
  C_MUL_ADD(res, c, (1. - u) * v);
  c = color_cvt_uint2float(get_pixel(tex, x0, y0 + 1));
  C_MUL_ADD(res, c, u * (1. - v));
  c = color_cvt_uint2float(get_pixel(tex, x0 + 1, y0 + 1));
  C_MUL_ADD(res, c, (1. - u) * (1. - v));
  return res;
}

#undef CLIP
#undef C_MUL_ADD

wg_color_t (*load_sampler(enum TEX_SAMPLE_MODE mode))(const wg_texture_t *tex, float x, float y) {
  if (mode == NEAREST) {
    return &sampler_nearest;
  } else if (mode == BILINEAR) {
    return &sampler_bilinear;
  } else {
    TODO();
  }
}
