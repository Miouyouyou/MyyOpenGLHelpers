#include <myy/helpers/fonts/packed_fonts_parser.h>

#include <myy/helpers/file.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

#include <myy/helpers/memory.h>

#include <string.h>

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

	print_stored_codepoints_infos(&infos);
	return infos;
}

