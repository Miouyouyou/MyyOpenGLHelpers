#ifndef MYY_SRC_HELPERS_OPENGL_QUADS_STRUCTURES
#define MYY_SRC_HELPERS_OPENGL_QUADS_STRUCTURES 1

#include <myy/current/opengl.h>
#include <myy/helpers/hitbox_action.h>
#include <myy/helpers/struct.h>
#include <stdint.h>

enum quad_coords_order {
	topleft_corner, bottomleft_corner, topright_corner, bottomright_corner,
	repeated_topright_corner, repeated_bottomleft_corner,
	n_corners_two_tris_quad
};

struct generated_quads {
	uint16_t count, size;
};
typedef struct generated_quads generated_quads_uS;
static inline generated_quads_uS generated_quads_uS_struct
(uint16_t count, uint16_t size)
{
	generated_quads_uS quads = { .count = count, .size  = size };
	return quads;
}

static inline void generated_quads_uS_add
(generated_quads_uS * __restrict const total,
 generated_quads_uS const gen_quads)
{
	total->count += gen_quads.count;
	total->size  += gen_quads.size;
}

struct myy_rgba { uint8_t r, g, b, a; } __PALIGN__;
typedef struct myy_rgba rgba_t;

struct point_2D { GLfloat x, y; } __PALIGN__;
struct point_3D { GLfloat x, y, z; } __PALIGN__;
struct triangle_2D { struct point_2D a, b, c; } __PALIGN__;
struct layered_triangle_2D { struct point_3D a, b, c; } __PALIGN__;
struct two_triangles_quad_2D {
	struct triangle_2D first, second;
} __PALIGN__;
struct two_layered_triangles_quad_2D {
	struct layered_triangle_2D first, second;
} __PALIGN__;

struct textured_point_2D { GLfloat x, y, s, t; } __PALIGN__;
struct textured_point_3D { GLfloat x, y, z, s, t; } __PALIGN__;
struct textured_triangle_2D {
	struct textured_point_2D a, b, c;
} __PALIGN__;
struct textured_layered_triangle_2D {
	struct textured_point_3D a, b, c;
} __PALIGN__;
struct two_triangles_textured_quad_2D {
	struct textured_triangle_2D first, second;
} __PALIGN__;
struct two_textured_layered_triangles_quad {
	struct textured_layered_triangle_2D first, second;
} __PALIGN__;

union two_triangles_textured_quad_2D_representations {
	struct two_triangles_textured_quad_2D quad;
	struct textured_triangle_2D triangles[2];
	struct textured_point_2D points[6];
	GLfloat raw_coords[24];
} __PALIGN__;

union two_textured_layered_triangles_quad_representations {
	struct two_textured_layered_triangles_quad quad;
	struct textured_layered_triangle_2D triangles[2];
	struct textured_point_3D points[6];
	GLfloat raw_coords[30];
} __PALIGN__;

typedef union two_triangles_textured_quad_2D_representations two_tris_quad;
typedef union two_textured_layered_triangles_quad_representations two_layered_tris_quad;

#define TWO_TRIANGLES_TEX_QUAD(left, right, down, up, left_tex, right_tex, down_tex, up_tex) { \
	.points = { \
    { .x = left,  .y = up,   .s = left_tex,  .t = up_tex},   \
    { .x = left,  .y = down, .s = left_tex,  .t = down_tex}, \
    { .x = right, .y = up,   .s = right_tex, .t = up_tex},   \
    { .x = right, .y = down, .s = right_tex, .t = down_tex}, \
    { .x = right, .y = up,   .s = right_tex, .t = up_tex},   \
    { .x = left,  .y = down, .s = left_tex,  .t = down_tex}  \
	} \
}

struct byte_point_2D { GLbyte x, y; } __ALIGN_AT(2);
struct BUS_textured_point_2D { GLushort s, t; GLbyte x, y; } __PALIGN__;
struct BUS_textured_triangle_2D { struct BUS_textured_point_2D a, b, c; } __PALIGN__;
struct BUS_two_triangles_textured_quad_2D { struct BUS_textured_triangle_2D first, second; } __PALIGN__;

union BUS_two_triangles_textured_quad_2D_representations {
	struct BUS_two_triangles_textured_quad_2D quad;
	struct BUS_textured_triangle_2D triangles[2];
	struct BUS_textured_point_2D points[6];
} __PALIGN__;

typedef union BUS_two_triangles_textured_quad_2D_representations BUS_two_tris_quad;

#define TWO_BYTES_TRIANGLES_TEX_QUAD(left, right, down, up, left_tex, right_tex, down_tex, up_tex) { \
	.points = { \
    { .s = left_tex,  .t = up_tex,   .x = left,  .y = up,  },  \
    { .s = left_tex,  .t = down_tex, .x = left,  .y = down },  \
    { .s = right_tex, .t = up_tex,   .x = right, .y = up,  },  \
    { .s = right_tex, .t = down_tex, .x = right, .y = down },  \
    { .s = right_tex, .t = up_tex,   .x = right, .y = up,  },  \
    { .s = left_tex,  .t = down_tex, .x = left,  .y = down },  \
	} \
}

#define STXYZ_QUAD(left, right, down, up, layer, left_tex, right_tex, down_tex, up_tex) { \
	.points = { \
    { .s = left_tex,  .t = up_tex,   .x = left,  .y = up,   .z = layer },  \
    { .s = left_tex,  .t = down_tex, .x = left,  .y = down, .z = layer },  \
    { .s = right_tex, .t = up_tex,   .x = right, .y = up,   .z = layer },  \
    { .s = right_tex, .t = down_tex, .x = right, .y = down, .z = layer },  \
    { .s = right_tex, .t = up_tex,   .x = right, .y = up,   .z = layer },  \
    { .s = left_tex,  .t = down_tex, .x = left,  .y = down, .z = layer },  \
	} \
}

struct byte_point_3D { GLbyte x, y, z; } __PALIGN__;
struct BUS_textured_point_3D { GLushort s, t; GLbyte x, y, z; } __PALIGN__;
struct BUS_textured_triangle_3D { struct BUS_textured_point_3D a, b, c; } __PALIGN__;
struct BUS_two_triangles_textured_quad_3D { struct BUS_textured_triangle_3D first, second; } __PALIGN__;

union BUS_two_triangles_textured_quad_3D_representations {
	struct BUS_two_triangles_textured_quad_3D quad;
	struct BUS_textured_triangle_3D triangles[2];
	struct BUS_textured_point_3D points[6];
} __PALIGN__;

typedef union BUS_two_triangles_textured_quad_3D_representations BUS_two_tris_3D_quad;

struct US_textured_point_2D { GLushort s, t; GLshort x, y; } __PALIGN__;
struct US_textured_triangle_2D { struct US_textured_point_2D a, b, c; } __PALIGN__;
struct US_two_triangles_textured_quad_2D { struct US_textured_triangle_2D first, second; } __PALIGN__;

union US_two_triangles_textured_quad_2D_representations {
	struct US_two_triangles_textured_quad_2D quad;
	struct US_textured_triangle_2D triangles[2];
	struct US_textured_point_2D points[6];
} __PALIGN__;

struct SuB_colored_point_2D { GLshort x, y; GLubyte r, g, b, a; }
__PALIGN__;
struct SuB_colored_triangle_2D {
	struct SuB_colored_point_2D a, b, c;
} __PALIGN__;
struct SuB_two_triangles_colored_quad_2D {
	struct SuB_colored_triangle_2D first, second;
};

union SuB_two_triangles_colored_quad_2D_representations {
	struct SuB_two_triangles_colored_quad_2D quad;
	struct SuB_colored_triangle_2D triangles[2];
	struct SuB_colored_point_2D points[6];
} __PALIGN__;

struct SuB_colored_point_3D { GLshort x, y, z; GLubyte r, g, b, a; }
__PALIGN__;
struct SuB_colored_triangle_3D {
	struct SuB_colored_point_3D a, b, c;
} __PALIGN__;
struct SuB_two_triangles_colored_quad_3D {
	struct SuB_colored_triangle_3D first, second;
} __PALIGN__;

union SuB_two_triangles_colored_quad_3D_representations {
	struct SuB_two_triangles_colored_quad_3D quad;
	struct SuB_colored_triangle_3D triangles[2];
	struct SuB_colored_point_3D points[6];
} __PALIGN__;

struct US_textured_point_3D { GLshort x, y, z; GLushort s, t; } __PALIGN__;
struct US_textured_triangle_3D { struct US_textured_point_3D a, b, c; } __PALIGN__;
struct US_two_triangles_textured_quad_3D { struct US_textured_triangle_3D first, second; } __PALIGN__;

union US_two_triangles_textured_quad_3D_representations {
	struct US_two_triangles_textured_quad_3D quad;
	struct US_textured_triangle_3D triangles[2];
	struct US_textured_point_3D points[6];
} __PALIGN__;

struct SuSuB_textured_colored_point_3D {
	GLshort x, y, z;
	GLushort s, t;
	GLubyte r, g, b, a;
} __PALIGN__;

struct SuSuB_textured_colored_triangle_3D { 
	struct SuSuB_textured_colored_point_3D a, b, c;
} __PALIGN__;
struct SuSuB_2t_textured_colored_quad_3D {
	struct SuSuB_textured_colored_triangle_3D first, second;
} __PALIGN__;

union SuSuB_2t_textured_colored_quad_3D_representations {
	struct SuSuB_2t_textured_colored_quad_3D quad;
	struct SuSuB_textured_colored_triangle_3D triangles[2];
	struct SuSuB_textured_colored_point_3D points[6];
} __PALIGN__;

typedef union US_two_triangles_textured_quad_2D_representations
US_two_tris_quad;
typedef union US_two_triangles_textured_quad_3D_representations
US_two_tris_quad_3D;
typedef union SuB_two_triangles_colored_quad_2D_representations
SuB_2t_colored_quad;
typedef union SuB_two_triangles_colored_quad_3D_representations
SuB_2t_colored_quad_3D;
typedef union SuSuB_2t_textured_colored_quad_3D_representations
SuSuB_2t_colored_quad_3D;

void US_two_tris_quad_3D_store
(US_two_tris_quad_3D * __restrict const quads,
 uint16_t const left, uint16_t const right,
 uint16_t const top,  uint16_t const bottom,
 uint16_t const layer,
 uint16_t const tex_left, uint16_t const tex_right,
 uint16_t const tex_top, uint16_t const tex_bottom);

void US_two_tris_quad_3D_draw_pixelscoords
(GLuint const buffer_id,
 GLuint const xyz_attribute, GLuint const st_attribute,
 GLuint const offset,
 unsigned int const n_quads);

void SuB_2t_colored_quad_store
(int_least16_t const left, int_least16_t const top,
 int_least16_t const right, int_least16_t const bottom,
 SuB_2t_colored_quad * __restrict const quad,
 uint_least8_t const r, uint_least8_t const g, uint_least8_t const b,
 uint_least8_t const a);

void SuB_2t_colored_quad_draw_pixel_coords
(GLuint const buffer_id,
 GLuint const xy_attribute, GLuint const rgba_attribute,
 GLuint const offset,
 unsigned int const n_quads);

void SuB_2t_colored_quad_store_box
(box_coords_S_t box,
 uint8_t const r, uint8_t const g,
 uint8_t const b, uint8_t const a,
 SuB_2t_colored_quad * __restrict const quad);

void SuB_2t_colored_quad_store_box_3D
(box_coords_S_t box,
 uint8_t const r, uint8_t const g,
 uint8_t const b, uint8_t const a,
 uint16_t layer,
 SuB_2t_colored_quad_3D * __restrict const quad);

void SuB_2t_colored_quad_store_box_rgba_3D
(box_coords_S_t const box, rgba_t const color, uint16_t const layer,
 SuB_2t_colored_quad_3D * __restrict const quad);

void SuB_2t_colored_quad_3D_draw_pixel_coords
(GLuint const buffer_id,
 GLuint const xy_attribute, GLuint const rgba_attribute,
 GLuint const offset,
 unsigned int const n_quads);

#endif
