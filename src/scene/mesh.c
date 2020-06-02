#include "scene/mesh.h"

#include <stdlib.h>

wg_vertex_t *assemble_vertex(const wg_mesh_t *mesh) {
  uint32_t nv = mesh->nVertex;
  wg_vertex_t *v = (wg_vertex_t*)malloc(nv * sizeof(wg_vertex_t));
  for (int i = 0; i < nv; i ++) {
    v[i].vPos = mesh->vertex[i];
    v[i].normal = mesh->normal[i];
    v[i].tc = mesh->tc[i];
    v[i].vColor = mesh->vColor[i];
  }
  return v;
}

void destroy_mesh(wg_mesh_t *mesh) {
  free(mesh->vertex);
  free(mesh->normal);
  free(mesh->tc);
  free(mesh->vColor);
  free(mesh->triangle);
}

wg_mesh_t* mesh_plane(float w, float h) {
  uint32_t nv = 4, nt = 2;
  wg_mesh_t* mesh = (wg_mesh_t*)malloc(sizeof(wg_mesh_t));
  mesh->nVertex = nv;
  mesh->nTriangle = nt;
  mesh->vertex = (wg_point_t*)malloc(nv * sizeof(wg_point_t));
  mesh->normal = (wg_point_t*)malloc(nv * sizeof(wg_point_t));
  mesh->tc = (wg_txcoord_t*)malloc(nv * sizeof(wg_txcoord_t));
  mesh->vColor = (wg_color_t*)malloc(nv * sizeof(wg_color_t));
  mesh->triangle = (uint32_t*)malloc(nt * 3 * sizeof(uint32_t));

  w *= .5;
  h *= .5;

  int p = 0;
  for (int i = 0; i < 2; i ++) {
    for (int j = 0; j < 2; j ++) {
      mesh->vertex[p] = (wg_point_t){ {{(j*2-1) * w, (i*2-1) * h, 0., 1.}} };
      mesh->normal[p] = (wg_point_t){ {{0., 0., 1., 0.}} };
      mesh->tc[p] = (wg_txcoord_t){ (float)j, (float)i };
      mesh->vColor[p] = (wg_color_t){ .5 + .5 * i, 1. - .5 * j, .5 };
      p ++;
    }
  }

  uint32_t *tp = mesh->triangle;
  *tp ++ = 0; *tp ++ = 3; *tp ++ = 1;
  *tp ++ = 0; *tp ++ = 2; *tp ++ = 3;

  return mesh;
}
