#ifndef MYY_HELPERS_MATRICES_H
#define MYY_HELPERS_MATRICES_H 1

#include <myy/helpers/struct.h>

enum matrix_dimensions { m_x, m_y, m_z, m_w, n_4x4_matrix_dimensions };

typedef float vec4 __attribute__ ((vector_size (16)));

struct myy_matrix_4_row { float X, Y, Z, W; } __ALIGN_AT(16);

union myy_4x4_matrix {
	float raw_data[16];
	vec4 vec_rows[4];
	struct {
		struct myy_matrix_4_row x, y, z, w;
	} row;
} __ALIGN_AT(64);

void myy_matrix_4x4_print
(union myy_4x4_matrix const * __restrict const matrix);

void myy_matrix_4x4_identity
(union myy_4x4_matrix * __restrict const matrix);

void myy_matrix_4x4_translate
(union myy_4x4_matrix * __restrict const matrix,
 vec4 translation);

void myy_matrix_4x4_mult
(union myy_4x4_matrix * __restrict const matrix_stack,
 union myy_4x4_matrix const * __restrict const new_matrix);

void myy_matrix_4x4_ortho_layered_window_coords
(union myy_4x4_matrix * __restrict const matrix,
 unsigned int const width, unsigned int const height,
 unsigned int layers);

void myy_matrix_4x4_ortho_layered
(union myy_4x4_matrix * __restrict const matrix,
 unsigned int const width, unsigned int const height,
 unsigned int layers);

#endif
