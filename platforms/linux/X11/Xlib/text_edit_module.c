#include <src/widgets/text_edit_module.h>
#include <myy/myy.h>



bool myy_text_edit_module_init(
	struct myy_text_edit_module * __restrict const text_edit_module)
{
	/* TODO Can't we have a default implementation instead ? */
	text_edit_module->flags = myy_text_edit_module_flag_inactive;
	text_edit_module->edited_buffer   = NULL;
	text_edit_module->area            = NULL;
	text_edit_module->edited_buffer_insertion_before = 0;
	text_edit_module->edited_buffer_insertion_after  = 0;
	/* NOTE 4096 octets doesn't mean 4096 UTF-8 characters,
	 *      as these can be encoded on multiple bytes
	 */
	text_edit_module->inserted_data_buffer =
		myy_vector_utf8_init(4096);
	
	return myy_vector_utf8_is_valid(
		&text_edit_module->inserted_data_buffer);
}

void myy_text_edit_module_attach(
	struct myy_text_edit_module * __restrict const text_edit_module,
	struct myy_text_area * __restrict const text_area,
	off_t insertion_point,
	myy_states * __restrict const states)
{
	text_edit_module->edited_buffer  = &text_area->value;
	text_edit_module->area           = text_area;
	text_edit_module->edited_buffer_insertion_before = insertion_point;
	text_edit_module->edited_buffer_insertion_after  = insertion_point;
	myy_vector_utf8_reset(&text_edit_module->inserted_data_buffer);
	/* Set it last, just in case something accessed this flag
	 * before everything was setup.
	 */
	text_edit_module->flags = myy_text_edit_module_flag_editing;
}

void myy_text_edit_module_detach(
	struct myy_text_edit_module * __restrict const text_edit_module,
	myy_states * __restrict const states)
{
	/* TODO Error management */
	text_edit_module->flags = myy_text_edit_module_flag_finishing;
	size_t const inserted_string_length =
		myy_vector_utf8_length(&text_edit_module->inserted_data_buffer);
	off_t const insertion_point =
		text_edit_module->edited_buffer_insertion_before;
	off_t const actual_after_insert_index =
		insertion_point
		+ inserted_string_length;
	myy_vector_utf8 * __restrict const edited_buffer =
		text_edit_module->edited_buffer;

	myy_vector_utf8_shift_from(
		edited_buffer,
		text_edit_module->edited_buffer_insertion_after,
		actual_after_insert_index);
	myy_vector_utf8_write_at(
		edited_buffer,
		insertion_point,
		myy_vector_utf8_data(&text_edit_module->inserted_data_buffer),
		inserted_string_length);
	myy_vector_utf8_add(
		edited_buffer,
		1,
		(uint8_t const * __restrict) "\0");

	/* TODO : There's no draw function for the text area... ? */

	text_edit_module->flags = myy_text_edit_module_flag_inactive;
}

void myy_text_edit_module_provoke_redraw(
	struct myy_text_edit_module * __restrict const text_edit_module)
{

}

bool myy_text_edit_module_add_text(
	struct myy_text_edit_module * __restrict const text_edit_module,
	uint8_t const * __restrict const text,
	size_t text_size)
{
	return false;
}
