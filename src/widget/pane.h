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

/*
 * pane function declarations
 */

struct scr_pane_t *scr_pane_new(struct scr_widget_t widget);
void scr_pane_delete(struct scr_pane_t *pane);

struct scr_widget_t scr_pane_widget(struct scr_pane_t *pane);

void scr_pane_render(struct scr_pane_t *pane, struct scr_view_t view, bool focus);
void scr_pane_keypress(struct scr_pane_t *pane, int32_t key, struct scr_context_t context);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
