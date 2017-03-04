#ifndef MYY_SRC_HELPERS_OPENGL_LOADERS
#define MYY_SRC_HELPERS_OPENGL_LOADERS 1

#include <myy/current/opengl.h>
#include <myy/generated/opengl/data_config.h>
#include <stdint.h>

int glhLoadShader
(GLenum const shaderType, char const * __restrict const name,
 GLuint const program);

unsigned int glhCompileProgram
(struct glsl_programs_shared_data const * __restrict const metadata,
 unsigned int const n_shaders,
 enum glsl_shader_name const * __restrict const shaders);

unsigned int glhLinkAndSaveProgram
(struct glsl_programs_shared_data * __restrict const metadata,
 enum glsl_program_name const program_index,
 GLuint const p);

unsigned int glhBuildAndSaveProgram
(struct glsl_programs_shared_data * __restrict const metadata,
 unsigned int const n_shaders,
 enum glsl_shader_name const * __restrict const shaders,
 enum glsl_program_name const program_index);

unsigned int glhBuildAndSaveSimpleProgram
(struct glsl_programs_shared_data * __restrict const metadata,
 enum glsl_shader_name vertex_shader,
 enum glsl_shader_name fragment_shader,
 enum glsl_program_name const program_index);

GLuint glhSetupProgram
(char const * __restrict const vsh_filename,
 char const * __restrict const fsh_filename,
 uint8_t const n_attributes,
 char const * __restrict const attributes_names);

GLuint glhSetupAndUse
(char const * __restrict const vsh_filename,
 char const * __restrict const fsh_filename,
 uint8_t n_attributes,
 char const * __restrict const attributes_names);

struct myy_raw_texture_content {
	/* The texture width */
	uint32_t const width;
	/* The texture height */
	uint32_t const height;
	/* myy_target = gl_target */
	uint32_t const myy_target;
	/* myy_format = gl_format */
	uint32_t const myy_format;
	/* myy_type   = gl_type   */
	uint32_t const myy_type;
	/* Used for glPixelStorei */
	uint32_t const alignment;
	/* The texture raw data */
	uint32_t const data[];
};

/**
 *  Create n textures buffers and upload the content of
 *  each \0 separated filename in "textures_names" into these buffers.
 * 
 * The raw files are supposed to follow a specific format, described by
 * struct myy_raw_texture_content .
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
 GLuint * __restrict const texid);

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
(GLuint const * const texids, int const n_textures);

#endif
