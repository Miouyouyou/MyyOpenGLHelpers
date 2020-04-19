#ifndef MYY_PACKED_FONTS_PARSER_H
#define MYY_PACKED_FONTS_PARSER_H 1

#include <stdint.h>

#include <myy/helpers/file.h>
#include <myy/helpers/vector.h>

#include <myy/helpers/opengl/loaders.h>
#include <myy/current/opengl.h>

#include <stdlib.h>

/* Please, keep it a power of 2 size */
struct myy_gl_text_quad {
	int16_t left, right, down, up;
	int16_t tex_left, tex_right, tex_down, tex_up;
};

myy_vector_template(quads, struct myy_gl_text_quad)

#define MYYF_SIGNATURE 0x4659594d
struct myy_packed_fonts_info_header {
	uint32_t signature;
	uint32_t n_stored_codepoints;
	uint32_t codepoints_start_offset;
	uint32_t glyphdata_start_offset;
	uint32_t texture_filenames_offset;
	int16_t min_bearing_y;
	uint16_t padding;
	uint32_t unused;
};


struct myy_packed_fonts_textures_filename {
	uint32_t size;
};
struct myy_packed_fonts_textures_filenames_section {
	uint32_t n_filenames;
	/* Followed by myy_packed_fonts_textures_filename */
};

struct myy_packed_fonts_codepoints {
	uint32_t codepoint;
};

struct myy_packed_fonts_glyphdata {
	uint16_t tex_left, tex_right, tex_bottom, tex_top;
	int16_t offset_x_px, offset_y_px;
	int16_t advance_x_px, advance_y_px;
	uint16_t width_px, height_px;
	int16_t padding[2];
};

struct gl_text_infos {
	uint32_t stored_codepoints;
	uint32_t const * __restrict codepoints_addr;
	struct myy_packed_fonts_glyphdata const * __restrict glyphdata_addr;
	myy_vector_quads * __restrict quads;
	int16_t tex_width, tex_height;
	GLuint tex_id;
	int16_t min_bearing_y;
#ifdef __cplusplus
	int16_t default_max_line_height() {
		return -glyphdata_addr->advance_y_px;
	}
#endif
};

typedef struct gl_text_infos gl_text_atlas_t;


bool myy_packed_fonts_load(
	char const * __restrict const filename,
	struct gl_text_infos * __restrict const infos,
	struct myy_fh_map_handle * __restrict const out_handle,
	struct myy_sampler_properties * __restrict const sample_props);

static inline void myy_packed_fonts_unload(
	struct myy_fh_map_handle handle)
{
	fh_UnmapFileFromMemory(handle);
}

#endif
