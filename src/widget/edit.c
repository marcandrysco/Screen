#include "../common.h"
#include "edit.h"
#include "../output.h"
#include "widget.h"


/**
 * Initialize an edit widget.
 *   @ref: Optional. The buffer reference.
 *   @edit: The edit widget.
 */

_export
void scr_edit_init(struct scr_edit_t *edit, char **ref)
{
	edit->idx = 0;
	edit->len = 0;
	edit->buf = str_dup("");
	edit->ref = ref;

	if(edit->ref)
		*edit->ref = edit->buf;
}

/**
 * Create an edit structure.
 *   @ref: Optional. The buffer reference.
 *   &returns: The edit widget.
 */

_export
struct scr_edit_t *scr_edit_new(char **ref)
{
	struct scr_edit_t *edit;

	edit = mem_alloc(sizeof(struct scr_edit_t));
	scr_edit_init(edit, ref);
	
	return edit;
}

/**
 * Destroy an edit widget.
 *   @edit: The edit widget.
 */

_export
void scr_edit_destroy(struct scr_edit_t *edit)
{
	if(edit->ref)
		*edit->ref = NULL;

	mem_free(edit->buf);
}

/**
 * Delete an edit widget.
 *   @edit: The edit widget.
 */

_export
void scr_edit_delete(struct scr_edit_t *edit)
{
	mem_free(edit->buf);
	mem_free(edit);
}


/**
 * Render an edit widget.
 *   @edit: The edit wiget.
 *   @view: The view.
 *   @focus: The focus flag.
 */

_export
void scr_edit_render(struct scr_edit_t *edit, struct scr_view_t view, bool focus)
{
	struct scr_output_t output = scr_output_view(view);

	scr_edit_print(edit, &output, focus);
}

/**
 * Print an edit widget.
 *   @edit: The edit wiget.
 *   @output: The output.
 *   @focus: The focus flag.
 */

_export
void scr_edit_print(struct scr_edit_t *edit, struct scr_output_t *output, bool focus)
{
	if(!focus)
		scr_printf(output, "%s", edit->buf);
	else
		scr_printf(output, "%C", scr_edit_chunk(edit));
}

/**
 * Handle a keypress on the edit widget.
 *   @edit: The edit widget.
 *   @key: The key.
 *   @context: The context.
 */

_export
void scr_edit_keypress(struct scr_edit_t *edit, int32_t key, struct scr_context_t context)
{
	switch(key) {
	case scr_left_e:
		if(edit->idx > 0)
			edit->idx--;

		break;

	case scr_right_e:
		if(edit->idx < edit->len)
			edit->idx++;

		break;

	case scr_backspace_e:
		if(edit->idx == 0)
			return;

		mem_move(edit->buf + edit->idx - 1, edit->buf + edit->idx, edit->len - edit->idx + 1);
		edit->buf = mem_realloc(edit->buf, edit->len);
		edit->len--;
		edit->idx--;

		break;

	case scr_delete_e:
		if(edit->idx == edit->len)
			break;

		mem_move(edit->buf + edit->idx, edit->buf + edit->idx + 1, edit->len - edit->idx);
		edit->buf = mem_realloc(edit->buf, edit->len);
		edit->len--;

		break;

	default:
		if(key > 0xFF || key < 0x20)
			return;

		edit->buf = mem_realloc(edit->buf, edit->len + 2);
		mem_move(edit->buf + edit->idx + 1, edit->buf + edit->idx, edit->len - edit->idx + 1);
		edit->buf[edit->idx] = key;
		edit->len++;
		edit->idx++;

		break;
	}

	if(edit->ref)
		*edit->ref = edit->buf;
}


/**
 * Replace text of the edit
 *   @edit: The edit.
 *   @str: The string.
 */

_export
void scr_edit_replace(struct scr_edit_t *edit, const char *str)
{
	str_replace(&edit->buf, str);
	edit->len = edit->idx = str_len(edit->buf);
}

/**
 * Append text to the end of the edit.
 *   @edit: The edit.
 *   @str: The string.
 */

_export
void scr_edit_append(struct scr_edit_t *edit, const char *str)
{
	str_cat(&edit->buf, str);
	edit->len = edit->idx = str_len(edit->buf);
}


static void chunk_proc(struct io_output_t output, const struct scr_edit_t *edit)
{
	io_output_write(output, edit->buf, edit->idx);

	io_chunk_proc(scr_chunk_neg(true), output);
	io_output_ch(output, (edit->idx < edit->len) ? edit->buf[edit->idx] : ' ');
	io_chunk_proc(scr_chunk_neg(false), output);

	if(edit->idx < edit->len)
		io_output_write(output, edit->buf + edit->idx + 1, edit->len - edit->idx - 1);
}

/**
 * Create an I/O chunk for displaying the edit with focus.
 *   @edit: The edit widget.
 *   &returns: The chunk.
 */

_export
struct io_chunk_t scr_edit_chunk(const struct scr_edit_t *edit)
{
	return (struct io_chunk_t){ (io_chunk_f)chunk_proc, (void *)edit };
}
