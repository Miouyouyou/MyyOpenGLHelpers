#ifndef MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H
#define MYY_SRC_FONTS_PACKED_FONTS_DISPLAY_H 1

#include <stdint.h>
#include <myy/helpers/opengl/quads_structures.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>

struct qinfos {
	int16_t quad_w, quad_h, q_off_x, q_off_y;
	int16_t tex_w, tex_h, tex_off_x, tex_off_y;
};

int16_t myy_glyph_to_twotris_quad
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 US_two_tris_quad_3D * __restrict const quad,
 int16_t x_offset_px);

int16_t myy_glyph_to_twotris_quad_window_coords
(struct glyph_infos const * __restrict const glyph_infos,
 uint32_t const codepoint,
 US_two_tris_quad_3D * __restrict const quad,
 int16_t x_offset_px);

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

#endif
