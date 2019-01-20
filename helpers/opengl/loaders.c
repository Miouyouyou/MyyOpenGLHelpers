/* This depends on a generated file */

#include <stdint.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/file.h>
#include <myy/helpers/log.h>
#include <myy/helpers/strings.h>

/* TODO : Replace exit by an "implementation-defined" panic function */
/* exit */
#include <stdlib.h>
#include <unistd.h>

struct gleanup {
	void (*check)(GLuint, GLenum, GLint * );
	int verif;
	void (*log)(GLuint, GLsizei, GLsizei*, GLchar*);
};

#define GL_SHADER_PROBLEMS 0
#define GL_PROGRAM_PROBLEMS 1

static const struct gleanup cleanupMethods[] = {
	[GL_SHADER_PROBLEMS] = {
	  .check = glGetShaderiv,
	  .verif = GL_COMPILE_STATUS,
	  .log   = glGetShaderInfoLog
	},
	[GL_PROGRAM_PROBLEMS] = {
	  .check = glGetProgramiv,
	  .verif = GL_LINK_STATUS,
	  .log   = glGetProgramInfoLog
	}
};

static bool check_if_ok
(GLuint const element, GLuint const method_id)
{
	struct gleanup gheckup = cleanupMethods[method_id];

	GLint ok = GL_FALSE;
	gheckup.check(element, gheckup.verif, &ok);

	if (ok == GL_TRUE) return true;

	int written = 0;
	/* "1KB should be enough for every log" */
	GLchar log_data[1024] = {0};
	gheckup.log(element, 1022, &written, (GLchar *) log_data);
	log_data[written] = 0;
	LOG("Problem was : %s\n", log_data);

	return false;
}

bool glhLinkProgram(
	GLuint const program_id)
{
	glLinkProgram(program_id);
	return check_if_ok(program_id, GL_PROGRAM_PROBLEMS);
}

struct shader_load_status glhLoadShader(
	GLenum const shaderType,
	uint8_t const * __restrict const shader_code,
	GLsizei const shader_code_size,
	GLuint const program)
{

	GLuint shader = glCreateShader(shaderType);
	bool ok = false;

	if (shader) {
		glShaderSource(
			shader, 1,
			(GLchar const * const *) (&shader_code),
			&shader_code_size);
		glCompileShader(shader);
		ok = check_if_ok(shader, GL_SHADER_PROBLEMS);
		if (ok) glAttachShader(program, shader);
		glDeleteShader(shader);
	}
	LOG("[glhLoadShader] Status : %d\n", ok);
	struct shader_load_status status = {
		.ok = ok,
		.shader_id = shader
	};
	return status;
}

struct shader_load_status glhLoadShaderFile(
	GLenum const shaderType,
	char const * __restrict const filepath,
	GLuint const program)
{

	struct myy_fh_map_handle mapping =
		fh_MapFileToMemory(filepath);

	struct shader_load_status status = {
		.ok = false,
		.shader_id = 0
	};
	if (mapping.ok) {
		LOG("Loading shader file : %s\n", filepath);
		status = glhLoadShader(
			shaderType, mapping.address, mapping.length, program);
		fh_UnmapFileFromMemory(mapping);
	}

	return status;
}

GLuint glhBuildProgramFromFiles
(char const * __restrict const vsh_filename,
 char const * __restrict const fsh_filename,
 uint8_t const n_attributes,
 char const * __restrict const attributes_names)
{
	GLuint p = glCreateProgram();
	struct shader_load_status status;
	GLuint vertex_shader   = 0;
	GLuint fragment_shader = 0;
	char const * bound_attribute_name;

	if (p == 0)
		goto could_not_create_program;

	/* Shaders phase */
	LOG("[glhBuildProgramFromFiles]\n");
	LOG("Loading Vertex shader : %s\n", vsh_filename);
	status = glhLoadShaderFile(GL_VERTEX_SHADER, vsh_filename, p);
	if (!status.ok)
		goto could_not_load_vertex_shader;
	vertex_shader = status.shader_id;

	LOG("Loading Fragment shader : %s\n", fsh_filename);
	status = glhLoadShaderFile(GL_FRAGMENT_SHADER, fsh_filename, p);
	if (!status.ok)
		goto could_not_load_fragment_shader;
	fragment_shader = status.shader_id;

	LOG("Shaders loaded\n");
	bound_attribute_name = attributes_names;

	for (uint32_t i = 0; i < n_attributes; i++) {
		glBindAttribLocation(p, i, bound_attribute_name);
		LOG("Attrib : %s - Location : %d\n", bound_attribute_name, i);
		sh_pointToNextString(bound_attribute_name);
	}

	if (!glhLinkProgram(p)) {
		LOG("Could not link the program.\n");
		goto could_not_link_program;
	}

	return p;

could_not_link_program:
	glDeleteShader(fragment_shader);
could_not_load_fragment_shader:
	glDeleteShader(vertex_shader);
could_not_load_vertex_shader:
	glDeleteProgram(p);
could_not_create_program:
	return p;
}

/* TODO : This must be customised */
static void setupTexture(
	struct myy_sampler_properties const sampler_properties)
{
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		sampler_properties.wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		sampler_properties.wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		sampler_properties.max_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		sampler_properties.min_filter);
}

void glhUploadMyyRawTextureData(
	uint8_t const * __restrict const data,
	GLuint const texture_id,
	struct myy_sampler_properties const * __restrict
		const sampler_properties)
{
	struct myy_raw_texture_content const * const tex = 
		(struct myy_raw_texture_content const *)
		data;
	struct myy_raw_texture_header header = tex->header;

	glBindTexture(header.gl_target, texture_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, header.alignment);
	LOG(
		"glPixelStorei(%d)\n"
		"glTexImage2D(%d, %d, %d, %d, %d, %d, %d, %d, %p)\n",
		header.alignment,
		header.gl_target, 0, header.gl_format,
		header.width, header.height, 0,
		header.gl_format, header.gl_type, tex->data);
	glTexImage2D(
		header.gl_target, 0, header.gl_format,
		header.width, header.height, 0,
		header.gl_format, header.gl_type, tex->data);
	setupTexture(*sampler_properties);
}

/**
 *  Create n textures buffers and upload the content of
 *  each \0 separated filename in "textures_names" into these buffers.
 * 
 * The raw files are supposed to follow a specific format, described by
 * the 
 *
 * Example :
 * GLuint textures_id[2];
 * glhUploadMyyRawTextures("tex/first_tex.raw\0tex/second_tex.raw\0", 2,
 *                         textures_id);
 *
 * CAUTION :
 * - This will replace the current active texture binding by a binding
 *   to the last texture uploaded.
 *
 * PARAMS :
 * @param textures_names The filepaths of the textures to upload
 *                       This is implementation specific.
 *                       For example, tex/first_tex.raw will be read
 *                       from the current Asset archive on Android.
 *
 * @param n              The number of textures to upload
 *
 * @param texid          The buffer receiving the generated textures id
 *
 * ADVICE :
 *   Once the textures uploaded, use glhActiveTextures to enable
 * multi-texturing.
 */
void glhUploadMyyRawTextures
(char const * __restrict const textures_names, int const n,
 GLuint * __restrict const texid)
{
	/* OpenGL 2.x way to load textures is certainly NOT intuitive !
	 * From what I understand :
	 * - The current activated texture unit is changed through
	 *   glActiveTexture.
	 * - glGenTextures will generate texture *names*.
	 * - glBindTexture will bind the current *storage* unit to the current
	 *   activated texture unit and, on the first time, will define the
	 *   current *storage* unit parameters.
	 *   Example : This storage unit must store GL_TEXTURE_2D textures.
	 * - glTexImage2D will upload the provided data in the texture *storage* unit
	 *   bound to the current texture unit.
	 */

	glGenTextures(n, texid);

	const char *current_name = textures_names;

	struct myy_sampler_properties sampler_properties =
		myy_sampler_properties_default();

	for (int i = 0; i < n; i++) {
		/* glTexImage2D
		   Specifies a two-dimensional or cube-map texture for the current
		   texture unit, specified with glActiveTexture. */

		LOG("Loading texture : %s\n", current_name);
		struct myy_fh_map_handle mapped_file_infos =
			fh_MapFileToMemory(current_name);
		if (mapped_file_infos.ok) {
			glhUploadMyyRawTextureData(
				(uint8_t const * __restrict)
				mapped_file_infos.address,
				texid[i],
				&sampler_properties);
			fh_UnmapFileFromMemory(mapped_file_infos);
			sh_pointToNextString(current_name);
		}
		else {
			LOG("You're sure about that file : %s ?\n", current_name);
			exit(1);
		}
	}
}

/**
 * Activate and bind the provided textures, in order.
 * The first texture will be activated and bound to GL_TEXTURE0
 * The following will be activated and bound to GL_TEXTURE0+n
 *
 * parameters :
 *   @param texids The OpenGL textures identifiers to activate
 *   @param n_textures The number of textures to activate
 *
 * Warnings :
 *   Does not check if texids is a valid pointer
 *   Does not check if texids contains n_textures
 *   You'll have to upload the textures to the GPU first
 *   before using this procedure.
 */
void glhActiveTextures
(GLuint const * const texids, int const n_textures)
{
	for (unsigned int i = 0; i < n_textures; i++) {
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, texids[i]);
	}
}
