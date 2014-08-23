#include "../common.h"
#include "widget.h"
#include "handler.h"
#include "resp.h"


/*
 * local function declarations
 */

static void blank_close(void *ref);

/*
 * local variables
 */

static const struct scr_widget_i blank_iface = { NULL, NULL, blank_close};

/*
 * global variables
 */

_export struct scr_widget_t scr_widget_null = { NULL, NULL };
_export struct scr_widget_t scr_widget_blank = { NULL, &blank_iface };


/**
 * Close a widget based on its reference.
 *   @widget: The widget reference.
 */

_export
void scr_widget_close(struct scr_widget_t *widget)
{
	if(scr_widget_isnull(*widget))
		return;

	scr_widget_delete(*widget);
	*widget = scr_widget_null;
}


/**
 * Close a blank widget.
 *   @ref: The reference.
 */

static void blank_close(void *ref)
{
}


/**
 * Create a prompt on the context.
 *   @context: The context.
 *   @prompt: The prompt text.
 *   @resp: The respnose handler.
 */

_export
void scr_context_prompt(struct scr_context_t context, struct io_chunk_t prompt, struct scr_resp_t resp)
{
	context.mode->iface->prompt(context.mode->ref, prompt, resp);
}

/**
 * Clear any message, prompt, or error.
 *   @context: The context.
 */

_export
void scr_context_clear(struct scr_context_t context)
{
	context.mode->iface->clear(context.mode->ref);
}

/**
 * Retrieve the prompt input.
 *   @context: The context.
 *   &returns: The input or null.
 */

_export
const char *scr_context_input(struct scr_context_t context)
{
	return context.mode->iface->input(context.mode->ref);
}
