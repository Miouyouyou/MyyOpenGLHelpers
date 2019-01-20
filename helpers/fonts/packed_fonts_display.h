#ifndef MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H
#define MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H 1

#include <stdint.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/position.h>
#include <myy/helpers/buffers.h>

enum myy_text_line_flow {
	line_left_to_right,
	line_right_to_left,
	line_top_to_bottom,
	line_bottom_to_top
};

enum myy_text_block_flow {
	block_top_to_bottom,
	block_left_to_right,
	block_right_to_left,
	block_bottom_to_top
};

struct myy_text_properties {
	int32_t myy_text_flows;
	int32_t z_layer;
	uint8_t r,g,b,a;
	void * user_metadata;
};

/* WARNING Currently broken */
int16_t myy_glyph_to_twotris_quad_window_coords(
	struct gl_text_infos const * __restrict const gl_text_infos,
	uint32_t const codepoint,
	position_S const text_pos);

int16_t myy_glyph_to_quad_window_coords(
	struct gl_text_infos const * __restrict const gl_text_infos,
	uint32_t const codepoint,
	position_S const text_pos);

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
	void * deal_with_generated_quads_user_arg);

#endif
