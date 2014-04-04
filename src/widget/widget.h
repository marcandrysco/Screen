#ifndef WIDGET_H
#define WIDGET_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_t;
struct scr_context_t;
struct scr_resp_t;


/**
 * Modify the raw mode.
 *   @ref: The reference.
 *   @val: The new mode value.
 */

typedef void (*scr_raw_f)(void *ref, bool val);

/**
 * Provide an message.
 *   @ref: The reference.
 *   @msg: The message chunk.
 */

typedef void (*scr_msg_f)(void *ref, struct io_chunk_t msg);

/**
 * Provide a help message.
 *   @ref: The reference.
 *   @msg: The message.
 */

typedef void (*scr_help_f)(void *ref, struct io_chunk_t msg);

/**
 * Provide a prompt.
 *   @ref: The reference.
 *   @prompt: The prompt chunk.
 *   @resp: The response handler.
 */

typedef void (*scr_prompt_f)(void *ref, struct io_chunk_t prompt, struct scr_resp_t resp);

/**
 * Provide an error.
 *   @ref: The reference.
 *   @error: The error chunk.
 */

typedef void (*scr_error_f)(void *ref, struct io_chunk_t error);

/**
 * Clear any message or error, but not the status.
 *   @ref: The reference.
 */

typedef void (*scr_clear_f)(void *ref);

/**
 * Provide an error.
 *   @ref: The reference.
 *   @error: The error chunk.
 */

typedef void (*scr_status_f)(void *ref, struct io_chunk_t status);

/**
 * Retrieve the input text.
 *   @ref: The reference.
 *   &returns: The current input text.
 */

typedef const char *(*scr_input_f)(void *ref);

/**
 * Mode interface.
 *   @raw: Modify the raw mode.
 *   @msg: Display a message.
 *   @help: Display a help message
 *   @prompt: Provide a prompt.
 *   @error: Display an error.
 *   @clear: Clear any message or error.
 *   @status: Display a status.
 *   @input: Retrieve prompt input.
 */

struct scr_mode_i {
	scr_raw_f raw;
	scr_msg_f msg;
	scr_help_f help;
	scr_prompt_f prompt;
	scr_error_f error;
	scr_clear_f clear;
	scr_status_f status;
	scr_input_f input;
};

/**
 * Mode structure.
 *   @ref: The refrence.
 *   @iface: The interface.
 */

struct scr_mode_t {
	void *ref;
	const struct scr_mode_i *iface;
};


/**
 * Close context.
 *   @arg: The argument.
 */

typedef void (*scr_close_f)(void *arg);

/**
 * Context structure.
 *   @mode: The mode.
 *   @close: The close callback.
 *   @arg: The argument.
 */

struct scr_context_t {
	struct scr_mode_t *mode;

	scr_close_f close;
	void *arg;
};

static inline void scr_context_close(struct scr_context_t context)
{
	context.close(context.arg);
}

static inline void scr_context_msg(struct scr_context_t context, struct io_chunk_t msg)
{
	context.mode->iface->msg(context.mode->ref, msg);
}

static inline void scr_context_help(struct scr_context_t context, struct io_chunk_t help)
{
	context.mode->iface->help(context.mode->ref, help);
}

static inline void scr_context_error(struct scr_context_t context, struct io_chunk_t error)
{
	context.mode->iface->error(context.mode->ref, error);
}


/**
 * Render a widget.
 *   @ref: The reference.
 *   @view: The target view.
 *   @focus: The focus flag.
 */

typedef void (*scr_render_f)(void *ref, struct scr_view_t view, bool focus);

/**
 * Handle a keypress on a widget.
 *   @ref: The reference.
 *   @key: The key.
 *   @context: The context.
 */

typedef void (*scr_keypress_f)(void *ref, int32_t key, struct scr_context_t context);

/**
 * Widget interface.
 *   @render: Render.
 *   @keypres: Key press.
 *   @delete: Delete.
 */

struct scr_widget_i {
	scr_render_f render;
	scr_keypress_f keypress;
	delete_f delete;
};

/**
 * Widget structure.
 *   @ref: The reference.
 *   @iface: The interface.
 */

struct scr_widget_t {
	void *ref;
	const struct scr_widget_i *iface;
};


/*
 * widget variables
 */

extern struct scr_widget_t scr_widget_null;

/*
 * widget function declarations
 */

void scr_widget_close(struct scr_widget_t *widget);

/*
 * context function declarations
 */

void scr_context_prompt(struct scr_context_t context, struct io_chunk_t prompt, struct scr_resp_t resp);
void scr_context_clear(struct scr_context_t context);
const char *scr_context_input(struct scr_context_t context);


/**
 * Render a widget.
 *   @widget: The widget.
 *   @view: The target view.
 *   @focus: The focus flag.
 */

static inline void scr_widget_render(struct scr_widget_t widget, struct scr_view_t view, bool focus)
{
	widget.iface->render(widget.ref, view, focus);
}

/**
 * Handle a keypress on a widget.
 *   @widget: The widget.
 *   @key: The key.
 *   @context: The context.
 */

static inline void scr_widget_keypress(struct scr_widget_t widget, int32_t key, struct scr_context_t context)
{
	widget.iface->keypress(widget.ref, key, context);
}

/**
 * Delete a widget.
 *   @widget: The widget.
 */

static inline void scr_widget_delete(struct scr_widget_t widget)
{
	widget.iface->delete(widget.ref);
}

/**
 * Delete a widget if is non-null.
 *   @widget: The widget.
 */

static inline void scr_widget_erase(struct scr_widget_t widget)
{
	if((widget.ref != NULL) || (widget.iface != NULL))
		widget.iface->delete(widget.ref);
}


/**
 * Determine if a widget is null.
 *   @widget: The widget.
 *   &returns: True if null, false otherwise.
 */

static inline bool scr_widget_isnull(struct scr_widget_t widget)
{
	return (widget.ref == NULL) && (widget.iface == NULL);
}

/**
 * Replace a widget, deleting the old widget.
 *   @dest: The destination widget reference. Non-null widget will be deleted.
 *   @src: The source widget, may be null.
 */

static inline void scr_widget_replace(struct scr_widget_t *dest, struct scr_widget_t src)
{
	if(!scr_widget_isnull(*dest))
		scr_widget_delete(*dest);

	*dest = src;
}


/**
 * Create a context that clears the widget reference.
 *   @widget: The widget reference.
 *   @mode: The mode.
 *   &returns: Context.
 */

static inline struct scr_context_t scr_widget_context(struct scr_widget_t *widget, struct scr_mode_t *mode)
{
	return (struct scr_context_t){ mode, (scr_close_f)scr_widget_close, widget };
}

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
