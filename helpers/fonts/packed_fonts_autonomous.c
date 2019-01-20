#include <myy/helpers/fonts/packed_fonts_parser.h>

#include <myy/helpers/file.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

#include <myy/helpers/memory.h>

#include <string.h>

struct myy_fonts_autonomous_display {
	GLuint gl_program_id;
	GLuint gl_unif_global_offset;
	struct gl_text_infos gl_text;
	myy_vector_t quads_to_store_in_gpu;
};

struct gl_text_point {
	GLshort x, y, z, w;
	GLshort s, t;
};
union gl_text_quad {
	struct gl_text_point points[6];
	struct {
		struct {
			struct gl_text_point up_left, down_left, up_right;
		} left;
		struct {
			struct gl_text_point down_right, down_left, up_right;
		} right;
	} triangles;
};

static inline add_quad_to_store(
	myy_vector_t * __restrict const quads_to_store_in_gpu,
	struct myy_gl_text_quad const * __restrict const quad)
{
	union gl_text_quad gl_quad = {
		.triangles = {
			.left = {
				.up_left = {
					.x = quad->left,
					.y = quad->up,
					.z = quad->z,
					.s = quad->tex_left,
					.t = quad->tex_up
				},
				.down_left = {
					.x = quad->left,
					.y = quad->down,
					.z = quad->z,
					.s = quad->tex_left,
					.t = quad->tex_down
				},
				.up_right = {
					.x = quad->right,
					.y = quad->up,
					.z = quad->z,
					.s = quad->tex_right,
					.t = quad->tex_up
				}
			},
			.right = {
				.down_right = {
					.x = quad->right,
					.y = quad->down,
					.z = quad->z,
					.s = quad->tex_right,
					.t = quad->tex_down
				},
				.down_left = {
					.x = quad->left,
					.y = quad->down,
					.z = quad->z,
					.s = quad->tex_left,
					.t = quad->tex_down
				},
				.up_right = {
					.x = quad->right,
					.y = quad->up,
					.z = quad->z,
					.s = quad->tex_right,
					.t = quad->tex_up
				}
			}
		}
	}

	myy_vector_add(quads_to_store_in_gpu, gl_quad);
}
void myy_gl_text_add(
	struct myy_fonts_autonomous_display const * __restrict const state,
	char const * __restrict const text_to_display,
	position_S * __restrict const position)
{
	
}

static void print_stored_codepoints_infos
(struct gl_text_infos const * __restrict const glyph_infos) {
	unsigned int n_codepoints = glyph_infos->stored_codepoints;
	struct myy_packed_fonts_glyphdata const * __restrict const glyphs =
		glyph_infos->glyphdata_addr;
	uint32_t const * __restrict const codepoints =
		glyph_infos->codepoints_addr;


	for (unsigned int i = 0; i < n_codepoints; i++) {
		print_codepoint_and_metadata(glyphs, codepoints, i);
	}
}

struct gl_text_infos myy_packed_fonts_load(
	char const * __restrict const filename,
	struct myy_fh_map_handle * __restrict const out_handle)
{

	/* Why do a copy though ?
	 * We could just put const pointers to the mapped file...
	 */
	struct myy_fh_map_handle mapping = fh_MapFileToMemory(filename);
	struct myy_packed_fonts_info_header const * __restrict header;

	if (out_handle != NULL)
		*out_handle = mapping;

	if (!mapping.ok) {
		LOG("Could not open the font metadata : %s\n", filename);
		struct gl_text_infos bogus = {0, 0, 0};
		return bogus;
	}

	header =
		(struct myy_packed_fonts_info_header const * __restrict)
		mapping.address;

	LOG("[myy_parse_packed_fonts]\n"
	    "  filename                   : %s\n"
	    "  Codepoints stored          : %d\n"
	    "  Codepoints starting offset : %d\n"
	    "  Glyph data start offset    : %d\n"
	    "  Texture names offset       : %d\n",
	    filename,
	    header->n_stored_codepoints,
	    header->codepoints_start_offset,
	    header->glyphdata_start_offset,
	    header->texture_filenames_offset);


	struct gl_text_infos infos = {
		.stored_codepoints = header->n_stored_codepoints,
		.codepoints_addr =
			(mapping.address + header->codepoints_start_offset),
		.glyphdata_addr =
			(mapping.address + header->glyphdata_start_offset)
	};

	//print_stored_codepoints_infos(&infos);
	return infos;
}

