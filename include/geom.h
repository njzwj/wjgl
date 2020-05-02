#ifndef __GEOM_H__
#define __GEOM_H__

typedef struct {
  union {
    struct { float x, y, z, w; };
    float v[4];
  };
} wg_vec4f;
typedef wg_vec4f wg_point_t;

typedef struct {
  union {
    struct {
      float _11, _12, _13, _14,
            _21, _22, _23, _24,
            _31, _32, _33, _34,
            _41, _42, _43, _44;
    };
    float m[4][4];
    float v[16];
  };
} wg_mat44f;

/* Some operations involving Mat44f and Vec4f */
wg_vec4f      v4f_add(wg_vec4f a, wg_vec4f b);
wg_vec4f      v4f_sub(wg_vec4f a, wg_vec4f b);
wg_vec4f      v4f_mul(wg_vec4f a, float b);
wg_vec4f      v4f_div(wg_vec4f a, float b);
float         v4f_dot_prod(wg_vec4f a, wg_vec4f b);
wg_vec4f      v4f_cross_prod(wg_vec4f a, wg_vec4f b);
float         lerp(float a, float b, float x);
wg_vec4f      lerp_vec4f(wg_vec4f a, wg_vec4f b, float x);
void          normalize_vec4f(wg_vec4f *a);

void          matmul(wg_mat44f *const a, wg_mat44f *const b, wg_mat44f *y);

void          matvecmul4(wg_mat44f *const m, wg_vec4f *const b, wg_vec4f *y);

typedef struct {
  float x, y;
} wg_vec2f;
typedef wg_vec2f wg_txcoord_t;

typedef struct {
  float r, g, b;
} wg_color_t;

typedef struct {
  wg_mat44f *world;
  wg_mat44f *camera;
  wg_mat44f *projection;
  wg_mat44f *transform;
  wg_mat44f *transform_p;
  float w, h;
} wg_transform_t;

void get_projection_mat(wg_mat44f *m, float fovH, float aspectRatio, float near, float far);

void get_translation_mat(wg_mat44f *m, float dx, float dy, float dz);

void get_lookat_mat(wg_mat44f *m, wg_point_t eye, wg_point_t center, wg_point_t up);

void get_identical_mat(wg_mat44f *m);

void transform_update(wg_transform_t *t);

typedef struct {
  wg_point_t vPosH;         // Homogenous position in form of (x, y, z, w)

  // The following attributes need to be interpolated
  wg_point_t vPos;          // Screen space position
  wg_point_t normal;        // Normal
  wg_txcoord_t tc;          // Texture coordinates
  wg_color_t vColor;        // Vertex color 
  float rhw;                // 1/z
} wg_vertex_t;

typedef struct {
  wg_vec4f vPosH;           // Homogenous position in form of (x, y, z, w)
  
  /* The following attributes need to be interpolated */
  wg_point_t vPos;          // Screen space position
  wg_point_t normal;        // Normal
  wg_txcoord_t tc;          // Texture coordinates
  wg_color_t vColor;        // Vertex color

  /* The following attributes need to be shaded by fragment shader */
  wg_color_t diffuseColor;  // Diffuse color
  wg_color_t specularColorAdder;      // Light color adder
  wg_color_t color;         // Final color
} wg_gbuff_t;

// Apply transform x -> y
void transform_apply(wg_transform_t *const t, wg_point_t *y, wg_point_t *const x);

// Transform x into homogenous position
// The procedure follows OpenGL: (x, y, z, w) -> (x / w, y / w, z / w, 1 / w)
//     in which w = (z in camera coordinate)
// Then transform x & y to screen size
void transform_homogenous(wg_transform_t *const t, wg_vertex_t *x);

// Transform everything into Attr / z
void vertex_init_rhw(wg_vertex_t *x);

// y += x
void vertex_add(wg_vertex_t *y, wg_vertex_t *const x);

// y -= x
void vertex_sub(wg_vertex_t *y, wg_vertex_t *const x);

void vertex_scale(wg_vertex_t *y, float x);

void vertex_step(wg_vertex_t *step, wg_vertex_t *const l, wg_vertex_t *const r);

void vertex_interp(wg_vertex_t *v, wg_vertex_t *const v1, wg_vertex_t *const v2, float x);

#endif