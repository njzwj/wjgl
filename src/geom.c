#include "geom.h"
#include <math.h>

const float PI = 3.1415926;

wg_vec4f v4f_add(wg_vec4f a, wg_vec4f b) {
  return (wg_vec4f){ {{a.x + b.x, a.y + b.y, a.z + b.z, 1.0f}} };
}

wg_vec4f v4f_sub(wg_vec4f a, wg_vec4f b) {
  return (wg_vec4f){ {{ a.x - b.x, a.y - b.y, a.z - b.z, 1.0f }} };
}

wg_vec4f v4f_mul(wg_vec4f a, float b) {
  return (wg_vec4f){ {{ a.x * b, a.y * b, a.z * b, 1.0f }} };
}

wg_vec4f v4f_div(wg_vec4f a, float b) {
  float inv = 1.0f / b;
  return (wg_vec4f){ {{ a.x * inv, a.y * inv, a.z * inv, 1.0f }} };
}

float v4f_dot_prod(wg_vec4f a, wg_vec4f b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

wg_vec4f v4f_cross_prod(wg_vec4f a, wg_vec4f b) {
  float x, y, z;
  x = a.y * b.z - a.z * b.y;
  y = a.z * b.x - a.x * b.z;
  z = a.x * b.y - a.y * b.x;
  return (wg_vec4f){ {{x, y, z, 1.0f}} };
}

float lerp(float a, float b, float x) {
  return (1.0f - x) * a + x * b;
}

wg_vec4f lerp_vec4f(wg_vec4f a, wg_vec4f b, float x) {
  return (wg_vec4f){ {{
    lerp(a.x, b.x, x),
    lerp(a.y, b.y, x),
    lerp(a.z, b.z, x),
    lerp(a.w, b.w, x),
  }} };
}

void normalize_vec4f(wg_vec4f *a) {
  float inv = 0.;
  for (int i = 0; i < 3; i ++) inv += a->v[i] * a->v[i];
  inv = 1. / sqrtf(inv);
  a->x *= inv;
  a->y *= inv;
  a->z *= inv;
  a->w = 1.0f;
}

void matmul(const wg_mat44f *a, const wg_mat44f *b, wg_mat44f *y) {
  float tmp[4][4];
  for (int i = 0; i < 4; i ++) {
    for (int j = 0; j < 4; j ++) {
      tmp[i][j] = 0.;
      for (int k = 0; k < 4; k ++) {
        tmp[i][j] += a->m[i][k] * b->m[k][j];
      }
    }
  }
  for (int i = 0; i < 4; i ++) {
    for (int j = 0; j < 4; j ++) {
      y->m[i][j] = tmp[i][j];
    }
  }
}

void matvecmul4(const wg_mat44f *m, const wg_vec4f *b, wg_vec4f *y) {
  float tmp[4];
  for (int i = 0; i < 4; i ++) {
    tmp[i] = 0;
    for (int j = 0; j < 4; j ++) {
      tmp[i] += m->m[i][j] * b->v[j];
    }
  }
  for (int i = 0; i < 4; i ++) y->v[i] = tmp[i];
}

/**
 * @description: Get projection matrix.
 * <pre>
 * 1 / fH         0               0               0
 * 0              1 / fH *aR      0               0
 * 0              0               f/(n-f)         nf/(n-f)
 * 0              0               -1              0
 * </pre>
 * This matrix can project x, y, z, 1 that x' y' in [-1, 1] and z' in [0, 1], w' = -z
 * @param {wg_mat44f *m} Pointer to the matrix to be modified.
 * @param {float fovH} Horizontal field-of-view
 * @param {aspectRatio}
 * @param {near} Distance between Origin to near plane (note that this should be positive)
 * @param {far}  Distance between Origin to far plane (should also be positive)
 * @return: 
 */
void get_projection_mat(wg_mat44f *m, float fovH, float aspectRatio, float near, float far) {
  fovH = tanf(fovH * 0.5 * PI / 180.0);
  for (int i = 0; i < 16; i ++) m->v[i] = 0.;
  m->m[0][0] = 1. / fovH;
  m->m[1][1] = 1. / fovH * aspectRatio;
  m->m[2][2] = far / (near - far); m->m[2][3] = near * far / (near - far);
  m->m[3][2] = -1.;
}

void get_translation_mat(wg_mat44f *m, float dx, float dy, float dz) {
  for (int i = 0; i < 16; i ++) m->v[i] = 0.;
  m->_11 = m->_22 = m->_33 = m->_44 = 1.;
  m->_14 = dx; m->_24 = dy; m->_34 = dz;
}

void get_lookat_mat(wg_mat44f *m, wg_point_t eye, wg_point_t center, wg_point_t up) {
  wg_point_t z = v4f_sub(eye, center);
  normalize_vec4f(&z);
  wg_point_t x = v4f_cross_prod(up, z);
  normalize_vec4f(&z);
  wg_point_t y = v4f_cross_prod(z, x);
  normalize_vec4f(&y);
  for (int i = 0; i < 3; i ++) {
    m->m[0][i] = x.v[i];
    m->m[1][i] = y.v[i];
    m->m[2][i] = z.v[i];
    m->m[3][i] = 0.;
  }
  for (int i = 0; i < 3; i ++) {
    m->m[i][3] = 0.;
    for (int j = 0; j < 3; j ++) {
      m->m[i][3] -= eye.v[j] * m->m[i][j];
    }
  }
  m->_44 = 1.;
}

void get_identical_mat(wg_mat44f *m) {
  for (int i = 0; i < 16; i ++) m->v[i] = 0.0f;
  m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
}

void transform_update(wg_transform_t *t) {
  matmul(t->camera, t->world, t->transform);
  matmul(t->projection, t->transform, t->transform_p);
}

void transform_apply(const wg_transform_t *t, wg_point_t *y, const wg_point_t *x) {
  matvecmul4(t->transform, x, y);
}

void transform_homogenous(const wg_transform_t *t, wg_vertex_t *x) {
  wg_vec4f *posH = &(x->vPosH);
  float rhz = 1.0f / posH->w;
  posH->x *= rhz;
  posH->y *= rhz;
  posH->z *= rhz;
  posH->w = rhz;
  // transform into screen size
  posH->x = (posH->x * 0.5f + 0.5f) * t->w;
  posH->y = (0.5f - posH->y * 0.5f) * t->h;
}

void vertex_init_rhw(wg_vertex_t *x) {
  x->rhw = x->vPosH.w;
  float rhw = x->rhw;
  for (int i = 0; i < 3; i ++) x->vPos.v[i] *= rhw;
  for (int i = 0; i < 3; i ++) x->normal.v[i] *= rhw;
  x->tc.x *= rhw;
  x->tc.y *= rhw;
  x->vColor.r *= rhw;
  x->vColor.g *= rhw;
  x->vColor.b *= rhw;
}

void vertex_add(wg_vertex_t *y, const wg_vertex_t *x) {
  for (int i = 0; i < 4; i ++) y->vPosH.v[i] += x->vPosH.v[i];
  for (int i = 0; i < 3; i ++) y->vPos.v[i] += x->vPos.v[i];
  for (int i = 0; i < 3; i ++) y->normal.v[i] += x->normal.v[i];
  y->tc.x += x->tc.x;
  y->tc.y += x->tc.y;
  y->vColor.r += x->vColor.r;
  y->vColor.g += x->vColor.g;
  y->vColor.b += x->vColor.b;
  y->rhw += x->rhw;
}

void vertex_sub(wg_vertex_t *y, const wg_vertex_t *x) {
  for (int i = 0; i < 4; i ++) y->vPosH.v[i] -= x->vPosH.v[i];
  for (int i = 0; i < 3; i ++) y->vPos.v[i] -= x->vPos.v[i];
  for (int i = 0; i < 3; i ++) y->normal.v[i] -= x->normal.v[i];
  y->tc.x -= x->tc.x;
  y->tc.y -= x->tc.y;
  y->vColor.r -= x->vColor.r;
  y->vColor.g -= x->vColor.g;
  y->vColor.b -= x->vColor.b;
  y->rhw -= x->rhw;
}

void vertex_scale(wg_vertex_t *y, float x) {
  for (int i = 0; i < 4; i ++) y->vPosH.v[i] *= x;
  for (int i = 0; i < 3; i ++) y->vPos.v[i] *= x;
  for (int i = 0; i < 3; i ++) y->normal.v[i] *= x;
  y->tc.x *= x;
  y->tc.y *= x;
  y->vColor.r *= x;
  y->vColor.g *= x;
  y->vColor.b *= x;
  y->rhw *= x;
}

void vertex_step(wg_vertex_t *step, const wg_vertex_t *l, const wg_vertex_t *r) {
  *step = *r;
  vertex_sub(step, l);
  vertex_scale(step, 1. / (r->vPosH.x - l->vPosH.x + 1e-6));
}

void vertex_interp(wg_vertex_t *v, const wg_vertex_t *v1, const wg_vertex_t *v2, float x) {
  v->vPosH = lerp_vec4f(v1->vPosH, v2->vPosH, x);
  v->vPos = lerp_vec4f(v1->vPos, v2->vPos, x);
  v->normal = lerp_vec4f(v1->normal, v2->normal, x);
  v->tc.x = lerp(v1->tc.x, v2->tc.x, x);
  v->tc.y = lerp(v1->tc.y, v2->tc.y, x);
  v->vColor.r = lerp(v1->vColor.r, v2->vColor.r, x);
  v->vColor.g = lerp(v1->vColor.g, v2->vColor.g, x);
  v->vColor.b = lerp(v1->vColor.b, v2->vColor.b, x);
  v->rhw = lerp(v1->rhw, v2->rhw, x);
}

uint32_t color_cvt_float2uint(const wg_color_t c) {
#define CLIP(x, l, h) ((x)>(l)?(x)<(h)?(x):(h):(l))
  float r = CLIP(c.r, 0., 1.);
  float g = CLIP(c.g, 0., 1.);
  float b = CLIP(c.b, 0., 1.);
  uint32_t res = 0;
  res |= (int)(r * 255.);
  res |= (int)(g * 255.) << 8;
  res |= (int)(b * 255.) << 16;
  return res;
#undef CLIP
}

wg_color_t color_cvt_uint2float(const uint32_t c) {
  float r = c & 255;
  float g = (c >> 8) & 255;
  float b = (c >> 16) & 255;
  return (wg_color_t){r / 255., g / 255., b / 255.};
}
