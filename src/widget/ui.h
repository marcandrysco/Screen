#ifndef WIDGET_UI_H
#define WIDGET_UI_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_cmd_h;
struct scr_resp_t;
struct scr_ui_t;


/**
 * Callback for generating a widget on a new pane.
 *   @arg: The argument.
 *   &returns: The widget.
 */

typedef struct scr_widget_t (*scr_ui_f)(void *arg);


/*
 * ui function declarations
 */

struct scr_ui_t *scr_ui_new(scr_ui_f func, void *arg);
void scr_ui_delete(struct scr_ui_t *ui);

void scr_ui_render(struct scr_ui_t *ui, struct scr_view_t view, bool focus);
void scr_ui_keypress(struct scr_ui_t *ui, int32_t key, bool *term);

void scr_ui_reset(struct scr_ui_t *ui);

void scr_ui_raw(struct scr_ui_t *ui, bool raw);
void scr_ui_msg(struct scr_ui_t *ui, struct io_chunk_t msg);
void scr_ui_help(struct scr_ui_t *ui, struct io_chunk_t msg);
void scr_ui_prompt(struct scr_ui_t *ui, struct io_chunk_t msg, struct scr_resp_t resp);
void scr_ui_error(struct scr_ui_t *ui, struct io_chunk_t error);
void scr_ui_clear(struct scr_ui_t *ui);
void scr_ui_status(struct scr_ui_t *ui, struct io_chunk_t status);
const char *scr_ui_input(struct scr_ui_t *ui);

void scr_ui_cmd(struct scr_ui_t *ui, struct scr_cmd_h handler);

void scr_ui_win(struct scr_ui_t *ui, const char *cmd);
void scr_ui_hsplit(struct scr_ui_t *ui);
void scr_ui_vsplit(struct scr_ui_t *ui);

void scr_ui_tab(struct scr_ui_t *ui);
void scr_ui_rtab(struct scr_ui_t *ui);
bool scr_ui_close(struct scr_ui_t *ui);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
