#include "../common.h"
#include "index.h"
#include "../output.h"
#include "../pack.h"
#include "edit.h"
#include "handler.h"
#include "widget.h"


/**
 * Index structure.
 *   @func: The index function.
 *   @arg: The argument.
 *   @compare: Key comparison callback.
 *   @delete: Key copy callback.
 *   @delete: Key delete callback.
 *   @empty: The empty text.
 *   @select: The selection handler.
 *   @sel: The selected index.
 *   @key: The selected key.
 *   @find, search: The find and search string.
 *   @edit: The find edit control.
 */

struct scr_index_t {
	scr_index_f func;
	void *arg;

	compare_f compare;
	copy_f copy;
	delete_f delete;
	struct io_chunk_t empty;
	struct scr_select_h select;

	unsigned int sel;
	void *key;

	char *find, *search;
	struct scr_edit_t edit;
};


/*
 * local function declarations
 */

static struct scr_iter_t arr_index(const char *const *arr);
static struct io_chunk_t arr_iter(const char *const **ptr, void **key);

/*
 * local variables
 */

static const struct scr_iter_i arr_iface = { (scr_iter_f)arr_iter, mem_free };

static const struct scr_widget_i index_iface = {
	(scr_render_f)scr_index_render,
	(scr_keypress_f)scr_index_keypress,
	(delete_f)scr_index_delete
};

static void *def_copy(void *ref)
{
	return ref;
}

static void def_delete(void *ref)
{
}

/**
 * Create an index.
 *   @func: The function.
 *   @select: The selection function.
 *   @arg: The argument.
 *   &returns: The index.
 */

_export
struct scr_index_t *scr_index_new(scr_index_f func, void *arg)
{
	struct scr_iter_t iter;
	struct scr_index_t *index;

	index = mem_alloc(sizeof(struct scr_index_t));
	index->func = func;
	index->arg = arg;
	index->compare = compare_ptr;
	index->copy = def_copy;
	index->delete = def_delete;
	index->empty = io_chunk_null;
	index->select = (struct scr_select_h){ NULL, NULL };
	index->sel = 0;
	index->key = NULL;
	index->find = NULL;
	index->search = NULL;

	iter = func(arg);
	scr_iter_next(iter, &index->key);
	scr_iter_delete(iter);
	index->key = NULL;//index->copy(index->key);

	return index;
}

/**
 * Indexed list function.
 *   @arg: The argument.
 *   &returns: The iterator.
 */

_export
struct scr_index_t *scr_index_arr(const char *const *arr)
{
	return scr_index_new((scr_index_f)arr_index, (void *)arr);
}


/**
 * Delete an index.
 *   @index: The index.
 */

_export
void scr_index_delete(struct scr_index_t *index)
{
	if(index->key != NULL)
		index->delete(index->key);

	mem_delete(index->find);
	mem_delete(index->search);
	mem_free(index);
}


/**
 * Retrieve the index as a widget.
 *   @index: The index.
 *   &returns: The widget.
 */

_export
struct scr_widget_t scr_index_widget(struct scr_index_t *index)
{
	return (struct scr_widget_t){ index, &index_iface };
}


/**
 * Render the index.
 *   @index: The index.
 *   @view: The view.
 *   @focus: The focus flag.
 */

_export
void scr_index_render(struct scr_index_t *index, struct scr_view_t view, bool focus)
{
	unsigned int i;
	void *key;
	struct scr_iter_t iter;
	struct io_chunk_t chunk;
	compare_f compare = index->compare;

	if(index->find == NULL) {
		unsigned int sel;
		struct scr_output_t output = scr_output_view(view);

		sel = index->sel;

		if(index->key != NULL) {
			iter = index->func(index->arg);

			for(i = 0; !io_chunk_isnull(scr_iter_next(iter, &key)); i++) {
				if(compare(key, index->key))
					continue;

				sel = i;
				break;
			}

			scr_iter_delete(iter);
		}

		iter = index->func(index->arg);

		for(i = 0; !io_chunk_isnull(chunk = scr_iter_next(iter, &key)); i++)
			scr_printf(&output, "%C%C%C\n", scr_chunk_neg((sel == i) && focus), chunk, scr_chunk_neg(false));

		if((i == 0) && !io_chunk_isnull(chunk = index->empty))
			scr_printf(&output, "%C", chunk);

		scr_iter_delete(iter);
	}
	else {
		struct scr_output_t output;
		struct scr_pair_t status = scr_pack_status(view);
		
		output = scr_output_view(status.front);
		iter = index->func(index->arg);

		for(i = 0; !io_chunk_isnull(chunk = scr_iter_next(iter, &key)); i++) {
			if(*index->find != '\0') {
				size_t len = str_len(index->find);
				char *sub, *str, entry[str_lprintf("%C", chunk) + 1];

				str_printf(entry, "%C", chunk);

				for(str = entry; (sub = str_str(str, index->find)) != NULL; str = sub + len) {
					scr_output_write(&output, str, sub - str);
					scr_printf(&output, "%C%s%C", scr_chunk_neg(focus), index->find, scr_chunk_neg(false));
				}

				scr_printf(&output, "%s\n", str);
			}
			else
				scr_printf(&output, "%C\n", chunk);
		}

		if((i == 0) && !io_chunk_isnull(chunk = index->empty))
			scr_printf(&output, "%C", chunk);

		scr_iter_delete(iter);

		output = scr_output_view(status.back);
		scr_printf(&output, "/%C\n", focus ? scr_edit_chunk(&index->edit) : io_chunk_str(index->find));
	}
}

/**
 * Handle a keypress on the index.
 *   @index: The index.
 *   @key: The key.
 *   @context: The context.
 */

_export
void scr_index_keypress(struct scr_index_t *index, int32_t key, struct scr_context_t context)
{
	if(index->find == NULL) {
		switch(key) {
		case ' ':
		case '\n':
			{
				char *str;
				void *key;

				scr_index_cur(index, &key, &str);
				scr_select_exec(index->select, str, key, context);

				if(str != NULL)
					mem_free(str);

				index->delete(key);
			}
			break;

		case 'k':
		case scr_up_e:
			scr_index_prev(index);
			break;

		case 'j':
		case scr_down_e:
			scr_index_next(index);
			break;

		case '/':
			scr_edit_init(&index->edit, &index->find);
			break;
		}
	}
	else {
		if((key == scr_backspace_e) && (*index->find == '\0'))
			scr_edit_destroy(&index->edit);
		else if(key == '\n') {
			mem_delete(index->search);

			index->search = index->find;
			index->find = NULL;
		}
		else
			scr_edit_keypress(&index->edit, key, context);
	}
}


/**
 * Set the key callback functions.
 *   @index: The index.
 *   @compare: The comparison callback.
 *   @copy: Key copying.
 *   @delete: Key deletion.
 */

_export
void scr_index_keys(struct scr_index_t *index, compare_f compare, copy_f copy, delete_f delete)
{
	index->compare = compare ?: compare_ptr;
	index->copy = copy ?: def_copy;
	index->delete = delete ?: def_delete;
}

/**
 * Set the empty text of the index.
 *   @index: The index.
 *   @empty: The empty chunk.
 */

_export
void scr_index_empty(struct scr_index_t *index, struct io_chunk_t empty)
{
	index->empty = empty;
}

/**
 * Add a select handler to the index.
 *   @index: The index.
 *   @handler: The handler.
 */

_export
void scr_index_select(struct scr_index_t *index, struct scr_select_h handler)
{
	index->select = handler;
}


/**
 * Retrieve the current index.
 *   @index: The index.
 *   @key: Optional. The key.
 *   @str: Optional. An allocated copy of the entry.
 *   &returns: The selected index.
 */

_export
unsigned int scr_index_cur(struct scr_index_t *index, void **key, char **str)
{
	unsigned int i;
	void *bykey, *byidx = NULL;
	struct scr_iter_t iter;
	struct io_chunk_t chunk, sel = io_chunk_null;
	compare_f compare = index->compare;

	iter = index->func(index->arg);

	for(i = 0; !io_chunk_isnull(chunk = scr_iter_next(iter, &bykey)); i++) {
		if((index->key != NULL) && !compare(index->key, bykey))
			break;
		else if(index->sel == i) {
			sel = chunk;
			byidx = bykey;
		}
	}

	if(io_chunk_isnull(chunk)) {
		chunk = sel;
		i = io_chunk_isnull(chunk) ?  UINT_MAX : index->sel;
		bykey = byidx;
	}

	if(key)
		*key = bykey ? index->copy(bykey) : NULL;

	if(str)
		*str = io_chunk_isnull(chunk) ? NULL : io_chunk_proc_str(chunk);

	scr_iter_delete(iter);

	return i;
}

static void index_update(struct scr_index_t *index, unsigned int sel, void *key)
{
	if(index->key != NULL)
		index->delete(index->key);

	index->sel = sel;
	index->key = key ? index->copy(key) : NULL;
}

/**
 * Select the previous elemect from the index.
 *   @index: The index.
 */

_export
void scr_index_prev(struct scr_index_t *index)
{
	unsigned int sel = 0;
	struct scr_iter_t iter;
	struct io_chunk_t chunk;
	void *key = NULL, *bykey = NULL, *byidx = NULL;
	compare_f compare = index->compare;

	iter = index->func(index->arg);

	while(!io_chunk_isnull(chunk = scr_iter_next(iter, &key))) {
		if((index->key != NULL) && (bykey != NULL) && !compare(index->key, key))
			break;
		else if(++sel == index->sel)
			byidx = bykey;

		bykey = key;
	}

	if(!io_chunk_isnull(chunk) && (sel > 0))
		index_update(index, sel, bykey);
	else if(index->sel > 0)
		index_update(index, index->sel - 1, byidx);

	scr_iter_delete(iter);
}

/**
 * Select the next elemect from the index.
 *   @index: The index.
 */

_export
void scr_index_next(struct scr_index_t *index)
{
	unsigned int sel = 0;
	struct scr_iter_t iter;
	struct io_chunk_t chunk;
	void *key = NULL, *bykey = NULL, *byidx = NULL;
	compare_f compare = index->compare;

	iter = index->func(index->arg);

	while(!io_chunk_isnull(chunk = scr_iter_next(iter, &key))) {
		if((index->key != NULL) && (bykey != NULL) && !compare(index->key, bykey))
			break;
		else if(sel++ == (index->sel + 1))
			byidx = key;

		bykey = key;
	}

	if(!io_chunk_isnull(chunk))
		index_update(index, sel, key);
	else if((index->sel + 1) < sel)
		index_update(index, index->sel + 1, byidx);

	scr_iter_delete(iter);
}

/**
 * Create an iterator for the array index.
 *   @arr: The array.
 *   &returns: The iterator.
 */

static struct scr_iter_t arr_index(const char *const *arr)
{
	const char *const **ptr;

	ptr = mem_alloc(sizeof(void *));
	*ptr = arr;
	
	return (struct scr_iter_t){ (void *)ptr, &arr_iface };
}

/**
 * Continue over the array index iterator.
 *   @ptr: The array pointer.
 *   @key: The current key.
 *   &returns: The current chunk or null.
 */

static struct io_chunk_t arr_iter(const char *const **ptr, void **key)
{
	*key = (void *)**ptr;

	return **ptr ? io_chunk_str(*((*ptr)++)) : io_chunk_null;
}


void scr_index_search(struct scr_index_t *index, const char *str)
{
	struct scr_iter_t iter;

	iter = index->func(index->arg);
	scr_iter_delete(iter);
}
