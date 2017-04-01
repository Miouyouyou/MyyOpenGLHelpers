#include <myy/helpers/log.h>
#include <myy/helpers/matrices.h>

#include <stdint.h>

void myy_matrix_4x4_print
(union myy_4x4_matrix const * __restrict const matrix) {
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

void myy_matrix_4x4_ortho_layered_window_coords
(union myy_4x4_matrix * __restrict const matrix,
 unsigned int const width, unsigned int const height,
 unsigned int const layers)
{

	union myy_4x4_matrix const ortho_matrix = {
		.vec_rows = {
			{2.0/width, 0, 0, 0},
			{0, -2.0/height, 0, 0},
			{0, 0, 1.0/layers, 0},
			{-1, 1, 0, 1}
		}
	};
	myy_matrix_4x4_print(&ortho_matrix);
	*matrix = ortho_matrix;
}

void myy_matrix_4x4_ortho_layered
(union myy_4x4_matrix * __restrict const matrix,
 unsigned int const width, unsigned int const height,
 unsigned int const layers)
{

	union myy_4x4_matrix const ortho_matrix = {
		.vec_rows = {
			{2.0/width, 0, 0, 0},
			{0, 2.0/height, 0, 0},
			{0, 0, 1.0/layers, 0},
			{-1, -1, 0, 1}
		}
	};
	myy_matrix_4x4_print(&ortho_matrix);
	*matrix = ortho_matrix;
}

vec4 myy_vec4_4x4_matrix_mult
(vec4 const * __restrict const vector,
 union myy_4x4_matrix const * __restrict const matrix) {
	float
		x = vector[0][vec_x],
		y = vector[0][vec_y],
		z = vector[0][vec_z],
		w = vector[0][vec_w];
		
	return matrix->vec_rows[m_x] * x +
		matrix->vec_rows[m_y] * y +
		matrix->vec_rows[m_z] * z +
		matrix->vec_rows[m_w] * w;
}

vec4 myy_3i16_vector_4x4_matrix_mult
(int16_t const * __restrict const vector,
 union myy_4x4_matrix const * __restrict const matrix,
 int16_t w_value)
{
	float
		x = vector[vec_x],
		y = vector[vec_y],
		z = vector[vec_z],
		w = w_value;
	
	return matrix->vec_rows[m_x] * x +
		matrix->vec_rows[m_y] * y +
		matrix->vec_rows[m_z] * z +
		matrix->vec_rows[m_w] * w;
}

void myy_vec4_print(vec4 const vector)
{
	LOG("%f, %f, %f, %f\n", 
	    vector[vec_x], vector[vec_y], vector[vec_z], vector[vec_w]);
}
