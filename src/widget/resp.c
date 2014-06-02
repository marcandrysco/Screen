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
 * Matching structure.
 *   @iter: Iterator.
 *   @proc: Process.
 *   @arg: The argument.
 */

struct match_t {
	scr_match_iter_f iter;
	scr_match_proc_f proc;
	void *arg;
};

/**
 * Selection structure.
 *   @iter: Iterator generator.
 *   @filter: The filter handler.
 *   @proc: Process a select.
 */

struct select_t {
	struct iter_enum_h iter;
	struct iter_filter_h filter;
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

static bool match_resp(struct match_t *match, int32_t key, struct scr_context_t context, struct scr_complete_h complete);
static void match_help(struct io_output_t output, void *arg);

static bool select_resp(struct select_t *select, int32_t key, struct scr_context_t context, struct scr_complete_h complete);
static void select_help(struct io_output_t output, void *arg);

static bool confirm_resp(struct confirm_t *confirm, int32_t key, struct scr_context_t context, struct scr_complete_h complete);

/*
 * local variables
 */

static struct scr_resp_i inst_iface = { (scr_resp_f)inst_resp, (delete_f)inst_delete };
static struct scr_resp_i callback_iface = { (scr_resp_f)callback_resp, mem_delete };
static struct scr_resp_i match_iface = { (scr_resp_f)match_resp, mem_delete };
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
 * Creating a responder for matching.
 *   @iter: The iterator generator.
 *   @proc: The processor.
 *   @arg: The argument.
 *   &returns: The responder.
 */

_export
struct scr_resp_t scr_resp_match(scr_match_iter_f iter, scr_match_proc_f proc, void *arg)
{
	struct match_t *match;

	match = mem_alloc(sizeof(struct match_t));
	match->iter = iter;
	match->proc = proc;
	match->arg = arg;

	return (struct scr_resp_t){ match, &match_iface };
}

/**
 * Create a responder for selecting.
 *   @iter: The iterator enumerator.
 *   @filter: The filter.
 *   @proc: Select response.
 *   &returns: The response.
 */

_export
struct scr_resp_t scr_resp_select(struct iter_enum_h iter, struct iter_filter_h filter, struct scr_select_h proc)
{
	struct select_t *select;
	static struct scr_resp_i iface = { (scr_resp_f)select_resp, mem_delete };

	select = mem_alloc(sizeof(struct select_t));
	select->iter = iter;
	select->filter = filter;
	select->proc = proc;

	return (struct scr_resp_t){ select, &iface };
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
 * Handle a matching response.
 *   @match: The matching information.
 *   @key: The key.
 *   @context: The context.
 *   @complete: The completion handler.
 *   &returns: Always true to process a character.
 */

static bool match_resp(struct match_t *match, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	const char *input = scr_context_input(context);

	if(key == '\t') {
		const char *item;
		struct iter_t iter;
		char *longest = NULL;
		size_t len, pre = str_len(input);

		iter = match->iter(match->arg);

		while((item = iter_next(iter)) != NULL) {
			if(!mem_isequal(input, item, pre) || item[pre] == '\0')
				continue;

			if(longest != NULL) {
				len = pre;
				while(longest[len] == item[len])
					len++;

				if(longest[len] == '\0')
					continue;

				longest = mem_realloc(longest, len + 1);
				longest[len] = '\0';
			}
			else
				longest = str_dup(item);
		}

		iter_delete(iter);

		if(longest != NULL) {
			if(str_isequal(input, longest)) {
				char *entry;
				struct avltree_t tree;

				avltree_init(&tree, compare_str, mem_free);
				iter = match->iter(match->arg);
				while((item = iter_next(iter)) != NULL) {
					if(!mem_isequal(input, item, pre) && item[pre] != '\0')
						continue;

					entry = str_dup(item);
					avltree_insert(&tree, entry, entry);
				}
				
				iter_delete(iter);

				scr_context_help(context, (struct io_chunk_t){ match_help, &tree });
				avltree_destroy(&tree);
			}
			else
				scr_complete_exec(complete, longest);

			mem_free(longest);
		}
	}
	else if(key == '\n') {
		try
			match->proc(context, match->arg);
		catch(e)
			scr_context_error(context, io_chunk_str(e));
	}

	return true;
}

/**
 * Produce the help text to enumerate all matching items.
 *   @output: The output.
 *   @arg: The argument.
 */

static void match_help(struct io_output_t output, void *arg)
{
	const char *item;
	struct avltree_iter_t iter;

	iter = avltree_iter(arg);
	while((item = avltree_iter_next(&iter)) != NULL)
		io_printf(output, "%s ", item);
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
	struct iter_t iter;
	const char *item, *input = scr_context_input(context);
	size_t pre = str_len(input);

	if(key == '\t') {
		char *longest = NULL;

		iter = iter_enum(select->iter);
		if(!iter_filter_isnull(select->filter))
			iter = iter_filter(iter, select->filter);

		while((item = iter_next(iter)) != NULL) {
			if(!str_isprefix(item, input))
				continue;

			if(longest != NULL)
				str_longest(longest + pre, item + pre);
			else
				longest = str_dup(item);
		}

		iter_delete(iter);

		if(longest != NULL) {
			if(str_isequal(input, longest)) {
				char *entry;
				struct avltree_t tree = avltree_empty(compare_str, mem_free);

				iter = iter_enum(select->iter);
				if(!iter_filter_isnull(select->filter))
					iter = iter_filter(iter, select->filter);

				while((item = iter_next(iter)) != NULL) {
					if(!str_isprefix(item, input))
						continue;

					entry = str_dup(item);
					avltree_insert(&tree, entry, entry);
				}
				
				iter_delete(iter);

				scr_context_help(context, (struct io_chunk_t){ select_help, &tree });
				avltree_destroy(&tree);
			}
			else
				scr_complete_exec(complete, longest);

			mem_free(longest);
		}
	}
	else if(key == '\n') {
		void *key, *sel = NULL;
		char *match = NULL;

		iter = iter_enum(select->iter);

		try {
			while((key = iter_next(iter)) != NULL) {
				item = select->filter.func(key, select->filter.arg);
				if(!str_isprefix(item, input))
					continue;

				if(sel != NULL) {
					sel = NULL;
					mem_free(match);

					break;
				}
				else {
					sel = key;
					match = str_dup(item);
				}

			}

			scr_select_exec(select->proc, sel ? match : NULL, sel, context);
		}
		catch(e)
			scr_context_error(context, io_chunk_str(e));

		mem_delete(match);
		iter_delete(iter);
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
