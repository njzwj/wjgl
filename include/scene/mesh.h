#ifndef __MESH_H__
#define __MESH_H__

#include "render.h"

typedef struct {
  size_t nVertex;
  size_t nTriangle;

  wg_point_t *vertex;
  wg_point_t *normal;
  wg_txcoord_t *tc;
  wg_color_t *vColor;
  
  uint32_t *triangle;
} wg_mesh_t;

wg_vertex_t *assemble_vertex(const wg_mesh_t *mesh);

void destroy_mesh(wg_mesh_t *mesh);

wg_mesh_t *mesh_plane(float w, float h);

#endif