#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/current/opengl.h>

#include <myy/helpers/file.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

#include <myy/helpers/memory.h>
#include <myy/helpers/c_types.h>

#include <string.h>
#include <stdint.h>
#include <unistd.h> // chdir, getcwd

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
		glyph.advance_x_px, -glyph.advance_y_px);
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

/* TODO : That's kind of ugly... I'll have to take care of that.
 * Note that I'd still like to return the whole structure at
 * once, without any pointer thing or whatever.
 */
myy_vector_quads text_quads;

static bool load_texture_and_get_metadata(
	struct myy_packed_fonts_info_header const * __restrict const header,
	struct gl_text_infos * __restrict const infos,
	struct myy_sampler_properties * __restrict const sampler_props)
{
	static char current_pwd[512];
	bool everything_went_fine = false;

	char const * __restrict const texture_filename =
		(char const * __restrict)
		(((uintptr_t) header)
		+ header->texture_filenames_offset
		+ sizeof(struct myy_packed_fonts_textures_filenames_section)
		+ sizeof(struct myy_packed_fonts_textures_filename));

	struct myy_fh_map_handle texture_mapping;
	struct myy_raw_texture_header const * __restrict texture_header;

	texture_mapping = myy_asset_map_to_memory(
		texture_filename, myy_asset_type_texture);

	if (!texture_mapping.ok) {
		LOG("Couldn't load texture %s\n", texture_filename);
		goto out;
	}

	texture_header =
		(struct myy_raw_texture_header const * __restrict)
		texture_mapping.address;

	infos->tex_width  = texture_header->width;
	infos->tex_height = texture_header->height;

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glhUploadMyyRawTextureData(
		texture_mapping.address, texture_id, sampler_props);
	infos->tex_id = texture_id;

	fh_UnmapFileFromMemory(texture_mapping);

out:
	chdir(current_pwd);
didnt_change_directory:
	return everything_went_fine;
}

bool myy_packed_fonts_load(
	char const * __restrict const filename,
	struct gl_text_infos * __restrict const infos,
	struct myy_fh_map_handle * __restrict const out_handle,
	struct myy_sampler_properties * __restrict const sample_props)
{

	bool everything_went_fine = false;

	struct myy_fh_map_handle mapping;
	struct myy_packed_fonts_info_header const * __restrict header;

	if (infos == NULL) {
		LOG("2nd argument can't be NULL in myy_packed_fonts_load.\n");
		goto out;
	}

	/* This buffer is used to generate the quads metadata
	 * that can display a string.
	 * Each quad represents a letter of the currently generated
	 * string. (position, dimensions, texture coordinates)
	 * After generating each string, we pass this buffer to
	 * a user-defined function pointer that deal with the
	 * generated buffer (store it in GPU memory,
	 * throw it away, ...).
	 * 
	 * The buffer is reused when generating each string.
	 * Now, it's best to avoid reallocating more memory during
	 * the procedure, as it will slow down the generation.
	 * 
	 * 1024 is the number of quads that can be stored in one run,
	 * without reallocating memory.
	 * When storing more, we'll try to increase the size of the
	 * vector dynamically with "realloc".
	 * 
	 * Since each string will be a pretty small string, ATM,
	 * 1024 should do the trick.
	 */
	text_quads = myy_vector_quads_init(1024);

	if (!myy_vector_quads_is_valid(&text_quads)) {
		LOG(
			"Could not get initial memory space for "
			"temporary string quads\n");
		goto out;
	}

	mapping = fh_MapFileToMemory(filename);
	if (out_handle != NULL)
		*out_handle = mapping;

	if (!mapping.ok) {
		LOG("Could not open the font metadata : %s\n", filename);
		goto out;
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

	infos->stored_codepoints = header->n_stored_codepoints;
	infos->codepoints_addr =
			(mapping.address + header->codepoints_start_offset);
	infos->glyphdata_addr =
			(mapping.address + header->glyphdata_start_offset);
	infos->quads = &text_quads;

	everything_went_fine =
		load_texture_and_get_metadata(header, infos, sample_props);

out:
	//print_stored_codepoints_infos(infos);
	return everything_went_fine;
}

