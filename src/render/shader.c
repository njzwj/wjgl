#include "render.h"
#include "common.h"
#include "texture.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// TODO: a fragment shader register
//   LIST: shader name -> shader function pointer
//   REGISTER (shader name, shader function pointer)

#define MAX_SHADER_NUM 256

typedef struct {
  const char* name[MAX_SHADER_NUM];
  const void* value[MAX_SHADER_NUM];
  size_t size;
} wg_register_t;

wg_register_t frag_shader_reg;

wg_fshader_t* get_frag_shader(const char* name);

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
  int w = render->width, h = render->height, len = w * h;
  if (render->renderMode == FRAMEWORK) {
    TODO();
  } else if (render->renderMode == VERTEX_COLOR) {
    for (int i = 0; i < len; i ++) {
      uint8_t *stencil = render->stencil + i;
      wg_gbuff_t *gbuff = render->gBuffer + i;
      gbuff->color = gbuff->vColor;
    }
  } else if (render->renderMode == SHADED) {
    Assert(render->texture != NULL, "Texture cannot be NULL in SHADE mode.");
    wg_color_t (*t_sampler)(const wg_texture_t *tex, float x, float y) = load_sampler(render->sampleMode);
    wg_fshader_t* fshader = get_frag_shader(render->fshaderName);
    Assert(fshader != NULL, "Shader %s doesn't exist.", render->fshaderName);
    for (int i = 0; i < len; i ++) {
      uint8_t *stencil = render->stencil + i;
      wg_gbuff_t *gbuff = render->gBuffer + i;
      if (*stencil > 0) {
        // sample texture
        gbuff->diffuseColor = (*t_sampler)(render->texture, gbuff->tc.x, gbuff->tc.y);
        // shade fragment
        (*fshader)(render, gbuff);
      }
    }
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

static void default_fshader(const wg_render_t* render, wg_gbuff_t* gbuff) {
  gbuff->color = gbuff->diffuseColor;
}

/**
 * @description: Initialize fragment shader register.
 */
void init_frag_shader_reg() {
  frag_shader_reg.size = 0;
  register_frag_shader("default", &default_fshader);
}

/**
 * @description: Register a user-defined fragment shader.
 * @param {const char* name} The name of shader. MUST BE UNIQUE.
 * @param {wg_fshader_t* shader} The shader function.
 */
void register_frag_shader(const char* name, wg_fshader_t* shader) {
  Assert(frag_shader_reg.size < MAX_SHADER_NUM, "Shader num out of bounds. Current MAX %d.", MAX_SHADER_NUM);
  wg_fshader_t* t = get_frag_shader(name);
  Assert(t == NULL, "Shader name MUST be unique! Name %s has been taken.", name);
  frag_shader_reg.name[frag_shader_reg.size] = name;
  frag_shader_reg.value[frag_shader_reg.size] = (void*)shader;
  frag_shader_reg.size ++;
  Log("Fragment shader %s has been successfully registed.", name);
}

/**
 * @description: Get a user-defined fragment shader.
 * @param {const char* name} The name of shader. 
 * @return: {wg_fshader_t*} The shader function required.
 */
wg_fshader_t* get_frag_shader(const char* name) {
  for (int i = 0; i < frag_shader_reg.size; i ++) {
    if (strcmp(name, frag_shader_reg.name[i]) == 0) {
      return (wg_fshader_t*)frag_shader_reg.value[i];
    }
  }
  return (wg_fshader_t*)NULL;
}