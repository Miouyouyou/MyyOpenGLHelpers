#include <myy/helpers/opengl/quads_structures.h>

void US_two_tris_quad_3D_store
(US_two_tris_quad_3D * __restrict const quads,
 uint16_t const left, uint16_t const right,
 uint16_t const top,  uint16_t const bottom,
 uint16_t const layer,
 uint16_t const tex_left, uint16_t const tex_right,
 uint16_t const tex_top, uint16_t const tex_bottom)
{
	struct US_textured_point_3D * __restrict const points =
		quads->points;
	
	points[topleft_corner].x = left;
	points[topleft_corner].y = top;
	points[topleft_corner].z = layer;
	points[topleft_corner].s = tex_left;
	points[topleft_corner].t = tex_top;
	
	points[bottomleft_corner].x = left;
	points[bottomleft_corner].y = bottom;
	points[bottomleft_corner].z = layer;
	points[bottomleft_corner].s = tex_left;
	points[bottomleft_corner].t = tex_bottom;

	points[topright_corner].x = right;
	points[topright_corner].y = top;
	points[topright_corner].z = layer;
	points[topright_corner].s = tex_right;
	points[topright_corner].t = tex_top;

	points[bottomright_corner].x = right;
	points[bottomright_corner].y = bottom;
	points[bottomright_corner].z = layer;
	points[bottomright_corner].s = tex_right;
	points[bottomright_corner].t = tex_bottom;
	
	points[repeated_topright_corner].x = right;
	points[repeated_topright_corner].y = top;
	points[repeated_topright_corner].z = layer;
	points[repeated_topright_corner].s = tex_right;
	points[repeated_topright_corner].t = tex_top;
	
	points[repeated_bottomleft_corner].x = left;
	points[repeated_bottomleft_corner].y = bottom;
	points[repeated_bottomleft_corner].z = layer;
	points[repeated_bottomleft_corner].s = tex_left;
	points[repeated_bottomleft_corner].t = tex_bottom;
}
 
void US_two_tris_quad_3D_draw_pixelscoords
(GLuint const buffer_id,
 GLuint const xyz_attribute, GLuint const st_attribute,
 GLuint const offset,
 unsigned int const n_quads)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
	glVertexAttribPointer(
		st_attribute, 2, GL_UNSIGNED_SHORT, GL_TRUE,
		sizeof(struct US_textured_point_3D),
		(uint8_t *) (offset+offsetof(struct US_textured_point_3D, s))
	);
	glVertexAttribPointer(
		xyz_attribute, 3, GL_SHORT, GL_FALSE,
		sizeof(struct US_textured_point_3D),
		(uint8_t *) (offset+offsetof(struct US_textured_point_3D, x))
	);
	glDrawArrays(GL_TRIANGLES, 0, n_quads*n_corners_two_tris_quad);
}
