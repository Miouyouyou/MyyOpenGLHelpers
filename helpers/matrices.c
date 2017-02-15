#include <myy/helpers/log.h>
#include <myy/helpers/matrices.h>

void myy_matrix_4x4_print
(union myy_4x4_matrix * __restrict const matrix) {
	LOG(
	  "      X     |     Y     |     Z     |     W      \n"
	  "[%10.4f |%10.4f |%10.4f |%10.4f ] * X\n"
	  "[%10.4f |%10.4f |%10.4f |%10.4f ] * Y\n"
	  "[%10.4f |%10.4f |%10.4f |%10.4f ] * Z\n"
	  "[%10.4f |%10.4f |%10.4f |%10.4f ] * W\n"
	  "      +     |     +     |     +     |     +     \n"
	  "      ↓     |     ↓     |     ↓     |     ↓     \n"
	  "[     x     |     y     |     z     |     w     ]\n",
	  matrix->row.x.X, matrix->row.x.Y, matrix->row.x.Z, matrix->row.x.W,
	  matrix->row.y.X, matrix->row.y.Y, matrix->row.y.Z, matrix->row.y.W,
	  matrix->row.z.X, matrix->row.z.Y, matrix->row.z.Z, matrix->row.z.W,
	  matrix->row.w.X, matrix->row.w.Y, matrix->row.w.Z, matrix->row.w.W
	);
}

void myy_matrix_4x4_identity
(union myy_4x4_matrix * __restrict const matrix)
{
	union myy_4x4_matrix const identity_4x4 = {
		.vec_rows = {
		  {1,0,0,0},
		  {0,1,0,0},
		  {0,0,1,0},
		  {0,0,0,1}
		}
	};

	*matrix = identity_4x4;
}

void myy_matrix_4x4_translate
(union myy_4x4_matrix * __restrict const matrix,
 vec4 translation)
{
	matrix->vec_rows[m_w] += translation;
}

void myy_matrix_4x4_mult
(union myy_4x4_matrix * __restrict const matrix_stack,
 union myy_4x4_matrix const * __restrict const new_matrix)
{

	union myy_4x4_matrix result_matrix = {0};

	for (enum matrix_dimensions d = 0; d < n_4x4_matrix_dimensions; d++) {
		vec4 const matrix_row = matrix_stack->vec_rows[d];
		float const
		  x = matrix_row[0],
		  y = matrix_row[1],
		  z = matrix_row[2],
		  w = matrix_row[3];

		result_matrix.vec_rows[d] =
		  new_matrix->vec_rows[m_x] * x +
		  new_matrix->vec_rows[m_y] * y +
		  new_matrix->vec_rows[m_z] * z +
		  new_matrix->vec_rows[m_w] * w;
	}

	*matrix_stack = result_matrix;

}

