#include "render.h"
#include <stdlib.h>
#include <math.h>

static wg_render_t *render = NULL;

static void try_init_render() {
  if (render == NULL) {
    render = (wg_render_t *)malloc(sizeof(wg_render_t));
    render->fshaderName = "default";
    render->stencil = NULL;
    render->frameBuffer = NULL;
    render->zBuffer = NULL;
    render->gBuffer = NULL;
    wg_transform_t *t = &(render->transform);
    t->world = NULL;
    t->camera = NULL;
    t->projection = NULL;
    t->transform = NULL;
    t->transform_p = NULL;
  }
}

wg_render_t* get_render() {
  try_init_render();
  return render;
}

void set_up_render(wg_render_t *render, int width, int height) {
  render->width = width;
  render->height = height;
  render->stencil = (uint8_t*)malloc(width * height * sizeof(uint8_t));
  render->frameBuffer = (uint8_t*)malloc(width * height * sizeof(uint32_t));
  render->zBuffer = (float*)malloc(width * height * sizeof(float));
  render->gBuffer = (wg_gbuff_t*)malloc(width * height * sizeof(wg_gbuff_t));
  wg_transform_t *t = &(render->transform);
  t->transform = (wg_mat44f*)malloc(sizeof(wg_mat44f));
  t->transform_p = (wg_mat44f*)malloc(sizeof(wg_mat44f));
  t->w = width;
  t->h = height;
}

void clear_render(wg_render_t *render) {
  int len = render->width * render->height;
  for (int i = 0; i < len; i ++) render->stencil[i] = 0;
  for (int i = 0; i < len * 4; i ++) render->frameBuffer[i] = 0;
  for (int i = 0; i < len; i ++) render->zBuffer[i] = 1.;
}

void set_light(wg_render_t *render, wg_light_t light) {
  render->light.color = light.color;
  matvecmul4(render->transform.transform, &light.position, &render->light.position);
}