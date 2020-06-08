#include "wjgl.h"
#include "svpng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

void blinn_phong_shader(wg_render_t *render, wg_gbuff_t *gbuff) {  
  float shininess = 10.0;
  wg_point_t *lpos = &render->light.position;
  wg_point_t *vpos = &gbuff->vPos;
  wg_point_t vl = v4f_sub(render->light.position, gbuff->vPos);
  normalize_vec4f(&vl);                                                 // light direction
  wg_point_t ve = v4f_sub((wg_point_t){0., 0., 0., 1.}, gbuff->vPos);   
  normalize_vec4f(&ve);                                                 // eye direction
  wg_point_t vn = v4f_add(vl, ve);
  normalize_vec4f(&vn);                                                 // half normal
  float shine = v4f_dot_prod(vn, gbuff->normal);
  shine = powf(shine, shininess);
  shine = shine < 0. ? 0. : shine;
  
  color_mul_add(&gbuff->specularColorAdder, render->light.color, shine);
  color_mul_add(&gbuff->color, gbuff->specularColorAdder, render->material.specular);
  color_mul_add(&gbuff->color, gbuff->diffuseColor, render->material.diffuse);
}

void test_render() {
  init_frag_shader_reg();
  register_frag_shader("BlinnPhongShader", &blinn_phong_shader);

  // Setup scene
  const int W = 256, H = 256;
  wg_render_t *render = get_render();
  wg_mat44f t_world, t_camera, t_projection;
  wg_point_t eye, center, up;
  wg_mesh_t *plane_mesh = mesh_plane(20., 20.);
  wg_texture_t *tex_chessboard = get_empty_texture(128, 128);
  set_chessboard_texture(tex_chessboard, 8, 8, 0xffffff, 0xff0000);
  render->texture = tex_chessboard;
  uint8_t rgb[W * H * 3], *p = rgb;

  eye = (wg_point_t){ {{ -10., -5., 20., 1.}} };
  center = (wg_point_t){ {{0., 0., 0., 1.}} };
  up = (wg_point_t){ {{0., 1., 0., 1.}} };

  render->renderMode = SHADED;
  render->sampleMode = BILINEAR;
  // render->sampleMode = NEAREST;

  render->fshaderName = "BlinnPhongShader";

  render->material = (wg_material_t){0.0, 0.4, 1.0};
  wg_light_t light = (wg_light_t){(wg_point_t){5., 5., 5., 1.}, (wg_color_t){.9, .2, .5}};

  set_up_render(render, W, H);
  get_identical_mat(&t_world);
  get_lookat_mat(&t_camera, eye, center, up);
  get_projection_mat(&t_projection, 60., 1., 15., 100.);
  render->transform.world = &t_world;
  render->transform.camera = &t_camera;
  render->transform.projection = &t_projection;
  transform_update(&render->transform);
  clear_render(render);
  set_light(render, light);

  render_mesh(render, plane_mesh);

  shade_fragment(render);
  shade_on_buffer(render);

  for (int i = 0; i < H; i += 16) {
    for (int j = 0; j < W; j += 8) {
      printf("%c", " #"[render->stencil[j + i * render->width]]);
    }
    printf("\n");
  }

  FILE *fp = fopen("demo_light.png", "wb");
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
