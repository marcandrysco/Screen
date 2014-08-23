#ifndef WIDGET_PANE_H
#define WIDGET_PANE_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_context_t;
struct scr_pane_t;
struct scr_widget_t;


/**
 * Split enumerator.
 *   @scr_split_horiz_e: Horizontal split.
 *   @scr_split_vert_e: Vertical split.
 */

enum scr_split_e {
	scr_split_horiz_e,
	scr_split_vert_e
};


/*
 * pane function declarations
 */

struct scr_pane_t *scr_pane_new(struct scr_widget_t widget);
void scr_pane_delete(struct scr_pane_t *pane);

struct scr_widget_t scr_pane_widget(struct scr_pane_t *pane);

void scr_pane_render(struct scr_pane_t *pane, struct scr_view_t view, bool focus);
void scr_pane_keypress(struct scr_pane_t *pane, int32_t key, struct scr_context_t context);

struct scr_widget_t scr_pane_get(struct scr_pane_t *pane);
void scr_pane_set(struct scr_pane_t *pane, struct scr_widget_t widget);
void scr_pane_replace(struct scr_pane_t *pane, struct scr_widget_t widget);

struct scr_pane_t *scr_pane_tab(struct scr_pane_t *pane);
struct scr_pane_t *scr_pane_rtab(struct scr_pane_t *pane);

struct scr_split_t *scr_pane_split(struct scr_pane_t *pane);

/*
 * split function declarations
 */

struct scr_split_t *scr_split_new(enum scr_split_e type, float size, struct scr_pane_t *front, struct scr_pane_t *back);
void scr_split_delete(struct scr_split_t *split);

struct scr_widget_t scr_split_widget(struct scr_split_t *split);

void scr_split_render(struct scr_split_t *split, struct scr_view_t view, bool focus);
void scr_split_keypress(struct scr_split_t *split, int32_t key, struct scr_context_t context);

struct scr_pane_t *scr_split_tab(struct scr_split_t *split);
struct scr_pane_t *scr_split_rtab(struct scr_split_t *split);
struct scr_pane_t *scr_split_close(struct scr_pane_t **pane, struct scr_split_t *split);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
