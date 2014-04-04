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


/*
 * local function declarations
 */

static bool inst_resp(struct inst_t *inst, int32_t key, struct scr_context_t context, struct scr_complete_h complete);
static void inst_delete(struct inst_t *inst);

static bool callback_resp(struct callback_t *callback, int32_t key, struct scr_context_t context, struct scr_complete_h complete);

/*
 * local variables
 */

static struct scr_resp_i inst_iface = { (scr_resp_f)inst_resp, (delete_f)inst_delete };
static struct scr_resp_i callback_iface = { (scr_resp_f)callback_resp, mem_delete };

/*
 * global variables
 */

_export const struct scr_resp_t scr_resp_null = { NULL, NULL };


/**
 * Create a responder with a dynamic interface..
 *   @func: The function.
 *   @arg: The argument.
 *   &returns: The response.
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
 *   &returns: The response.
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


static bool inst_resp(struct inst_t *inst, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	return inst->resp(inst->ref, key, context, complete);
}

static void inst_delete(struct inst_t *inst)
{
	inst->delete(inst->ref);
	mem_free(inst);
}

static bool callback_resp(struct callback_t *callback, int32_t key, struct scr_context_t context, struct scr_complete_h complete)
{
	return callback->func(key, context, complete, callback->arg);
}
