#include "texture.h"
#include <stdlib.h>

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
 * @param {type} 
 * @return: 
 */
uint32_t get_pixel(const wg_texture_t *tex, uint32_t x, uint32_t y) {
  Assert(x >= 0 && x < tex->width && y >= 0 && y < tex->height, "Coord must be in bounds of texture.");
  uint32_t *buf = (uint32_t*)tex->buffer;
  return buf[x + y * tex->width];
}

/**
 * @description: Set texture to chessboard
 * @param {type} 
 * @return: 
 */
void set_checkboard_texture(wg_texture_t *tex, int u, int v, uint32_t c1, uint32_t c2) {
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