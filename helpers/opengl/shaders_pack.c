#include <myy/helpers/log.h>
#include <myy/helpers/opengl/shaders_pack.h>
#include <myy/helpers/opengl/loaders.h>

#include <stdlib.h>
#include <string.h>

static void print_header(
	struct pack_header const * __restrict const header)
{
	LOG(
		"Signature       : 0x%08x\n"
		"Data offset     : 0x%04x\n"
		"Metadata offset : 0x%04x\n"
		"N programs      : %d\n",
		header->signature,
		header->data_offset,
		header->metadata_offset,
		header->n_programs);
}

GLuint myy_shaders_pack_compile_program(
	char const * __restrict program_name,
	struct program_definition_header const * __restrict const header,
	uint8_t const * __restrict const data,
	uint8_t * __restrict const runtime_data)
{
	struct shaders_metadata    const * __restrict shaders_mdata;
	struct attributes_metadata const * __restrict attributes_mdata;
	struct uniforms_metadata   const * __restrict uniforms_mdata;

	GLuint  * __restrict shaders_id;
	int_fast32_t n_shader_id = 0;

	uint8_t const * __restrict mdata_cursor =
		(uint8_t const * __restrict) header;

	GLuint * __restrict const linked_id =
		(GLuint * __restrict)
		(runtime_data + header->offset_in_struct);
	GLuint program;

	LOG("[myy_shaders_pack_compile_program]\n"
		"\tLoading program : %s\n",
		program_name);
	program = glCreateProgram();

	if (program == 0) {
		goto glcreateprogram_failed_miserably;
	}

	mdata_cursor += header->section_size;

	shaders_mdata =
		(struct shaders_metadata const * __restrict)
		mdata_cursor;

	/* Allocate temporary memory to store the shaders identifiers.
	 * In case something goes wrong, we deallocate all the shaders
	 * and delete the program in reverse order.
	 */
	shaders_id =
		(GLuint * __restrict)
		malloc(sizeof(GLuint) * shaders_mdata->n_shaders);

	if (shaders_id == NULL)
		goto could_not_alloc_shaders_id;

	for (n_shader_id = 0;
	     n_shader_id < shaders_mdata->n_shaders;
	     n_shader_id++)
	{

		struct shader_load_status status = glhLoadShader(
			shaders_mdata->shader[n_shader_id].shader_type,
			(data + shaders_mdata->shader[n_shader_id].data_offset),
			shaders_mdata->shader[n_shader_id].data_size,
			program
		);
		
		if (status.ok) 
			shaders_id[n_shader_id] = status.shader_id;
		else {
			goto one_shader_failed_to_compile;
		}
	}

	mdata_cursor += shaders_mdata->section_size;
	attributes_mdata =
		(struct attributes_metadata const * __restrict)
		mdata_cursor;

	for (uint32_t i = 0; i < attributes_mdata->n_attributes; i++) {
		glBindAttribLocation(
			program,
			attributes_mdata->attribute[i].bind_id,
			(GLchar const *)
			(data + attributes_mdata->attribute[i].name_offset)
		);
	}

	if (!glhLinkProgram(program)) {
		glDeleteProgram(program);
		for (int32_t j = n_shader_id - 1; j >= 0; j--)
			glDeleteShader(shaders_id[j]);

		free(shaders_id);
		goto could_not_link_program;
	}

	/* Now that everything went fine : 
	 * delete previously set program id.
	 * 
	 * No need to worry about the previously written uniforms ids.
	 * We're going to overwrite them a few lines below.
	 * Every "GetUniformLocation" miss will just generate
	 * -1 afterwards. Using -1 with "Uniform*" just generate
	 * no result.
	 * 
	 * It is expected that the structure is initialized to 0 on
	 * the first run.
	 */
	if (*linked_id)
		glDeleteProgram(*linked_id);

	*linked_id = program;

	mdata_cursor += attributes_mdata->section_size;
	uniforms_mdata =
		(struct uniforms_metadata const * __restrict)
		mdata_cursor;

	for (uint32_t i = 0; i < uniforms_mdata->n_uniforms; i++) {
		GLuint * __restrict const uniform_id =
			(GLuint * __restrict)
			(runtime_data+uniforms_mdata->uniform[i].offset_in_struct);
		*uniform_id = glGetUniformLocation(
			program,
			(GLchar const *)
			(data + uniforms_mdata->uniform[i].name_offset));
	}

	return program;

could_not_link_program:
one_shader_failed_to_compile:
	for (int32_t j = n_shader_id - 1; j >= 0; j--)
		glDeleteShader(shaders_id[j]);
	free(shaders_id);
could_not_alloc_shaders_id:
	glDeleteProgram(program);
glcreateprogram_failed_miserably:
	LOG("	Could not load the program correctly\n");
	return 0;
}

bool myy_shaders_pack_load_all_programs(
	uint8_t const * __restrict const pack,
	uint8_t       * __restrict const data_structure)
{
	struct pack_header const * __restrict const header =
		(struct pack_header const * __restrict)
		pack;
	uint8_t const * __restrict data_section;
	uint8_t const * __restrict metadata_section;
	struct program_metadata * __restrict programs;
	bool all_programs_load_status = true;

	if (pack == NULL) {
		LOG("Pack is NULL...");
		goto no_pack;
	}

	if (header->signature != MYYP_SIGNATURE) {
		LOG("Bad signature");
		goto bad_signature;
	}

	print_header(header);
	data_section     = pack + header->data_offset;
	metadata_section = pack + header->metadata_offset;
	programs         = 
		(struct program_metadata * __restrict)
		(pack + sizeof(*header));

	for (uint_fast32_t i = 0; i < header->n_programs; i++) {
		GLuint p = myy_shaders_pack_compile_program(
			/* The name */
			(char const * __restrict)
			(data_section + programs[i].name_offset),
			/* The metadata */
			(struct program_definition_header const * __restrict)
			(metadata_section + programs[i].metadata_offset),
			/* The data section itself */
			data_section,
			/* the data structure receiving the loaded identifiers */
			data_structure);
		all_programs_load_status &= (p != 0);
	}

	if (!all_programs_load_status)
		LOG("Some programs failed to load :C\n");

	return all_programs_load_status;

bad_signature:
no_pack:
	return false;
}

bool myy_shaders_pack_load_all_programs_from_file(
	char    const * __restrict const filepath,
	uint8_t       * __restrict const data_structure)
{
	struct myy_fh_map_handle mapping = fh_MapFileToMemory(filepath);

	bool all_programs_load_status = false;
	if (mapping.ok) {
		all_programs_load_status = myy_shaders_pack_load_all_programs(
			(uint8_t const * __restrict)
			mapping.address,
			data_structure);
		fh_UnmapFileFromMemory(mapping);
	}

	return all_programs_load_status;
}

GLuint myy_shaders_pack_load_program(
	uint8_t const * __restrict const pack,
	uint8_t       * __restrict const data_structure,
	char    const * __restrict const program_name,
	size_t  const program_name_size)
{
	struct pack_header const * __restrict const header =
		(struct pack_header const * __restrict)
		pack;
	uint8_t const * __restrict data_section;
	uint8_t const * __restrict metadata_section;
	struct program_metadata * __restrict programs;
	GLuint loaded_program = 0;

	if (header->signature != MYYP_SIGNATURE) {
		LOG("Bad signature");
		goto out;
	}

	data_section     = pack + header->data_offset;
	metadata_section = pack + header->metadata_offset;
	programs         = 
		(struct program_metadata * __restrict)
		(pack + sizeof(*header));

	for (uint_fast32_t i = 0; i < header->n_programs; i++) {
		char const * __restrict const current_program_name =
			(char const * __restrict)
			(data_section + programs[i].name_offset);
		bool it_is_the_program_asked = (
			strncmp(
				program_name, 
				current_program_name,
				program_name_size)
			== 0);
		if (it_is_the_program_asked) {
			loaded_program = myy_shaders_pack_compile_program(
				/* The name */
				current_program_name,
				/* The metadata */
				(struct program_definition_header const * __restrict)
				(metadata_section + programs[i].metadata_offset),
				/* The data section itself */
				data_section,
				/* the data structure receiving
				 * the loaded identifiers */
				data_structure);
			break;
		}
	}

	if (loaded_program == 0)
		LOG("Could not load program %s\n", program_name);

out:
	return loaded_program;
}
