#ifndef MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H
#define MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H 1

#include <stdint.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>

struct qinfos {
	int16_t quad_w, quad_h, q_off_x, q_off_y;
	int16_t tex_w, tex_h, tex_off_x, tex_off_y;
};

struct text_offset {
	unsigned int x_offset, y_offset;
};

int16_t myy_glyph_to_twotris_quad
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 US_two_tris_quad_3D * __restrict const quad,
 int16_t x_offset_px);

void myy_glyph_to_twotris_quad_window_coords
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 US_two_tris_quad_3D * __restrict const quad,
 struct text_offset * __restrict const offset);

void myy_codepoints_to_glyph_twotris_quads
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 US_two_tris_quad_3D * __restrict const quads);

void myy_codepoints_to_glyph_twotris_quads_window_coords
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 US_two_tris_quad_3D * __restrict const quads);

void myy_codepoints_to_glyph_qinfos
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const * __restrict const string,
 unsigned int const n_characters,
 struct qinfos * __restrict const quads);

int16_t myy_glyph_to_qinfos
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 struct qinfos * __restrict const quad,
 int16_t x_offset_px);

struct generated_quads myy_strings_to_quads_va
(struct glyph_infos const * __restrict const glyph_infos,
 unsigned int const n_strings,
 uint8_t const * const * __restrict const strings,
 uint8_t * __restrict const buffer,
 int16_t const vertical_offset_px);

struct generated_quads myy_single_string_to_quads
(struct glyph_infos const * __restrict const glyph_infos,
 uint8_t const * __restrict string,
 uint8_t const * __restrict const quads_buffer_address,
 struct text_offset * __restrict const text_offset);

#endif
