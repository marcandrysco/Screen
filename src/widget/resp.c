#include "../common.h"
#include "resp.h"
#include "handler.h"
#include "widget.h"


/**
 * Instance structure.
 *   @ref: The reference.
 *   @resp: The response.
 *   @delete: Deletion.
 */

struct inst_t {
	void *ref;
	scr_resp_f resp;
	delete_f delete;
};

/**
 * Callback structure.
 *   @func: Callback function.
 *   @arg: The argument.
 */

struct callback_t {
	scr_resp_callback_f func;
	void *arg;
};

/**
 * Entry structure.
 *   @func: The function.
 *   @arg: The argument.
 */

struct entry_t {
	scr_entry_f func;
	void *arg;
};

/**
 * Selection structure.
 *   @iter: Iterator generator.
 *   @filter: The filter handler.
 *   @proc: Process a select.
 */

struct select_t {
	struct enum_t iter;
	struct io_filter_t filter;
	struct scr_select_h proc;
};

/**
 * Confirmation structure.
 *   @func: The function.
 *   @arg: The argument.
 */

struct confirm_t {
	scr_confirm_f func;
	void *arg;
};


/*
 * local function declarations
 */

static bool inst_resp(struct inst_t *inst, int32_t key, struct scr_context_t context, struct scr_complete_h complete);
static void inst_delete(struct inst_t *inst);

static bool callback_resp(struct callback_t *callback, int32_t key, struct scr_context_t context, struct scr_complete_h complete);

static bool entry_resp(struct entry_t *entry, int32_t key, struct scr_context_t context, struct scr_complete_h complete);

static bool select_resp(struct select_t *select, int32_t key, struct scr_context_t context, struct scr_complete_h complete);
static void select_delete(struct select_t *select);
static void select_help(struct io_output_t output, void *arg);

static bool confirm_resp(struct confirm_t *confirm, int32_t key, struct scr_context_t context, struct scr_complete_h complete);

/*
 * local variables
 */

static struct scr_resp_i inst_iface = { (scr_resp_f)inst_resp, (delete_f)inst_delete };
static struct scr_resp_i callback_iface = { (scr_resp_f)callback_resp, mem_delete };
static struct scr_resp_i confirm_iface = { (scr_resp_f)confirm_resp, mem_delete };

/*
 * global variables
 */

_export const struct scr_resp_t scr_resp_null = { NULL, NULL };


/**
 * Create a responder with a dynamic interface..
 *   @func: The function.
 *   @arg: The argument.
 *   &returns: The responder.
 */

_export
struct scr_resp_t scr_resp_new(void *ref, scr_resp_f resp, delete_f delete)
{
	struct inst_t *inst;

	inst = mem_alloc(sizeof(struct inst_t));
	*inst = (struct inst_t){ ref, resp, delete };

	return (struct scr_resp_t){ inst, &inst_iface };
}

/**
 * Create a responder with a static callback.
 *   @func: The function.
 *   @arg: The argument.
 *   &returns: The responder.
 */

_export
struct scr_resp_t scr_resp_callback(scr_resp_callback_f func, void *arg)
{
	struct callback_t *callback;

	callback = mem_alloc(sizeof(struct callback_t));
	*callback = (struct callback_t){ func, arg };

	return (struct scr_resp_t){ callback, &callback_iface };
}


/**
 * Create a response for an entry callback.
 *   @func: The function.
 *   @arg: The argument.
 */

_export
struct scr_resp_t scr_resp_entry(scr_entry_f func, void *arg)
{
	struct entry_t *entry;
	static const struct scr_resp_i iface = { (scr_resp_f)entry_resp, mem_delete };

	entry = mem_alloc(sizeof(struct entry_t));
	*entry = (struct entry_t){ func, arg };

	return (struct scr_resp_t){ entry, &iface };
}

/**
 * Process an entry response.
 *   @entry: The entry.
 *   @key: The key.
 *   @context: The context.
 *   @complete: The autocomplete structure.
 *   &returns: Whether or not to process the character.
 */

static bool entry_resp(struct entry_t *entry, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	if(key == '\n') {
		try {
			entry->func(scr_context_input(context), entry->arg);

			scr_context_clear(context);
		}
		catch(e)
			scr_context_error(context, io_chunk_str(e));
	}

	return true;
}


/**
 * Create a responder for selecting.
 *   @iter: The iterator enumerator.
 *   @filter: The filter.
 *   @proc: Select response.
 *   &returns: The response.
 */

_export
struct scr_resp_t scr_resp_select(struct enum_t iter, struct io_filter_t filter, struct scr_select_h proc)
{
	struct select_t *select;
	static struct scr_resp_i iface = { (scr_resp_f)select_resp, (delete_f)select_delete };

	select = mem_alloc(sizeof(struct select_t));
	select->iter = iter;
	select->filter = filter;
	select->proc = proc;

	return (struct scr_resp_t){ select, &iface };
}

/**
 * Handle a selection response.
 *   @select: The selection information.
 *   @key: The key.
 *   @context: The context.
 *   @complete: The completion handler.
 *   &returns: Always true to process a character.
 */

static bool select_resp(struct select_t *select, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	const char *input = scr_context_input(context);
	size_t pre = str_len(input);

	if(key == '\t') {
		struct io_iter_t iter;
		char *longest = NULL;
		struct io_chunk_t chunk;

		iter = io_iter_filter(enum_iter(select->iter), select->filter);
		while(!io_chunk_isnull(chunk = io_iter_next(iter))) {
			char item[io_chunk_proc_len(chunk) + 1];

			io_chunk_proc_buf(chunk, item);
			if(!str_isprefix(item, input))
				continue;

			if(longest != NULL)
				str_longest(longest + pre, item + pre);
			else
				longest = str_dup(item);
		}

		io_iter_delete(iter);

		if(longest != NULL) {
			if(str_isequal(input, longest)) {
				char *entry;
				struct avltree_t tree = avltree_empty(compare_str, mem_free);

				iter = io_iter_filter(enum_iter(select->iter), select->filter);
				while(!io_chunk_isnull(chunk = io_iter_next(iter))) {
					char item[io_chunk_proc_len(chunk) + 1];

					io_chunk_proc_buf(chunk, item);
					if(!str_isprefix(item, input))
						continue;

					entry = str_dup(item);
					avltree_insert(&tree, entry, entry);
				}
				
				io_iter_delete(iter);

				scr_context_help(context, (struct io_chunk_t){ select_help, &tree });
				avltree_destroy(&tree);
			}
			else
				scr_complete_exec(complete, longest);

			mem_free(longest);
		}
	}
	else if(key == '\n') {
		char *volatile match = NULL;

		try {
			struct iter_t iter;
			void *key, *sel = NULL;

			iter = enum_iter(select->iter);

			if(*input != '\0') {
				while((key = iter_next(iter)) != NULL) {
					struct io_chunk_t chunk = io_filter_apply(select->filter, key);
					char item[io_chunk_proc_len(chunk) + 1];

					io_chunk_proc_buf(chunk, item);
					if(!str_isequal(item, input))
						continue;

					sel = key;
					match = str_dup(item);

					break;
				}
			}

			iter_delete(iter);

			scr_select_exec(select->proc, sel ? match : input, sel, context);
		}
		catch(e)
			scr_context_error(context, io_chunk_str(e));

		mem_delete(match);
	}

	return true;
}


/**
 * Produce the help text to enumerate all matching selections.
 *   @output: The output.
 *   @arg: The argument.
 */

static void select_help(struct io_output_t output, void *arg)
{
	const char *item;
	struct avltree_iter_t iter;

	iter = avltree_iter(arg);
	while((item = avltree_iter_next(&iter)) != NULL)
		io_printf(output, "%s ", item);
}

/**
 * Delete a selection response.
 *   @select: The selector.
 */

static void select_delete(struct select_t *select)
{
	io_filter_delete(select->filter);
	enum_delete(select->iter);
	mem_free(select);
}


/**
 * Creating a responder for yes/noconfirmation.
 *   @func: The confirmation function.
 *   @arg: The argument.
 *   &returns: The responder.
 */

_export
struct scr_resp_t scr_resp_confirm(scr_confirm_f func, void *arg)
{
	struct confirm_t *confirm;

	confirm = mem_alloc(sizeof(struct confirm_t));
	confirm->func = func;
	confirm->arg = arg;

	return (struct scr_resp_t){ confirm, &confirm_iface };
}

/**
 * Execute the responder.
 *   @resp: The responder.
 *   @key: The key.
 *   @context: The context from the pane.
 *   @complete: Auto-complete handler.
 *   &returns: A flag whether to process the character.
 */

_export
bool scr_resp_exec(struct scr_resp_t resp, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	return resp.iface->resp(resp.ref, key, context, complete);
}

/**
 * Delete a responder.
 *   @resp: The responder.
 */

_export
void scr_resp_delete(struct scr_resp_t resp)
{
	resp.iface->delete(resp.ref);
}


/**
 * Handle a response for a dynamic instance response.
 *   @inst: The instance.
 *   @key: The key.
 *   @context: The context.
 *   @complete: The autocomplete handler.
 *   &returns: Whether or not to process a character.
 */

static bool inst_resp(struct inst_t *inst, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	return inst->resp(inst->ref, key, context, complete);
}

/**
 * Delete a dynamic instance.
 *   @inst: The instance.
 */

static void inst_delete(struct inst_t *inst)
{
	inst->delete(inst->ref);
	mem_free(inst);
}

/**
 * Handle a response for a static callback.
 *   @callback: The callback.
 *   @key: The key.
 *   @context: The context.
 *   @complete: The autocomplete handler.
 *   &returns: Whether or not to process a character.
 */

static bool callback_resp(struct callback_t *callback, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	return callback->func(key, context, complete, callback->arg);
}


/**
 * Handle a keypress on the home widget.
 *   @home: The home widget.
 *   @key: The key.
 *   @context: The context.
 */

static bool confirm_resp(struct confirm_t *confirm, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	switch(key) {
	case 'y':
	case 'Y':
		{
			struct confirm_t copy = *confirm;

			scr_context_clear(context);
			copy.func(copy.arg);
		}
		break;

	case 'n':
	case 'N':
	case scr_esc_e:
		scr_context_clear(context);
		break;

	default:
		break;
	}

	return false;
}
