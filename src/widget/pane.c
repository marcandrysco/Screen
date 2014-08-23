#include "../common.h"
#include "pane.h"
#include "../pack.h"
#include "../output.h"
#include "widget.h"


/**
 * Pane widget structure.
 *   @widget: The child widget.
 */

struct scr_pane_t {
	struct scr_widget_t widget;
};


/**
 * Split structure.
 *   @type: The split type.
 *   @front, back: The front and back panes.
 *   @size: The size.
 *   @focus: The front focus flag.
 */

struct scr_split_t {
	enum scr_split_e type;
	struct scr_pane_t *front, *back;

	double size;

	bool focus;
};


/*
 * local variables
 */

static struct scr_widget_i pane_iface = {
	(scr_render_f)scr_pane_render,
	(scr_keypress_f)scr_pane_keypress,
	(delete_f)scr_pane_delete
};

static struct scr_widget_i split_iface = {
	(scr_render_f)scr_split_render,
	(scr_keypress_f)scr_split_keypress,
	(delete_f)scr_split_delete
};


/**
 * Create a new pane widget.
 *   @widget; The internal widget.
 *   &returns: The pane.
 */

struct scr_pane_t *scr_pane_new(struct scr_widget_t widget)
{
	struct scr_pane_t *pane;

	pane = mem_alloc(sizeof(struct scr_pane_t));
	pane->widget = widget;

	return pane;
}

/**
 * Delete a pane widget.
 *   @pane: The pane widget.
 */

_export
void scr_pane_delete(struct scr_pane_t *pane)
{
	scr_widget_delete(pane->widget);
	mem_free(pane);
}


/**
 * Retrieve the pane as a widget.
 *   @pane: The pane.
 *   @widget: The widget.
 */

_export
struct scr_widget_t scr_pane_widget(struct scr_pane_t *pane)
{
	return (struct scr_widget_t){ pane, &pane_iface };
}


/**
 * Render a pane.
 *   @pane: The pane.
 *   @view: The target view.
 *   @focus: The focus flag.
 */

void scr_pane_render(struct scr_pane_t *pane, struct scr_view_t view, bool focus)
{
	scr_widget_render(pane->widget, view, focus);
}

/**
 * Handle a keypress on a widget.
 *   @pane: The pane.
 *   @key: The key.
 *   @context: The context.
 */

void scr_pane_keypress(struct scr_pane_t *pane, int32_t key, struct scr_context_t context)
{
	scr_widget_keypress(pane->widget, key, context);
}


/**
 * Retrieve the child widget.
 *   @pane: The pane.
 *   &returns: The child widget.
 */

_export
struct scr_widget_t scr_pane_get(struct scr_pane_t *pane)
{
	return pane->widget;
}

/**
 * Set the pane's child.
 *   @pane: The pane.
 *   @widget: The widget.
 */

_export
void scr_pane_set(struct scr_pane_t *pane, struct scr_widget_t widget)
{
	pane->widget = widget;
}

/**
 * Replace the pane's child, deleting the old widget.
 *   @pane: The pane.
 *   @widget: The widget.
 */

_export
void scr_pane_replace(struct scr_pane_t *pane, struct scr_widget_t widget)
{
	scr_widget_replace(&pane->widget, widget);
}


/**
 * Process a tab on the pane.
 *   @pane: The pane.
 *   &returns: The newly focused pane.
 */

_export
struct scr_pane_t *scr_pane_tab(struct scr_pane_t *pane)
{
	if(pane->widget.iface != &split_iface)
		return NULL;

	return scr_split_tab(pane->widget.ref);
}

/**
 * Process a reverse tab on the pane.
 *   @pane: The pane.
 *   &returns: The newly focused pane.
 */

_export
struct scr_pane_t *scr_pane_rtab(struct scr_pane_t *pane)
{
	if(pane->widget.iface != &split_iface)
		return NULL;

	return scr_split_rtab(pane->widget.ref);
}


_export
struct scr_split_t *scr_pane_split(struct scr_pane_t *pane)
{
	return (pane->widget.iface == &split_iface) ? pane->widget.ref : NULL;
}


/**
 * Create a split.
 *   @type: The type.
 *   @size: The size.
 *   @front: The front pane.
 *   @back: The back pane.
 */

_export
struct scr_split_t *scr_split_new(enum scr_split_e type, float size, struct scr_pane_t *front, struct scr_pane_t *back)
{
	struct scr_split_t *split;

	split = mem_alloc(sizeof(struct scr_split_t) + 300);
	split->type = type;
	split->size = size;
	split->front = front;
	split->back = back;
	split->focus = true;

	return split;
}

/**
 * Delete a split.
 *   @split: The split.
 */

_export
void scr_split_delete(struct scr_split_t *split)
{
	scr_pane_delete(split->front);
	scr_pane_delete(split->back);
	mem_free(split);
}


/**
 * Retrieve the split as a widget.
 *   @split: The split.
 *   @widget: The widget.
 */

_export
struct scr_widget_t scr_split_widget(struct scr_split_t *split)
{
	return (struct scr_widget_t){ split, &split_iface };
}


/**
 * Render a split.
 *   @split: The split
 *   @view: The target view.
 *   @focus: The focus flag.
 */

_export
void scr_split_render(struct scr_split_t *split, struct scr_view_t view, bool focus)
{
	if(split->type == scr_split_horiz_e) {
		unsigned int top, bottom;

		top = (view.box.size.height - 1) * split->size;
		bottom = view.box.size.height - top - 1;

		scr_pane_render(split->front, scr_pack_vert(&view, top), focus && split->focus);
		scr_view_fill_code(scr_pack_vert(&view, 1), '-');
		scr_pane_render(split->back, scr_pack_vert(&view, bottom), focus && !split->focus);
	}
	else if(split->type == scr_split_vert_e) {
		unsigned int left, right;

		left = (view.box.size.width - 1) * split->size;
		right = view.box.size.width - left - 1;

		scr_pane_render(split->front, scr_pack_horiz(&view, left), focus && split->focus);
		scr_view_fill_code(scr_pack_horiz(&view, 1), '|');
		scr_pane_render(split->back, scr_pack_horiz(&view, right), focus && !split->focus);
	}
	else
		_fatal("Invalid split type.");
}

/**
 * Handle a keypress on a split.
 *   @ref: The reference.
 *   @key: The key.
 *   @context: The context.
 */

_export
void scr_split_keypress(struct scr_split_t *split, int32_t key, struct scr_context_t context)
{
	if(split->focus)
		scr_pane_keypress(split->front, key, context);
	else
		scr_pane_keypress(split->back, key, context);
}


/**
 * Process a tab on the split.
 *   @split: The split.
 *   &returns: The newly focused pane.
 */

_export
struct scr_pane_t *scr_split_tab(struct scr_split_t *split)
{
	if(split->focus) {
		struct scr_pane_t *cur;

		cur = scr_pane_tab(split->front);
		if(cur != NULL)
			return cur;

		split->focus = false;

		return split->back;
	}
	else
		return scr_pane_tab(split->back);
}

/**
 * Process a tab on the split.
 *   @split: The split.
 *   &returns: True if the tab was consumed, false otherwise.
 */

_export
struct scr_pane_t *scr_split_rtab(struct scr_split_t *split)
{
	if(!split->focus) {
		struct scr_pane_t *cur;

		cur = scr_pane_rtab(split->back);
		if(cur != NULL)
			return cur;

		split->focus = true;

		return split->front;
	}
	else
		return scr_pane_rtab(split->front);
}


/**
 * Close the split.
 *   @split: The split.
 *   &returns: The newly focused pane or null.
 */

_export
struct scr_pane_t *scr_split_close(struct scr_pane_t **pane, struct scr_split_t *split)
{
	struct scr_split_t *sub;

	if(split->focus) {
		sub = scr_pane_split(split->front);
		if(sub != NULL)
			return scr_split_close(&split->front, sub);

		mem_free(*pane);
		*pane = split->back;

		scr_pane_delete(split->front);
		mem_free(split);
	}
	else {
		sub = scr_pane_split(split->back);
		if(sub != NULL)
			return scr_split_close(&split->back, sub);

		mem_free(*pane);
		*pane = split->front;

		scr_pane_delete(split->back);
		mem_free(split);
	}

	return *pane;
}
