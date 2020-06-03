#include "wjgl.h"
#include "svpng.h"
#include <stdio.h>
#include <stdlib.h>

void render_mesh(wg_render_t *render, wg_mesh_t *mesh) {
  wg_vertex_t *v = assemble_vertex(mesh);
  uint32_t nv = mesh->nVertex, nt = mesh->nTriangle;
  uint32_t *tri = mesh->triangle;
  project_vertexes(render, v, nv);
  for (int i = 0; i < nt * 3; i += 3) {
    cull_and_draw_triangle(render, v+tri[i], v+tri[i+1], v+tri[i+2]);
  }
  free(v);
}

void test_render() {
  // Setup scene
  const int W = 256, H = 256;
  wg_render_t *render = get_render();
  wg_mat44f t_world, t_camera, t_projection;
  wg_point_t eye, center, up;
  wg_mesh_t *plane_mesh = mesh_plane(20., 20.);
  wg_texture_t *tex_chessboard = get_empty_texture(32, 32);
  set_chessboard_texture(tex_chessboard, 8, 8, 0xffffff, 0xff0000);
  render->texture = tex_chessboard;
  uint8_t rgb[W * H * 3], *p = rgb;

  eye = (wg_point_t){ {{5., 0., 20., 1.}} };
  center = (wg_point_t){ {{0., 0., 0., 1.}} };
  up = (wg_point_t){ {{0., 1., 0., 1.}} };

  render->renderMode = SHADED;

  set_up_render(render, W, H);
  get_identical_mat(&t_world);
  get_lookat_mat(&t_camera, eye, center, up);
  get_projection_mat(&t_projection, 60., 1., 15., 100.);
  render->transform.world = &t_world;
  render->transform.camera = &t_camera;
  render->transform.projection = &t_projection;
  transform_update(&render->transform);
  clear_render(render);

  render_mesh(render, plane_mesh);

  shade_fragment(render);
  shade_on_buffer(render);

  for (int i = 0; i < H; i += 16) {
    for (int j = 0; j < W; j += 8) {
      printf("%c", " #"[render->stencil[j + i * render->width]]);
    }
    printf("\n");
  }

  FILE *fp = fopen("demo_texture.png", "wb");
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
  test_render();
  return 0;
}
