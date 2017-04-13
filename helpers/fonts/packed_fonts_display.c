#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

static unsigned int find_codepoint_in
(struct myy_packed_fonts_codepoints const * __restrict const codepoints,
 uint32_t searched_codepoint) {
	unsigned int i = 1;
	while (codepoints[i].codepoint &&
	       (codepoints[i].codepoint != searched_codepoint))
		i++;
	unsigned int found_index =
	  i * (codepoints[i].codepoint == searched_codepoint);
	return found_index;
}

/* This is outdated : All the quads must be generated in advance and
 * stored somewhere.
 *
 * Storing in :
 * CPU Memory : More data to pass to the GPU every time
 * GPU Memory : Too much data when passing everything at once
 *
 * However, there's still two things to take in mind :
 * - Either the text is predetermined, without effects (like
 *   typewriter effect, with characters appearing one by one) and it
 *   might be faster to render the shown texts in an external texture.
 * - Or the text is dynamic and it might be better to generate only
 *   the characters needed on the fly, store them in the GPU and use
 *   them when needed.
 *
 * At the moment, a mix of these solutions will be used. Only a subset
 * of characters will be pre-generated and rendered dynamically.
 *
 * Still, for a HUD, static rendering might make things way easier,
 * if no alpha channel is involved.
 *
 */
int16_t myy_glyph_to_twotris_quad
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 US_two_tris_quad_3D * __restrict const quad,
 int16_t x_offset_px) {

	struct myy_packed_fonts_codepoints const * __restrict const codepoints =
	  glyph_infos->codepoints_addr;

	unsigned int codepoint_index =
	  find_codepoint_in(codepoints, codepoint);
	if (codepoint_index) {
		struct myy_packed_fonts_glyphdata const * __restrict const glyphdata =
		  glyph_infos->glyphdata_addr+codepoint_index;

		int16_t glyph_x_offset_px = glyphdata->offset_x_px + x_offset_px;
		int16_t glyph_y_offset_px = glyphdata->offset_y_px;
		int16_t right_px = glyphdata->width_px  + glyph_x_offset_px;
		int16_t up_px    = glyphdata->height_px + glyph_y_offset_px;
		int16_t advance_x = x_offset_px + glyphdata->advance_x_px;

		uint16_t const
		  left  = glyph_x_offset_px,
		  right = right_px,
		  down  = glyph_y_offset_px,
		  up    = up_px,
		  layer = 32,
		  tex_left  = glyphdata->tex_left,
		  tex_right = glyphdata->tex_right,
		  tex_up    = glyphdata->tex_top,
		  tex_down  = glyphdata->tex_bottom;

		US_two_tris_quad_3D_store(
			quad, left, right, up, down, layer,
			tex_left, tex_right, tex_up, tex_down
		);
		return advance_x;
	}
	return x_offset_px;
}

/* This *heavily* depends on the current coordinate system in use.
	* While using the OpenGL coordinate system seems natural with
	* OpenGL, it feels extremely weird with the Windowing coordinate
	* system.
	* 
	* The problem :
	* - In OpenGL, Y goes UP meaning that higher values point at the 
	*   top of the screen and lower values point at the bottom.
	* - In Windowing coordinates systems, Y goes DOWN, meaning that
	*   higher values point at the bottom and lower values point at
	*   the top.
	*
	* The OpenGL really makes sense until you think about how you'll
	* organise text in a window. When you're use to TOP-BOTTOM writing
	* systems (like European and some Asiatic languages), when you'll
	* create a window, you'll "start writing at y pixels from the TOP".
	* You could said that you'll start writing at -y pixels from the
	* TOP but that does not appear natural to me.
	* 
	* Still... I'm really hesitating on that one... */
void myy_glyph_to_twotris_quad_window_coords
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 US_two_tris_quad_3D * __restrict const quad,
 struct text_offset * __restrict const offset) {

	struct myy_packed_fonts_codepoints const * __restrict const codepoints =
	  glyph_infos->codepoints_addr;

	unsigned int codepoint_index =
	  find_codepoint_in(codepoints, codepoint);
	int16_t x_offset_px = offset->x_offset;
	int16_t y_offset_px = offset->y_offset;
	if (codepoint_index) {
		struct myy_packed_fonts_glyphdata const * __restrict const glyphdata =
		  glyph_infos->glyphdata_addr+codepoint_index;
		
		int16_t baseline_px = 16+y_offset_px; // px
		int16_t glyph_x_offset_px = glyphdata->offset_x_px + x_offset_px;
		int16_t glyph_y_offset_px = baseline_px - glyphdata->offset_y_px;
		int16_t right_px = glyphdata->width_px  + glyph_x_offset_px;
		int16_t up_px    = glyph_y_offset_px - glyphdata->height_px;
		int16_t advance_x = x_offset_px + glyphdata->advance_x_px;

		uint16_t const
		  left  = glyph_x_offset_px,
		  right = right_px,
		  down  = glyph_y_offset_px,
		  up    = up_px,
		  layer = 32,
		  tex_left  = glyphdata->tex_left,
		  tex_right = glyphdata->tex_right,
		  tex_up    = glyphdata->tex_top,
		  tex_down  = glyphdata->tex_bottom;

		US_two_tris_quad_3D_store(
			quad, left, right, up, down, layer,
			tex_left, tex_right, tex_up, tex_down
		);
		
		x_offset_px = advance_x;
	}
	
	offset->x_offset = x_offset_px;
	offset->y_offset = y_offset_px;
	
	return;
}

int16_t myy_glyph_to_qinfos
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 struct qinfos * __restrict const quad,
 int16_t x_offset_px)
{
	struct myy_packed_fonts_codepoints const * __restrict const codepoints =
	  glyph_infos->codepoints_addr;

	unsigned int codepoint_index =
	  find_codepoint_in(codepoints, codepoint);
	if (codepoint_index) {
		struct myy_packed_fonts_glyphdata const * __restrict const glyphdata =
		  glyph_infos->glyphdata_addr+codepoint_index;

		quad->quad_w       = glyphdata->width_px;
		quad->quad_h       = glyphdata->height_px;
		quad->q_off_x      = glyphdata->offset_x_px + x_offset_px;
		uint16_t advance_x = x_offset_px + glyphdata->advance_x_px;
		quad->q_off_y      = glyphdata->offset_y_px;
		quad->tex_off_x    = glyphdata->tex_left;
		quad->tex_w        = glyphdata->tex_right - glyphdata->tex_left;
		quad->tex_off_y    = glyphdata->tex_bottom;
		quad->tex_h        = glyphdata->tex_top - glyphdata->tex_bottom;

		return advance_x;
	}
	return x_offset_px;
}

void myy_codepoints_to_glyph_qinfos
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 struct qinfos * __restrict const quads)
{
	int16_t x_offset = 0;
	for (unsigned int i = 0; i < n_characters; i++)
		x_offset = myy_glyph_to_qinfos(
		  glyph_infos, string[i], quads+i, x_offset
		);
}

void myy_codepoints_to_glyph_twotris_quads
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 US_two_tris_quad_3D * __restrict const quads) {

	int16_t x_offset = 0;
	for (unsigned int i = 0; i < n_characters; i++)
		x_offset = myy_glyph_to_twotris_quad(
		  glyph_infos, string[i], quads+i, x_offset
		);

}

void myy_codepoints_to_glyph_twotris_quads_window_coords
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 US_two_tris_quad_3D * __restrict const quads) {

	struct text_offset offsets = {
		.x_offset = 0,
		.y_offset = 0
	};
	for (unsigned int i = 0; i < n_characters; i++)
		myy_glyph_to_twotris_quad_window_coords(
		  glyph_infos, string[i], quads+i, &offsets
		);

}

unsigned int myy_strings_to_quads_va
(struct glyph_infos const * __restrict const glyph_infos,
 unsigned int const n_strings,
 uint8_t const * const * __restrict const strings,
 uint8_t * __restrict const buffer,
 int16_t const vertical_offset_px)
{
	US_two_tris_quad_3D * __restrict const quads_buffer =
		(US_two_tris_quad_3D *) buffer;

	unsigned int stored_quads = 0;
	struct text_offset offset = {.x_offset = 0, .y_offset = 0};

	for (unsigned int s = 0; s < n_strings; s++) {
		offset.x_offset = 0; // line start
		uint8_t const * string = strings[s];

		/* Generate the quads representing the current string characters */
		while(*string) {
			
			struct utf8_codepoint const utf8_codepoint = 
				utf8_codepoint_and_size(string);

			string += utf8_codepoint.size;

			myy_glyph_to_twotris_quad_window_coords(
				glyph_infos, utf8_codepoint.raw,
				quads_buffer+stored_quads, &offset
			);

			stored_quads += 1;

		}
		
		// Offset the next line
		offset.y_offset += vertical_offset_px; // next line
	}
	
	return stored_quads;
}


