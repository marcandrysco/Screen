#ifndef WIDGET_INDEX_H
#define WIDGET_INDEX_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_t;
struct scr_context_t;
struct scr_select_h;


/**
 * Indexed list function.
 *   @arg: The argument.
 *   &returns: The iterator.
 */

typedef struct scr_iter_t (*scr_index_f)(void *arg);

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


/*
 * index function declarations
 */

struct scr_index_t *scr_index_new(scr_index_f func, void *arg);
struct scr_index_t *scr_index_arr(const char *const *arr);
void scr_index_delete(struct scr_index_t *index);

struct scr_widget_t scr_index_widget(struct scr_index_t *index);

void scr_index_render(struct scr_index_t *index, struct scr_view_t view, bool focus);
void scr_index_keypress(struct scr_index_t *index, int32_t key, struct scr_context_t context);

void scr_index_keys(struct scr_index_t *index, compare_f compare, copy_f copy, delete_f delete);
void scr_index_empty(struct scr_index_t *index, struct io_chunk_t empty);
void scr_index_select(struct scr_index_t *index, struct scr_select_h handler);

unsigned int scr_index_cur(struct scr_index_t *index, void **key, char **str);
void scr_index_prev(struct scr_index_t *index);
void scr_index_next(struct scr_index_t *index);

void scr_index_search(struct scr_index_t *index, const char *str);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
