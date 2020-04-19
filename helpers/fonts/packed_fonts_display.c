#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

#include <stdint.h>

static unsigned int find_codepoint_in(
	uint32_t const * __restrict const codepoints,
	uint32_t const searched_codepoint)
{
	unsigned int i = 1;
	while (codepoints[i] && (codepoints[i] != searched_codepoint))
		i++;
	unsigned int found_index =
		i * (codepoints[i] == searched_codepoint);
	return found_index;
}



static void print_codepoint_and_metadata(
	struct myy_packed_fonts_glyphdata const * __restrict const glyphs,
	uint32_t const * __restrict const codepoints,
	uint_fast32_t const index)
{
	struct myy_packed_fonts_glyphdata const glyph = glyphs[index];
	uint32_t codepoint = codepoints[index];
	char codepoint_str[5] = {0};
	utf32_to_utf8_string(codepoint, codepoint_str);
	printf(
		"Codepoint  : %d (%s)\n"
		"Glyph_data\n"
		"\tTexture (%d←→%d) (%d↓↑%d)\n"
		"\tOrigin offset (→%d, ↓%d)\n"
		"\tAdvance (→%d, ↓%d)\n",
		codepoint, codepoint_str,
		glyph.tex_left,     glyph.tex_right,
		glyph.tex_bottom,   glyph.tex_top,
		glyph.offset_x_px,  glyph.offset_y_px,
		glyph.advance_x_px, glyph.advance_y_px);
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
 * systems (like European, Arabic and some Asiatic languages), when
 * you'll create a window, you'll "start writing at y pixels from
 * the TOP".
 * You could said that you'll start writing at -y pixels from the
 * TOP but that does not appear natural to me.
 */
int16_t myy_glyph_to_quad_window_coords(
	struct gl_text_infos const * __restrict const gl_text_infos,
	uint32_t const codepoint,
	position_S const text_pos)
{

	int16_t new_x = text_pos.x;

	uint32_t const * __restrict const codepoints =
		gl_text_infos->codepoints_addr;

	unsigned int codepoint_index =
		find_codepoint_in(codepoints, codepoint);

	if (codepoint_index) {
		struct myy_packed_fonts_glyphdata const * __restrict const glyphdata =
		  gl_text_infos->glyphdata_addr+codepoint_index;
		myy_vector_quads * __restrict const quads =
			gl_text_infos->quads;

		int16_t const glyph_x_offset_px =
			new_x + glyphdata->offset_x_px;
		/* FIXME offset_y_px is computed to
		 * generate a double negative here... Ugh... */
		int16_t const glyph_y_offset_px =
			text_pos.y - glyphdata->offset_y_px;
		int16_t const right_px =
			glyph_x_offset_px + glyphdata->width_px;
		int16_t const up_px    =
			glyph_y_offset_px - glyphdata->height_px;
		int16_t const advance_x =
			new_x + glyphdata->advance_x_px;

		struct myy_gl_text_quad text_quad = {
			.left      = glyph_x_offset_px,
			.right     = right_px,
			.down      = glyph_y_offset_px,
			.up        = up_px,
			.tex_left  = (int16_t) glyphdata->tex_left,
			.tex_right = (int16_t) glyphdata->tex_right,
			.tex_down  = (int16_t) glyphdata->tex_top,
			.tex_up    = (int16_t) glyphdata->tex_bottom // TODO : Fix this in the texture !!
		};
		myy_vector_quads_add(quads, 1, &text_quad);

		new_x = advance_x;
	}
	
	return new_x;
}

/* Only deal with left-to-right, top-to-bottom right now */
/* TODO Cache the newline size somewhere. It's not going to change.
 */
static inline void position_to_new_line(
	position_S * __restrict const current_draw_position,
	position_S const initial_position,
	int16_t const newline_size,
	struct myy_text_properties const * __restrict const current_metadata)
{
	/* myy_text_flows is supposed to be :
	 * (binary representation)
	 * [00000000|00000000| 00000000 |00000000]
	 *   unused   unused  block_flow text_flow
	 * 
	 * uint32_t text_flow = current_metadata->myy_text_flows;
	 */
	current_draw_position->x  = initial_position.x;
	current_draw_position->y += newline_size;
}

void myy_string_to_quads(
	struct gl_text_infos const * __restrict const gl_text_infos,
	uint8_t const * __restrict utf8_string,
	position_S * __restrict const draw_at_px,
	struct myy_text_properties const * __restrict const current_metadata,
	void (*deal_with_generated_quads)(
		void * __restrict user_arg,
		struct myy_gl_text_quad const * __restrict const quads,
		uint32_t n_quads,
		struct myy_text_properties const * __restrict const metadata),
	void * deal_with_generated_quads_user_arg)
{
	position_S const start_pos_px = *draw_at_px;
	position_S current_pos_px = *draw_at_px;

	/* TODO : Store a header that specifies the newline offsets
	 * when displaying top-to-bottom, left-to-right.
	 * Inverse the value if drawing in the other direction
	 */
	struct myy_packed_fonts_glyphdata const * __restrict const glyphdata =
		  gl_text_infos->glyphdata_addr;
	myy_vector_quads_reset(gl_text_infos->quads);

	int16_t new_line_size = -glyphdata->advance_y_px;
	while (*utf8_string) {
		struct utf8_codepoint const utf8_codepoint = 
			utf8_codepoint_and_size(utf8_string);
		utf8_string += utf8_codepoint.size;

		if (utf8_codepoint.raw != 0xA) {
			current_pos_px.x = myy_glyph_to_quad_window_coords(
				gl_text_infos, utf8_codepoint.raw, current_pos_px);
		}
		else {
			/* TODO That only works with :
			* - Left to Right, Down to Bottom writings */
			position_to_new_line(
				&current_pos_px, start_pos_px, new_line_size, current_metadata);
		}
	}

	myy_vector_quads * __restrict const quads = gl_text_infos->quads;
	deal_with_generated_quads(
		deal_with_generated_quads_user_arg,
		myy_vector_quads_data(quads),
		myy_vector_quads_length(quads),
		current_metadata);
	*draw_at_px = current_pos_px;
}

void myy_gl_text_infos_chars_to_quads(
	struct gl_text_infos const * __restrict const gl_text_infos,
	uint8_t const * __restrict utf8_string,
	size_t const utf8_string_size,
	position_S * __restrict const draw_at_px,
	struct myy_text_properties const * __restrict const current_metadata,
	void (*deal_with_generated_quads)(
		void * __restrict user_arg,
		struct myy_gl_text_quad const * __restrict const quads,
		uint32_t n_quads,
		struct myy_text_properties const * __restrict const metadata),
	void * deal_with_generated_quads_user_arg)
{
	position_S const start_pos_px = *draw_at_px;
	position_S current_pos_px = *draw_at_px;

	/* TODO : Store a header that specifies the newline offsets
	 * when displaying top-to-bottom, left-to-right.
	 * Inverse the value if drawing in the other direction
	 */
	struct myy_packed_fonts_glyphdata const * __restrict const glyphdata =
		  gl_text_infos->glyphdata_addr;
	myy_vector_quads_reset(gl_text_infos->quads);

	int16_t new_line_size = -glyphdata->advance_y_px;
	uint8_t const * __restrict const utf8_string_end =
		utf8_string + utf8_string_size;

	while (utf8_string < utf8_string_end) {
		struct utf8_codepoint const utf8_codepoint = 
			utf8_codepoint_and_size(utf8_string);
		utf8_string += utf8_codepoint.size;

		if (utf8_codepoint.raw != 0xA) {
			current_pos_px.x = myy_glyph_to_quad_window_coords(
				gl_text_infos, utf8_codepoint.raw, current_pos_px);
		}
		else {
			/* TODO That only works with :
			* - Left to Right, Down to Bottom writings */
			position_to_new_line(
				&current_pos_px, start_pos_px, new_line_size, current_metadata);
		}
	}

	myy_vector_quads * __restrict const quads = gl_text_infos->quads;
	deal_with_generated_quads(
		deal_with_generated_quads_user_arg,
		myy_vector_quads_data(quads),
		myy_vector_quads_length(quads),
		current_metadata);
}

void myy_strings_list_to_quads(
	struct gl_text_infos const * __restrict const gl_text_infos,
	uint8_t const * __restrict const * __restrict utf8_strings,
	position_S * __restrict const draw_at_px,
	struct myy_text_properties const * __restrict const current_metadata,
	void (*deal_with_generated_quads)(
		void * __restrict user_arg,
		struct myy_gl_text_quad const * __restrict const quads,
		uint32_t n_quads,
		struct myy_text_properties const * __restrict const metadata),
	void * deal_with_generated_quads_user_arg,
	int64_t const vertical_offset_between_strings)
{

	int16_t line_start = draw_at_px->x;
	for (
		uint8_t const * __restrict current_utf8_string = *utf8_strings++;
		current_utf8_string != NULL;
		current_utf8_string = *utf8_strings++)
	{
		myy_string_to_quads(
			gl_text_infos,
			current_utf8_string,
			draw_at_px,
			current_metadata,
			deal_with_generated_quads,
			deal_with_generated_quads_user_arg);
		draw_at_px->x  = line_start;
		draw_at_px->y += vertical_offset_between_strings;
	}
}
