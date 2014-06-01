#ifndef WIDGET_RESP_H
#define WIDGET_RESP_H

/*
 * Start Header Creation: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_context_t;
struct scr_complete_h;


/**
 * Response function.
 *   @ref: The reference.
 *   @key: The key.
 *   @context: The context from the pane.
 *   @complete: Auto-complete handler.
 *   &returns: A flag whether to process the character.
 */

typedef bool (*scr_resp_f)(void *ref, int32_t key, struct scr_context_t context, struct scr_complete_h complete);

/**
 * Stateless response function.
 *   @key: The key.
 *   @context: The context from the pane.
 *   @complete: Auto-complete handler.
 *   @arg: The argument.
 *   &returns: A flag whether to process the character.
 */

typedef bool (*scr_resp_callback_f)(int32_t key, struct scr_context_t context, struct scr_complete_h complete, void *arg);

/**
 * Response interface.
 *   @resp: Response.
 *   @delete: Delete.
 */

struct scr_resp_i {
	scr_resp_f resp;
	delete_f delete;
};

/**
 * Response structure.
 *   @ref: The reference.
 *   @iface: The interface.
 */

struct scr_resp_t {
	void *ref;
	const struct scr_resp_i *iface;
};


/**
 * Create an iterator for matching.
 *   @arg: The argument.
 *   &returns: The iterator.
 */

typedef struct iter_t (*scr_match_iter_f)(void *arg);

/**
 * Process the matched input.
 *   @context: The context.
 *   @arg: The argument.
 */

typedef void (*scr_match_proc_f)(struct scr_context_t context, void *arg);


/**
 * Confirmation handler.
 *   @arg: The argument.
 */

typedef void (*scr_confirm_f)(void *arg);


/*
 * response variables
 */

extern const struct scr_resp_t scr_resp_null;

/*
 * response function declarations
 */

struct scr_resp_t scr_resp_new(void *ref, scr_resp_f resp, delete_f delete);
struct scr_resp_t scr_resp_callback(scr_resp_callback_f func, void *arg);
struct scr_resp_t scr_resp_match(scr_match_iter_f iter, scr_match_proc_f proc, void *arg);
struct scr_resp_t scr_resp_select(scr_enum_f iter, struct iter_filter_h filter, scr_select_f proc, void *arg);
struct scr_resp_t scr_resp_confirm(scr_confirm_f func, void *arg);
bool scr_resp_exec(struct scr_resp_t resp, int32_t key, struct scr_context_t context, struct scr_complete_h complete);
void scr_resp_delete(struct scr_resp_t resp);


/**
 * Determine if a responder. is non-null.
 *   @resp: The repsonder.
 *   &returns: True if non-null, false otherwise.
 */

static inline bool scr_resp_isnull(struct scr_resp_t resp)
{
	return (resp.ref == NULL) && (resp.iface == NULL);
}

/**
 * Replace a responder.
 *   @dest: The destination.
 *   @src: The source.
 */

static inline void scr_resp_replace(struct scr_resp_t *dest, struct scr_resp_t src)
{
	if(!scr_resp_isnull(*dest))
		scr_resp_delete(*dest);

	*dest = src;
}

/* %~scr.h% */

/*
 * End Header Creation: scr.h
 */

#endif
