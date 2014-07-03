#ifndef WIDGET_SELECT_H
#define WIDGET_SELECT_H

/*
 * Start Header Creation: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_select_t;

/*
 * select widget function declarations
 */

struct scr_select_t *scr_select_new(struct enum_t iter, struct io_filter_t filter);
void scr_select_delete(struct scr_select_t *select);

void scr_select_render(struct scr_select_t *select, struct scr_view_t view, bool focus);
void scr_select_keypress(struct scr_select_t *select, int32_t key, struct scr_context_t context);

void scr_select_empty(struct scr_select_t *select, struct io_chunk_t empty);

void *scr_select_cur(struct scr_select_t *select);
void scr_select_prev(struct scr_select_t *select);
void scr_select_next(struct scr_select_t *select);

/* %~scr.h% */

/*
 * End Header Creation: scr.h
 */

#endif
