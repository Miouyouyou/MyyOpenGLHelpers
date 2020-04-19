#ifndef MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H
#define MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H 1

#include <stdint.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/position.h>
#include <myy/helpers/buffers.h>
#include <myy/helpers/colors.h>

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

__attribute__((unused))
static inline uint16_t myy_text_flow(
	enum myy_text_block_flow const vertical,
	enum myy_text_block_flow const horizontal)
{
	return (uint16_t) ((vertical & 0xff) << 8) | (horizontal & 0xff);
}

__attribute__((unused))
static inline uint16_t myy_text_flow_occidental(void) {
	return myy_text_flow(block_top_to_bottom, block_left_to_right);
}

struct myy_text_properties {
	uint16_t myy_text_flows;
	uint16_t z_layer;
	uint8_t r,g,b,a;
	void * user_metadata;
};

typedef struct myy_text_properties myy_text_props_t;

/* WARNING Currently broken */
__attribute__((unused))
static inline myy_text_props_t myy_text_props_layered_color(
	uint16_t const layer,
	rgba8_t const color)
{
	myy_text_props_t const default_props = {
		.myy_text_flows = myy_text_flow_occidental(),
		.z_layer = layer,
		.r = color.r,
		.g = color.g,
		.b = color.b, 
		.a = color.a,
		.user_metadata = (void *) 0
	};
	return default_props;
}

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
	int64_t const vertical_offset_between_strings);

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
	void * deal_with_generated_quads_user_arg);

#endif
