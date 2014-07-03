#include "../common.h"
#include "select.h"
#include "../output.h"
#include "widget.h"


/**
 * Select structure.
 *   @iter: The iterator generator.
 *   @filter: The filter.
 *   @proc: The processing handler.
 *   @key: The current key.
 *   @index: The current index.
 *   @empty: The empty message.
 */

struct scr_select_t {
	struct enum_t iter;
	struct io_filter_t filter;

	void *key;
	unsigned int index;

	struct io_chunk_t empty;
};


/**
 * Create a new select widget.
 *   @iter: The iterator generator.
 *   @filter: The text filter.
 *   &returns: The select widget.
 */

_export
struct scr_select_t *scr_select_new(struct enum_t iter, struct io_filter_t filter)
{
	struct scr_select_t *select;

	select = mem_alloc(sizeof(struct scr_select_t));
	select->iter = iter;
	select->filter = filter;
	select->index = 0;
	select->key = NULL;
	select->empty = io_chunk_null;

	return select;
}

/**
 * Delete a select widget.
 *   @select: The select widget.
 */

_export
void scr_select_delete(struct scr_select_t *select)
{
	enum_delete(select->iter);
	io_filter_delete(select->filter);
	mem_free(select);
}


/**
 * Render a select widget.
 *   @select: The select widget.
 *   @view: The target view.
 *   @focus: The focus flag.
 */

_export
void scr_select_render(struct scr_select_t *select, struct scr_view_t view, bool focus)
{
	unsigned int i;
	void *key, *sel;
	struct iter_t iter;
	struct llist_t list;
	struct scr_output_t output = scr_output_view(view);

	sel = NULL;
	list = llist_empty(NULL);

	iter = enum_iter(select->iter);
	for(i = 0; (key = iter_next(iter)) != NULL; i++) {
		llist_append(&list, key);

		if(key == select->key)
			sel = key;
		else if((sel == NULL) && (i == select->index))
			sel = key;
	}

	iter_delete(iter);

	if(i > 0) {
		if(sel == NULL)
			sel = llist_back(&list);

		while((key = llist_front_remove(&list)) != NULL)
			scr_printf(&output, "%C%C%C\n", scr_chunk_neg(key == sel), io_filter_apply(select->filter, key), scr_chunk_neg(false));
	}
	else if(!io_chunk_isnull(select->empty))
		scr_printf(&output, "%C\n", select->empty);

	llist_destroy(&list);
}

/**
 * Handle a keypress on the select widget.
 *   @select: The select widget.
 *   @key: The key.
 *   @context: The context.
 */

_export
void scr_select_keypress(struct scr_select_t *select, int32_t key, struct scr_context_t context)
{
	switch(key) {
	case 'k':
	case scr_up_e:
		scr_select_prev(select);
		break;

	case 'j':
	case scr_down_e:
		scr_select_next(select);
		break;
	}
}


/**
 * Set the empty selector message.
 *   @select: The selection widget.
 *   @empty: The empty message or null.
 */

_export
void scr_select_empty(struct scr_select_t *select, struct io_chunk_t empty)
{
	select->empty = empty;
}


/**
 * Retrieve the current selection.
 *   @select: The select widget.
 *   &returns: The selected key or null.
 */

_export
void *scr_select_cur(struct scr_select_t *select)
{
	unsigned int i;
	struct iter_t iter;
	void *key, *byidx = NULL, *bykey = NULL;

	iter = enum_iter(select->iter);
	for(i = 0; (key = iter_next(iter)) != NULL; i++) {
		if(key == select->key)
			bykey = key;

		if(i == select->index)
			byidx = key;
	}

	iter_delete(iter);

	if(bykey != NULL)
		return bykey;
	else if(byidx != NULL)
		return byidx;
	else
		return NULL;
}

/**
 * Move the select widget to the previous entry.
 *   @select: The select widget.
 */

_export
void scr_select_prev(struct scr_select_t *select)
{
	unsigned int i, idxkey = UINT_MAX;
	void *key, *bykey = NULL, *byidx = NULL, *last = NULL;
	struct iter_t iter;

	iter = enum_iter(select->iter);
	for(i = 0; (key = iter_next(iter)) != NULL; i++) {
		if(key == select->key) {
			idxkey = i - 1;
			bykey = last;
		}

		if(i == (select->index - 1))
			byidx = key;

		last = key;
	}

	iter_delete(iter);

	if(bykey != NULL) {
		select->key = bykey;
		select->index = idxkey;
	}
	else if(byidx != NULL) {
		select->key = byidx;
		select->index--;
	}
	else if(select->index > 0) {
		select->key = last;
		select->index = i - 1;
	}
}

/**
 * Move the select widget to the next entry.
 *   @select: The select widget.
 */

_export
void scr_select_next(struct scr_select_t *select)
{
	unsigned int i, idxkey = UINT_MAX;
	void *key, *bykey = NULL, *byidx = NULL, *last = NULL;
	struct iter_t iter;

	iter = enum_iter(select->iter);
	for(i = 0; (key = iter_next(iter)) != NULL; i++) {
		if(key == select->key)
			idxkey = i + 1;
		else if(idxkey == i)
			bykey = key;

		if(i == (select->index + 1))
			byidx = key;

		last = key;
	}

	iter_delete(iter);

	if(bykey != NULL) {
		select->key = bykey;
		select->index = idxkey;
	}
	else if(byidx != NULL) {
		select->key = byidx;
		select->index++;
	}
	else {
		select->key = last;
		select->index = i - 1;
	}
}
