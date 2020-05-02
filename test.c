#include "wjgl.h"
#include "svpng.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

void test_mat44f() {
  wg_mat44f mat;
  for (int i = 0; i < 16; i ++) {
    mat.v[i] = i;
  }
  assert(mat._11 == mat.v[0]);
  assert(mat._23 == mat.v[6]);
  assert(mat._44 == mat.v[15]);
}

void debug_mat(wg_mat44f *m, const char *name) {
  printf("%s\n", name);
  for (int i = 0; i < 4; i ++) {
    for (int j = 0; j < 4; j ++) {
      printf("%.2f ", m->m[i][j]);
    }
    printf("\n");
  }
}

void render_mesh(wg_render_t *render, wg_mesh_t *mesh) {
  wg_vertex_t *v = assemble_vertex(mesh);
  uint32_t nv = mesh->nVertex, nt = mesh->nTriangle;
  uint32_t *tri = mesh->triangle;
  shade_vertex(render, v, nv, &default_vs);
  // Clipping in homogenous space
  for (int i = 0; i < nt * 3; i += 3) {
    draw_triangle(render, v+tri[i], v+tri[i+1], v+tri[i+2]);
  }
  free(v);
}

void test_render() {
  wg_render_t *render = get_render();
  wg_mat44f t_world, t_camera, t_projection;
  wg_point_t eye, center, up;
  const int W = 512, H = 512;
  wg_mesh_t *plane_mesh = mesh_plane(20., 20.);

  eye = (wg_point_t){ {{-10., 0., 25., 1.}} };
  center = (wg_point_t){ {{0., 0., 0., 1.}} };
  up = (wg_point_t){ {{0., 1., 0., 1.}} };

  render->renderMode = VERTEX_COLOR;

  set_up_render(render, W, H);
  get_identical_mat(&t_world);
  get_lookat_mat(&t_camera, eye, center, up);
  debug_mat(&t_camera, "camera");
  get_projection_mat(&t_projection, 60., 1., 1., 100.);
  render->transform.world = &t_world;
  render->transform.camera = &t_camera;
  render->transform.projection = &t_projection;
  transform_update(&render->transform);
  clear_render(render);
  uint8_t rgb[W * H * 3], *p = rgb;

  render_mesh(render, plane_mesh);

  shade_fragment(render);
  shade_on_buffer(render);

  for (int i = 0; i < H; i += 16) {
    for (int j = 0; j < W; j += 8) {
      printf("%c", " #"[render->stencil[j + i * render->width]]);
    }
    printf("\n");
  }

  FILE *fp = fopen("test.png", "wb");
  for (int y = 0; y < H; y ++) {
    for (int x = 0; x < W; x ++) {
      uint8_t *buf = render->frameBuffer + (y * W + x) * 4;
      *p++ = *buf++;
      *p++ = *buf++;
      *p++ = *buf++;
    }
  }
  svpng(fp, render->width, render->height, rgb, 0);
  fclose(fp);

  destroy_mesh(plane_mesh);
  free(plane_mesh);
}

int main() {
  test_mat44f();
  
  test_render();

  printf("All tests are done.\n");
  return 0;
}
