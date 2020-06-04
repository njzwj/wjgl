#ifndef __RENDER_H__
#define __RENDER_H__

#include <stdint.h>
#include "common.h"
#include "geom.h"
#include "texture.h"

enum RENDER_MODE {
  FRAMEWORK = 1,
  VERTEX_COLOR,
  SHADED,
};

typedef struct {
  /* Render mode */
  enum RENDER_MODE renderMode;
  enum TEX_SAMPLE_MODE sampleMode;
  
  /* Color: edge, fill. 
     only takes effect when render mode = FRAMEWORK */
  uint32_t colorEdge, colorFill;

  /* Frame width, height */
  uint32_t width, height;

  /* Transform */
  wg_transform_t transform;

  /* Texture */
  wg_texture_t *texture;

  /* buffers */
  uint8_t *stencil;
  uint8_t *frameBuffer;
  float *zBuffer;
  wg_gbuff_t *gBuffer;
} wg_render_t;

wg_render_t* get_render();

void set_up_render(wg_render_t *render, int width, int height);

void clear_render(wg_render_t *render);

void shade_vertex(
  const wg_render_t *render, 
  wg_vertex_t *v, size_t size, 
  void (*vs)(const wg_render_t *render, wg_vertex_t *v));

void default_vs(const wg_render_t *render, wg_vertex_t *v);

void project_vertexes(
  const wg_render_t *render, 
  wg_vertex_t *v, size_t size
);

void cull_and_draw_triangle(
  const wg_render_t *render,
  const wg_vertex_t *v1,
  const wg_vertex_t *v2,
  const wg_vertex_t *v3
);

void shade_fragment(wg_render_t *render);

void shade_on_buffer(wg_render_t *render);

#endif