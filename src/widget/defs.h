#ifndef WIDGET_DEFS_H
#define WIDGET_DEFS_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_context_t;


/**
 * Retrieve the next item from the index iterator.
 *   @arg: The argument.
 *   @key: The key.
 *   &returns: The chunk.
 */

typedef struct io_chunk_t (*scr_iter_f)(void *ref, void **key);

/**
 * Iterator structure.
 *   @next: The next value.
 *   @delete: Delete.
 */

struct scr_iter_i {
	scr_iter_f next;
	delete_f delete;
};

/**
 * Iterator structure.
 *   @ref: The reference.
 *   @iface: The interface.
 */

struct scr_iter_t {
	void *ref;
	const struct scr_iter_i *iface;
};


/**
 * Retrieve the next item from the iterator.
 *   @arg: The argument.
 *   @key: The key.
 *   &returns: The chunk.
 */

static inline struct io_chunk_t scr_iter_next(struct scr_iter_t iter, void **key)
{
	return iter.iface->next(iter.ref, key);
}

/**
 * Delete the iterator.
 *   @iter: The iterator.
 */

static inline void scr_iter_delete(struct scr_iter_t iter)
{
	iter.iface->delete(iter.ref);
}


/**
 * Enumeration function to generate an iterator.
 *   @arg: The argument.
 *   &returns: The iterator.
 */

typedef struct iter_t (*scr_enum_f)(void *arg);

/**
 * Enumeration handler.
 *   @func: The function.
 *   @arg: The argumet.
 */

struct scr_enum_h {
	scr_enum_f func;
	void *arg;
};

/**
 * Execute an enumeration handler.
 *   @handler: The handler.
 *   &returns: The iterator.
 */

static inline struct iter_t scr_enum_exec(struct scr_enum_h handler)
{
	return handler.func(handler.arg);
}


/**
 * Selection function.
 *   @entry: The entry.
 *   @key: The selected key.
 *   @context: The context.
 *   @arg: The argument.
 */

typedef void (*scr_select_f)(const char *entry, void *key, struct scr_context_t context, void *arg);

/**
 * Selection handler.
 *   @func: The function.
 *   @arg: The argument.
 */

struct scr_select_h {
	scr_select_f func;
	void *arg;
};

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
