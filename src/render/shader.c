#include "render.h"
#include <stdlib.h>
#include <math.h>
#include "common.h"

void shade_vertex(
  const wg_render_t *render, 
  wg_vertex_t *v, size_t size, 
  void (*vs)(const wg_render_t *render, wg_vertex_t *v)) {
  for (int i = 0; i < size; i ++) {
    (*vs)(render, v + i);
  }
}

void default_vs(const wg_render_t *render, wg_vertex_t *v) {
  matvecmul4(render->transform.transform_p, &v->vPos, &v->vPosH);
  matvecmul4(render->transform.transform, &v->vPos, &v->vPos);
  matvecmul4(render->transform.transform, &v->normal, &v->normal);
  transform_homogenous(&render->transform, v);
  vertex_init_rhw(v);
}

void shade_fragment(wg_render_t *render) {
  if (render->renderMode == FRAMEWORK) {
    TODO();
  } else if (render->renderMode == VERTEX_COLOR) {
    int w = render->width, h = render->height, len = w * h;
    for (int i = 0; i < len; i ++) {
      uint8_t *stencil = render->stencil + i;
      wg_gbuff_t *gbuff = render->gBuffer + i;
      gbuff->color = gbuff->vColor;
    }
  } else if (render->renderMode == SHADED) {
    TODO();
  }
}

static uint32_t rgb_to_uint(const wg_color_t *t) {
  int c[3];
  uint32_t res = 0;
  c[0] = t->r * 255;
  c[1] = t->g * 255;
  c[2] = t->b * 255;
  for (int i = 0; i < 3; i ++) {
    c[i] = c[i] > 255 ? 255 : c[i];
    c[i] = c[i] < 0 ? 0 : c[i];
    res += c[i] << (i << 3);
  }
  return res;
}

void shade_on_buffer(wg_render_t *render) {
  int w = render->width, h = render->height, len = w * h;
  for (int i = 0; i < len; i ++) {
    uint8_t *stencil = render->stencil + i;
    wg_gbuff_t *gbuff = render->gBuffer + i;
    uint32_t *fbuff = (uint32_t*)(render->frameBuffer + i * 4);
    if (*stencil > 0) {
      *fbuff = rgb_to_uint(&gbuff->color);
    } else {
      *fbuff = 0;
    }
  }
}
