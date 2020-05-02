#include "render.h"
#include <stdlib.h>
#include <math.h>

typedef struct {
  /* left edge: v1 -> v2 right edge: v3 -> v4. up->down */
  wg_vertex_t v1, v2, v3, v4;

  /* top & bottom line */
  float top, bottom;
} wg_trapezoid_t;

typedef struct {
  /* Starting vertex & vertex step */
  wg_vertex_t v, step;

  /* scanline starting point & width */
  int x, y, w;
} wg_scanline_t;

static void draw_trapezoid(
  wg_render_t *const render,
  wg_trapezoid_t *const t
);

static void draw_scanline(
  wg_render_t *const render,
  wg_scanline_t *const s
);

static void swap_ptr(void **a, void **b) {
  void *c = *a;
  *a = *b;
  *b = c;
}

static int map_coord_to_offset(wg_render_t *const render, int x, int y) {
  return render->width * y + x;
}

static int split_trapezoid(
  wg_vertex_t *const v1,
  wg_vertex_t *const v2,
  wg_vertex_t *const v3,
  wg_trapezoid_t *t1,
  wg_trapezoid_t *t2
) {
  wg_vertex_t vv4;
  wg_vertex_t *v4 = &vv4;
  // y1 < y2 < y3
  if (v1->vPosH.y > v2->vPosH.y) swap_ptr((void**)(&v1), (void**)(&v2));
  if (v1->vPosH.y > v3->vPosH.y) swap_ptr((void**)(&v1), (void**)(&v3));
  if (v2->vPosH.y > v3->vPosH.y) swap_ptr((void**)(&v2), (void**)(&v3));
  if (v1->vPosH.y == v2->vPosH.y) {
    if (v1->vPosH.x > v2 -> vPosH.x) swap_ptr((void**)(&v1), (void**)(&v2));
    *t1 = (wg_trapezoid_t){*v1, *v3, *v2, *v3, v1->vPosH.y, v3->vPosH.y};
    return 1;
  }
  if (v2->vPosH.y == v3->vPosH.y) {
    if (v2->vPosH.x > v3->vPosH.x) swap_ptr((void**)(&v2), (void**)(&v3));
    *t1 = (wg_trapezoid_t){*v1, *v2, *v1, *v3, v1->vPosH.y, v2->vPosH.y};
    return 1;
  }
  float _ratio = (v2->vPosH.y - v1->vPosH.y) / (v3->vPosH.y - v1->vPosH.y);
  vertex_interp(v4, v1, v3, _ratio);
  if (v2->vPosH.x > v4->vPosH.x) swap_ptr((void**)(&v2), (void**)(&v4));
  *t1 = (wg_trapezoid_t){*v1, *v2, *v1, *v4, v1->vPosH.y, v2->vPosH.y};
  *t2 = (wg_trapezoid_t){*v2, *v3, *v4, *v3, v2->vPosH.y, v3->vPosH.y};
  return 2;
}

void draw_triangle(
  wg_render_t *const render,
  wg_vertex_t *const v1,
  wg_vertex_t *const v2,
  wg_vertex_t *const v3
) {
  wg_trapezoid_t t1, t2;
  int n_trape = split_trapezoid(v1, v2, v3, &t1, &t2);
  draw_trapezoid(render, &t1);
  if (n_trape > 1) {
    draw_trapezoid(render, &t2);
  }
}

static void draw_trapezoid(
  wg_render_t *const render,
  wg_trapezoid_t *const t
) {
  wg_vertex_t l, r, step, start;
  wg_scanline_t scanline;
  float ystep = 1.0f / (t->bottom - t->top);
  float yratio = (t->v1.vPosH.y - t->top) / (t->bottom - t->top);
  for (int y = (int)ceilf(t->top); y < t->bottom; y ++, yratio += ystep) {
    vertex_interp(&l, &(t->v1), &(t->v2), yratio);
    vertex_interp(&r, &(t->v3), &(t->v4), yratio);
    vertex_step(&step, &l, &r);
    float x = ceilf(l.vPosH.x), w = ceilf(r.vPosH.x) - ceilf(l.vPosH.x);
    float _r = (x-l.vPosH.x) / (r.vPosH.x-l.vPosH.x);
    vertex_interp(&start, &l, &r, _r);
    scanline = (wg_scanline_t){start, step, (int)x, (int)y, (int)w};
    draw_scanline(render, &scanline);
  }
}

static void draw_scanline(
  wg_render_t *const render,
  wg_scanline_t *const s
) {
  int x = s->x, y = s->y;
  wg_vertex_t v = s->v;
  wg_gbuff_t *geom;
  uint8_t *stencil;
  float *depth;
  for (int i = 0; i < s->w; i ++) {
    if (x < 0 || x >= render->width) continue;
    float z = v.vPosH.z, w = 1. / v.rhw;
    depth = render->zBuffer + map_coord_to_offset(render, x, y);
    if (z < *depth) {
      stencil = render->stencil + map_coord_to_offset(render, x, y);
      *depth = z;
      *stencil = 1;
      geom = render->gBuffer + map_coord_to_offset(render, x, y);
      geom->vPosH = v.vPosH;
      geom->normal = v4f_mul(v.normal, w);
      geom->tc = (wg_txcoord_t){v.tc.x * w, v.tc.y * w};
      geom->vColor = (wg_color_t){v.vColor.r * w, v.vColor.g * w, v.vColor.b * w};
    }
    vertex_add(&v, &(s->step));
    x ++;
  }
}