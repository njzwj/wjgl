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
  const wg_render_t *render,
  const wg_trapezoid_t *t
);

static void draw_triangle(
  const wg_render_t *render,
  const wg_vertex_t *v1,
  const wg_vertex_t *v2,
  const wg_vertex_t *v3
);

static void draw_scanline(
  const wg_render_t *render,
  const wg_scanline_t *s
);

static void swap_ptr(void **a, void **b) {
  void *c = *a;
  *a = *b;
  *b = c;
}

/**
 * @description: Returns the point position relative to boundaries.
 *  high -> low
 *  top bottom right left far near
 *  near:   z < 0
 *  far:    z >  w
 *  left:   x < -w
 *  right:  x >  w
 *  bottom: y < -w
 *  top:    y >  w
 * @param {const wg_point_t *p} 
 * @return: int that represents current status.
 */
static int check_cvv(const wg_point_t *p) {
  float w = p->w;
  int res = 0;
  if (p->z < 0.) res |= 1;
  if (p->z >  w) res |= 2;
  if (p->x < -w) res |= 4;
  if (p->x >  w) res |= 8;
  if (p->y < -w) res |= 16;
  if (p->y >  w) res |= 32;
  return res;
}

/**
 * @description: Returns if directional line v1->v2 intersects plane (x y z w) X = 0.
 * @param {*v1} Pointer to vector 1.
 * @param {*v2} Pointer to vector 2.
 * @param {x, y, z, w} Parameters that define culling plane.
 * @param {*inter} Inter ratio raletive to v1->v2. 
 * @return: int. 1 if intersects.
 */
static int line_border_inter(const wg_point_t *v1, const wg_point_t *v2, float x, float y, float z, float w, float *ratio) {
  float dist1 = x * v1->x + y * v1->y + z * v1->z + w * v1->w;
  float dist2 = x * v2->x + y * v2->y + z * v2->z + w * v2->w;
  if (dist1 * dist2 > 0.f) return 0;
  *ratio = dist1 / (dist1 - dist2);
  *ratio = *ratio < 0.f ? -*ratio: *ratio;
  Assert(*ratio <= 1. && *ratio >= 0., "Ratio must be in [0, 1].");
  return 1;
}

static int map_coord_to_offset(const wg_render_t *render, int x, int y) {
  return render->width * y + x;
}

static int split_trapezoid(
  const wg_vertex_t *v1,
  const wg_vertex_t *v2,
  const wg_vertex_t *v3,
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

/**
 * @description: Projects a single vertex (without screen space div)
 * @param {const wg_render_t *render} Render pointer.
 * @param {wg_vertex_t *v} Vertex to be projected.
 */
static void project_vertex(const wg_render_t *render, wg_vertex_t *v) {
  matvecmul4(render->transform.transform_p, &v->vPos, &v->vPosH);
  matvecmul4(render->transform.transform, &v->vPos, &v->vPos);
  matvecmul4(render->transform.transform, &v->normal, &v->normal);
}

void project_vertexes(
  const wg_render_t *render, 
  wg_vertex_t *v, size_t size
) {
  for (int i = 0; i < size; i ++) {
    project_vertex(render, v + i);
  }
}

/**
 * @description: Cull and draw triangle.
 * vertex position is unnormalized homogeunous pos.
 * @param {render}
 * @param {v1}
 * @param {v2}
 * @param {v3} 
 */
void cull_and_draw_triangle(
  const wg_render_t *render,
  const wg_vertex_t *v1,
  const wg_vertex_t *v2,
  const wg_vertex_t *v3
) {
  // near plane culling
  wg_point_t *p1 = &v1->vPosH, *p2 = &v2->vPosH, *p3 = &v3->vPosH;
  int cp1 = check_cvv(p1) & 1;
  int cp2 = check_cvv(p2) & 1;
  int cp3 = check_cvv(p3) & 1;
  Log("Position of vertex: %d %d %d", cp1, cp2, cp3);
  Log("Vertex 1 posH (x, y, z, w) = %.4f %.4f %.4f %.4f", v1->vPosH.x, \
      v1->vPosH.y, v1->vPosH.z, v1->vPosH.w);
  Log("Vertex 2 posH (x, y, z, w) = %.4f %.4f %.4f %.4f", v2->vPosH.x, \
      v2->vPosH.y, v2->vPosH.z, v2->vPosH.w);
  Log("Vertex 1 pos (x, y, z, w) = %.4f %.4f %.4f %.4f", v1->vPos.x, \
      v1->vPos.y, v1->vPos.z, v1->vPos.w);
  // Will only deal with near plane culling because only when w is smaller than 0 most of 
  //   the rasterization algorithms followed will have issue to run correctly.
  // Also, by doing this, the alogorithm won't be too complicated.
  // Why clip by z=0? Because this will avoid the "divide by negative z error" in the next
  //   few steps.
  wg_vertex_t v[4];         // After culling using z == 0, there are at most 4 different vertexes.
  int n_vertex = 0;             // Number of vertexes.
  
  // Note that in triangle v1 -> v2 -> v3, new vertexes of triangle or quad must be continuous 
  //   in v1 -> inter -> v2 -> inter -> v3 -> inter. Additionally, inter vertexes must be less
  //   than or equal to 2.
  int f;
  float ratio;
  if (cp1 == 0) v[n_vertex++] = *v1;
  f = line_border_inter(&v1->vPosH, &v2->vPosH, 0., 0., 1., 0., &ratio);
  if (f) {
    vertex_interp(&v[n_vertex], v1, v2, ratio);
    Log("First ratio %f", ratio);
    ++ n_vertex;
  }
  if (cp2 == 0) v[n_vertex++] = *v2;
  f = line_border_inter(&v2->vPosH, &v3->vPosH, 0., 0., 1., 0., &ratio);
  if (f) {
    vertex_interp(&v[n_vertex], v2, v3, ratio);
    ++ n_vertex;
  }
  if (cp3 == 0) v[n_vertex++] = *v3;
  f = line_border_inter(&v3->vPosH, &v1->vPosH, 0., 0., 1., 0., &ratio);
  if (f) {
    vertex_interp(&v[n_vertex], v3, v1, ratio);
    ++n_vertex;
  }
  Assert(n_vertex <= 4, "Number of vertexes should be <= 4 after culling!");
  Log("Number of vertex: %d", n_vertex);
  Log("Vertex 1 after culling posH (x, y, z, w) = %.4f %.4f %.4f %.4f", v[0].vPosH.x, \
      v[0].vPosH.y, v[0].vPosH.z, v[0].vPosH.w);
  for (int i = 0; i < n_vertex; i ++) {
    transform_homogenous(&render->transform, &v[i]);
    vertex_init_rhw(&v[i]);
  }

  if (n_vertex <= 2) return;
  if (n_vertex >= 3) {
    draw_triangle(render, &v[0], &v[1], &v[2]);
  }
  if (n_vertex == 4) {
    draw_triangle(render, &v[0], &v[2], &v[3]);
  }
}

static void draw_triangle(
  const wg_render_t *render,
  const wg_vertex_t *v1,
  const wg_vertex_t *v2,
  const wg_vertex_t *v3
) {
  wg_trapezoid_t t1, t2;
  int n_trape = split_trapezoid(v1, v2, v3, &t1, &t2);
  draw_trapezoid(render, &t1);
  if (n_trape > 1) {
    draw_trapezoid(render, &t2);
  }
}

static void draw_trapezoid(
  const wg_render_t *render,
  const wg_trapezoid_t *t
) {
  wg_vertex_t l, r, step, start;
  wg_scanline_t scanline;
  float ystep = 1.0f / (t->bottom - t->top);
  float yratio = (t->v1.vPosH.y - t->top) / (t->bottom - t->top);
  for (int y = (int)ceilf(t->top); y < t->bottom; y ++, yratio += ystep) {
    if (y < 0) continue;
    if (y >= render->height) break;
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
  const wg_render_t *render,
  const wg_scanline_t *s
) {
  int x = s->x, y = s->y;
  wg_vertex_t v = s->v;
  wg_gbuff_t *geom;
  uint8_t *stencil;
  float *depth;
  for (int i = 0; i < s->w; i ++) {
    if (x < 0) {
      vertex_add(&v, &(s->step));
      x ++;
      continue;
    }
    if (x >= render->width) {
      break;
    }
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
      // Log("TC %f %f", geom->tc.x, geom->tc.y);
      geom->vColor = (wg_color_t){v.vColor.r * w, v.vColor.g * w, v.vColor.b * w};
    }
    vertex_add(&v, &(s->step));
    x ++;
  }
}