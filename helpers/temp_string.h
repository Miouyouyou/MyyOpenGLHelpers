#ifndef MYY_HELPERS_TEMP_STRING_H
#define MYY_HELPERS_TEMP_STRING_H 1

#include "myy/myy.h"

#include <stddef.h>
#include <stdbool.h>


/* TODO Créer une classe pour les chaînes de caractères temporaire.
 * - Pas d'arguments spéciaux à la création.
 * - Capacité de remplir comme un StringBuilder en Java.
 */
struct temp_string_s {
	myy_vector_utf8 vector;
};

typedef struct temp_string_s temp_string_t;

__attribute__((unused))
static inline
char * temp_string_text(
	temp_string_t * __restrict const temp_string)
{
	return (char *) myy_vector_utf8_data(&temp_string->vector);
}

__attribute__((unused))
static inline
bool temp_string_valid(
	temp_string_t * __restrict const temp_string)
{
	return myy_vector_utf8_is_valid(&temp_string->vector);
}

__attribute__((unused))
static inline
size_t temp_string_length(
	temp_string_t const * __restrict const temp_string)
{
	return myy_vector_utf8_length(&temp_string->vector);
}

char * temp_string_text_set(
	temp_string_t * __restrict const temp_string,
	char const * __restrict const text);

__attribute__((format(printf, 2, 3)))
char * temp_string_text_set_format(
	temp_string_t * __restrict const temp_string,
	char const * __restrict const fmt,
	...);

__attribute__((unused))
static inline
temp_string_t temp_string_create(
	char const * __restrict const text)
{
	temp_string_t temp_string = {
		.vector = myy_vector_utf8_init(strlen(text)),
	};
	LOG("[Temp_string_create]\n");
	temp_string_text_set(&temp_string, text);
	return temp_string;
}

__attribute__((unused))
static inline
void temp_string_destroy(
	temp_string_t * __restrict const temp_string)
{
	if (temp_string_valid(temp_string))
		myy_vector_utf8_free_content(temp_string->vector);
}


#endif
