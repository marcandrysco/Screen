#include "../common.h"
#include "pane.h"
#include "../pack.h"
#include "widget.h"


/**
 * Pane widget structure.
 *   @widget: The child widget.
 */

struct scr_pane_t {
	struct scr_widget_t widget;
};


/*
 * local variables
 */

static struct scr_widget_i pane_iface = {
	(scr_render_f)scr_pane_render,
	(scr_keypress_f)scr_pane_keypress,
	(delete_f)scr_pane_delete
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
 *   @pane: THe pane widget.
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
	scr_widget_replace(&pane->widget, widget);
}


/**
 * Split enumerator.
 *   @scr_split_horiz_e: Horizontal split.
 *   @scr_split_vert_e: Vertical split.
 */

enum scr_split_e {
	scr_split_horiz_e,
	scr_split_vert_e
};

/**
 * Split structure.
 *   @type: The split type.
 *   @front, back: The front and back panes.
 *   @size: The size.
 */

struct scr_split_t {
	enum scr_split_e type;
	struct scr_pane_t *front, *back;

	double size;
};


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

	split = mem_alloc(sizeof(struct scr_split_t));
	split->type = type;
	split->size = size;
	split->front = front;
	split->back = back;

	return split;
}

/**
 * Delete a split.
 *   @split: The split.
 */

_export
void scr_split_delete(struct scr_split_t *split)
{
	mem_free(split);
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

		scr_pane_render(split->front, scr_pack_vert(&view, top), focus);
		scr_pane_render(split->back, scr_pack_vert(&view, bottom), focus);
	}
	else if(split->type == scr_split_vert_e) {
		unsigned int left, right;

		left = (view.box.size.width - 1) * split->size;
		right = view.box.size.width - left - 1;

		scr_pane_render(split->front, scr_pack_horiz(&view, left), focus);
		scr_pane_render(split->back, scr_pack_horiz(&view, right), focus);
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
}
