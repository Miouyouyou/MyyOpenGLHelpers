#ifndef MYY_SHADERS_PACK_H
#define MYY_SHADERS_PACK_H 1

#include <myy/current/opengl.h>
#include <myy/helpers/c_types.h>

#include <stdint.h>

#define MYYP_SIGNATURE 0x5059594D
/* The whole structure is designed like this :
 * [HEADER]   : pack_header
 * [PROGRAMS] : program_metadata (Name, Metadata_offset)
 * -- Metadata 
 * +--> [PROGRAM_SECTION] : program_definition_header
 * +--> [SHADERS_SECTION] : shaders_metadata
 * +--> [ATTRIBUTES_SECTION] : attributes_metadata
 * +--> [UNIFORMS_SECTION]   : uniforms_metadata
 * 
 * The "offset in struct" refers to a big data structure,
 * which definition is generated with the .pack file.
 * This data structure makes literal program a bit more
 * "literate", like :
 * 
 * glUseProgram(myy_programs.heatmap.id);
 * glUniform1i(myy_programs.heatmap.uniform.sampler, ...);
 * 
 * In order to initialize the program id and the different
 * uniforms ids inside the data structure, the offset of
 * each id inside the data structure is saved in the .pack
 * file and reused when loading a program with the provided
 * utility functions.
 */

/* Absolute offsets start from the beginning of the file */
struct pack_header {
	/* Must be 0x5059594D */
	uint32_t signature;
	/* Absolute offset of the data section */
	uint32_t data_offset;
	/* Absolute offset of the metadata section */
	uint32_t metadata_offset;
	/* Number of programs stored in the pack */
	uint32_t n_programs;
};

struct program_metadata {
	/* The name string relative offset in the data section */
	uint32_t name_offset;
	/* The associated metadata relative offset
	 * in the metadata section */
	uint32_t metadata_offset;
};

struct program_definition_header {
	/* The size of the section.
	 * Used to move a pointer when scanning parsing the metadata.
	 * Example :
	 * [...]
	 * uint8_t const * __restrict cursor = metadata_start;
	 * 
	 * struct program_definition_header * __restrict header =
	 * 	(struct program_definition_header * __restrict)
	 * 	cursor;
	 * 
	 * [Parse header]
	 * 
	 * cursor += header->section_size;
	 * 
	 * struct shaders_metadata * __restrict shaders_mdata =
	 * 	(struct shaders_metadata * __restrict)
	 * 	cursor;
	 * 
	 * [Parse shaders metadata]
	 * 
	 * [...]
	 */
	uint32_t section_size;
	uint32_t offset_in_struct;
};

struct shader_metadata {
	/* The GLenum value (e.g. GL_FRAGMENT_SHADER) */
	uint32_t shader_type;
	/* The size of the shader data */
	uint32_t data_size;
	/* The relative offset of the shader data in the data section */
	uint32_t data_offset;
};

struct shaders_metadata {
	uint32_t section_size;
	uint32_t n_shaders;
	struct shader_metadata shader[];
};

struct attribute_metadata {
	/* The relative offset of the attribute name in the data section */
	uint32_t name_offset;
	/* The id used to bind the attribute.
	 * Identical to the associated enum value for this attribute.
	 */
	uint32_t bind_id;
};

struct attributes_metadata {
	uint32_t section_size;
	uint32_t n_attributes;
	struct attribute_metadata attribute[];
};

struct uniform_metadata {
	/* The relative offset of the id in the big data structure */
	uint32_t offset_in_struct;
	/* The relative offset of the uniform name in the data section */
	uint32_t name_offset;
};

struct uniforms_metadata {
	uint32_t section_size;
	uint32_t n_uniforms;
	struct uniform_metadata uniform[];
};

GLuint myy_shaders_pack_compile_program(
	char    const * __restrict program_name,
	struct program_definition_header const * __restrict const header,
	uint8_t const * __restrict const data,
	uint8_t       * __restrict const runtime_data);

bool myy_shaders_pack_load_all_programs(
	uint8_t const * __restrict const pack,
	uint8_t       * __restrict const data_structure);

bool myy_shaders_pack_load_all_programs_from_file(
	char    const * __restrict const filepath,
	uint8_t       * __restrict const data_structure);

GLuint myy_shaders_pack_load_program(
	uint8_t const * __restrict const pack,
	uint8_t       * __restrict const data_structure,
	char    const * __restrict const program_name,
	size_t  const program_name_size);

#endif
