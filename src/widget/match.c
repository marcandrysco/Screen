#include "../common.h"
#include "match.h"
#include "handler.h"
#include "resp.h"
#include "widget.h"


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


/*
 * local function declarations
 */

static bool scr_match_resp(int32_t key, struct scr_context_t context, struct scr_complete_h complete, void *arg);
static void match_help(struct io_output_t output, void *arg);


/**
 * Creating a maching response.
 *   @iter: The iterator generator.
 *   @proc: The processor.
 *   @arg: The argument.
 */

_export
struct scr_resp_t scr_match(scr_match_iter_f iter, scr_match_proc_f proc, void *arg)
{
	struct match_t *match;

	match = mem_alloc(sizeof(struct match_t));
	match->iter = iter;
	match->proc = proc;
	match->arg = arg;

	return scr_resp_callback(scr_match_resp, match);
}

/**
 * Handle a matching response.
 *   @key: The key.
 *   @context: The context.
 *   @complete: The completion handler.
 *   @arg: The argument.
 *   &returns: Always true to process a character.
 */

static bool scr_match_resp(int32_t key, struct scr_context_t context, struct scr_complete_h complete, void *arg)
{
	struct match_t *match = arg;
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
		try {
			match->proc(input, match->arg);
			scr_context_clear(context);
		}
		catch(e)
			scr_context_error(context, io_chunk_str(e));

		mem_free(match);
	}
	else if(key == scr_esc_e)
		mem_free(match);

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
