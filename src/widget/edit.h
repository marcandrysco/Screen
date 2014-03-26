#ifndef WIDGET_EDIT_H
#define WIDGET_EDIT_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_context_t;
struct scr_edit_t;
struct scr_output_t;


/**
 * Edit widget structure.
 *   @buf, ref: The buffer and buffer reference.
 *   @idx, len: The cursor index and buffer length.
 */

struct scr_edit_t {
	char *buf, **ref;
	size_t idx, len;
};


/*
 * edit function declarations
 */

void scr_edit_init(struct scr_edit_t *edit, char **ref);
struct scr_edit_t *scr_edit_new(char **ref);
void scr_edit_destroy(struct scr_edit_t *edit);
void scr_edit_delete(struct scr_edit_t *edit);

void scr_edit_render(struct scr_edit_t *edit, struct scr_view_t view, bool focus);
void scr_edit_print(struct scr_edit_t *edit, struct scr_output_t *output, bool focus);
void scr_edit_keypress(struct scr_edit_t *edit, int32_t key, struct scr_context_t context);

void scr_edit_replace(struct scr_edit_t *edit, const char *str);
void scr_edit_append(struct scr_edit_t *edit, const char *str);

struct io_chunk_t scr_edit_chunk(const struct scr_edit_t *edit);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
