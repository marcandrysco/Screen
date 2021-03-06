#include "../common.h"
#include "ui.h"
#include "../accum.h"
#include "../buf.h"
#include "../output.h"
#include "../pack.h"
#include "edit.h"
#include "handler.h"
#include "pane.h"
#include "resp.h"
#include "widget.h"


/**
 * UI widget structure.
 *   @pane, cur: The child and current pane.
 *   @raw: The raw flags.
 *   @msg, help: Message and help chunk.
 *   @prompt: The prompt edit widget.
 *   @cmd: The command handler.
 *   @resp: The response handler.
 *   @delay, expire: The message delay and expiry.
 *   @func: The UI function.
 *   @arg: The argument.
 */

struct scr_ui_t {
	struct scr_pane_t *pane, *cur;

	char *buf;
	bool raw;
	struct scr_buf_t *msg, *help;
	struct scr_edit_t prompt;

	struct scr_cmd_h cmd;
	struct scr_resp_t resp;

	uint64_t delay, expire;

	scr_ui_f func;
	void *arg;
};


/*
 * local function declarations
 */

static void error_proc(struct io_output_t output, const struct io_chunk_t *error);

static bool cmd_resp(int32_t key, struct scr_context_t context, struct scr_complete_h complete, void *arg);

/*
 * local variables
 */

static const struct scr_mode_i mode_iface = {
	(scr_raw_f)scr_ui_raw,
	(scr_msg_f)scr_ui_msg,
	(scr_help_f)scr_ui_help,
	(scr_prompt_f)scr_ui_prompt,
	(scr_error_f)scr_ui_error,
	(scr_clear_f)scr_ui_clear,
	(scr_status_f)scr_ui_status,
	(scr_input_f)scr_ui_input
};


/**
 * Create a new UI widget.
 *   @func: The blank pane function.
 *   @arg: The argument.
 *   &returns: The UI widget.
 */

_export
struct scr_ui_t *scr_ui_new(scr_ui_f func, void *arg)
{
	struct scr_ui_t *ui;

	ui = mem_alloc(sizeof(struct scr_ui_t));
	ui->raw = false;
	ui->msg = NULL;
	ui->help = NULL;
	ui->func = func;
	ui->arg = arg;
	ui->resp = scr_resp_null;
	ui->cmd = (struct scr_cmd_h){ NULL, NULL };
	ui->pane = ui->cur = scr_pane_new(func(arg));

	return ui;
}

/**
 * Delete a UI widget
 *   @ui: The UI widget.
 */

_export
void scr_ui_delete(struct scr_ui_t *ui)
{
	if(!scr_resp_isnull(ui->resp))
		scr_resp_delete(ui->resp);

	if(ui->msg != NULL)
		scr_buf_delete(ui->msg);

	if(ui->help != NULL)
		scr_buf_delete(ui->help);

	if(!scr_resp_isnull(ui->resp))
		scr_resp_delete(ui->resp);

	scr_pane_delete(ui->pane);
	mem_free(ui);
}


/**
 * Render a UI widget.
 *   @ui: The UI widget.
 *   @view: The target view.
 *   @focus: The focus flag.
 */

_export
void scr_ui_render(struct scr_ui_t *ui, struct scr_view_t view, bool focus)
{
	struct scr_pair_t pair;

	pair = scr_pack_status(view);
	scr_pane_render(ui->pane, pair.front, focus && scr_resp_isnull(ui->resp));

	if(ui->msg != NULL) {
		scr_draw_view(scr_pack_horiz(&pair.back, ui->msg->box.size.width), ui->msg);

		if(!scr_resp_isnull(ui->resp))
			scr_edit_render(&ui->prompt, pair.back, true);
	}

	if(ui->help != NULL)
		scr_draw_view(scr_pack_bottom(pair.front, ui->help->box.size.height).back, ui->help);
}

static void ui_term(void *arg)
{
	if(arg != NULL)
		*(bool *)arg = true;
}

static void ui_complete(const char *str, void *arg)
{
	struct scr_ui_t *ui = arg;

	scr_edit_replace(&ui->prompt, str);
	ui->buf = ui->prompt.buf;
}

/**
 * Handle a keypress on a UI widget.
 *   @ui: The UI widget.
 *   @key: The key.
 *   @term: Termination flag.
 */

_export
void scr_ui_keypress(struct scr_ui_t *ui, int32_t key, bool *term)
{
	struct scr_mode_t mode;
	struct scr_context_t context;

	mode = (struct scr_mode_t){ ui, &mode_iface };
	context.mode = &mode;
	context.close = ui_term;
	context.arg = term;

	if(!scr_resp_isnull(ui->resp)) {
		if(scr_resp_exec(ui->resp, key, context, (struct scr_complete_h){ ui_complete, ui })) {
			if(!scr_resp_isnull(ui->resp)) {
				if(ui->prompt.len == 0 && key == scr_backspace_e)
					scr_ui_clear(ui);
				else
					scr_edit_keypress(&ui->prompt, key, context);
			}
		}

		if(!scr_resp_isnull(ui->resp) && (key == scr_esc_e)) {
			scr_edit_destroy(&ui->prompt);
			scr_resp_replace(&ui->resp, scr_resp_null);
			scr_buf_replace(&ui->msg, NULL);
			scr_buf_replace(&ui->help, NULL);
		}
	}
	else if(key == ':')
		scr_ui_prompt(ui, io_chunk_str(":"), scr_resp_callback(cmd_resp, ui));
	else if(key == '\t')
		scr_ui_tab(ui);
	else if(key == scr_rtab_e)
		scr_ui_rtab(ui);
	else {
		if(ui->msg != NULL) {
			scr_buf_delete(ui->msg);
			ui->msg = NULL;
		}

		scr_pane_keypress(ui->pane, key, context);
	}
}


/**
 * Reset the UI widget.
 *   @ui: The UI widget.
 */

_export
void scr_ui_reset(struct scr_ui_t *ui)
{
	scr_pane_delete(ui->pane);

	ui->pane = ui->cur = scr_pane_new(ui->func(ui->arg));
}


/**
 * Set the UI widget raw flag.
 *   @ui: The UI widget.
 *   @raw: The raw flag.
 */

_export
void scr_ui_raw(struct scr_ui_t *ui, bool raw)
{
	ui->raw = raw;
}

/**
 * Set the message.
 *   @ui: The UI widget.
 *   @msg: The message chunk.
 */

_export
void scr_ui_msg(struct scr_ui_t *ui, struct io_chunk_t msg)
{
	struct scr_accum_t *accum;
	struct scr_output_t output;

	if(!scr_resp_isnull(ui->resp)) {
		scr_resp_replace(&ui->resp, scr_resp_null);
		scr_edit_destroy(&ui->prompt);
	}

	accum = scr_accum_new();

	output = scr_output_accum(accum);
	scr_printf(&output, "%C", msg);

	scr_buf_replace(&ui->help, NULL);
	scr_buf_replace(&ui->msg, scr_accum_buf(accum));
	scr_accum_delete(accum);
}

/**
 * Set a help message.
 *   @ui: The UI widget.
 *   @msg: The message chunk.
 */

_export
void scr_ui_help(struct scr_ui_t *ui, struct io_chunk_t msg)
{
	struct scr_accum_t *accum;
	struct scr_output_t output;

	accum = scr_accum_new();

	output = scr_output_accum(accum);
	scr_printf(&output, "%C", msg);

	scr_buf_replace(&ui->help, scr_accum_buf(accum));
	scr_accum_delete(accum);
}

/**
 * Prompt the UI widget for input.
 *   @ui: The UI.
 *   @msg: The prompt message.
 *   @resp: The response handler.
 */

_export
void scr_ui_prompt(struct scr_ui_t *ui, struct io_chunk_t msg, struct scr_resp_t resp)
{
	struct scr_accum_t *accum;
	struct scr_output_t output;

	if(!scr_resp_isnull(ui->resp))
		scr_edit_destroy(&ui->prompt);

	accum = scr_accum_new();

	output = scr_output_accum(accum);
	scr_printf(&output, "%C", msg);

	scr_buf_replace(&ui->help, NULL);
	scr_buf_replace(&ui->msg, scr_accum_buf(accum));
	scr_accum_delete(accum);

	ui->buf = NULL;
	scr_resp_replace(&ui->resp, resp);
	scr_edit_init(&ui->prompt, &ui->buf);
}

/**
 * Provide an error to the UI widget.
 *   @ui: The UI widget.
 *   @error: The error chunk.
 */

_export
void scr_ui_error(struct scr_ui_t *ui, struct io_chunk_t error)
{
	scr_ui_msg(ui, (struct io_chunk_t){ (io_chunk_f)error_proc, &error });
}

/**
 * Callback for formatting an error message.
 *   @output: Theoutput.
 *   @error: The error chunk.
 */

static void error_proc(struct io_output_t output, const struct io_chunk_t *error)
{
	io_printf(output, "%C%C%C", scr_chunk_error(true), *error, scr_chunk_error(false));
}

/**
 * Clear any message or error on the UI widget.
 *   @ui: The UI widget.
 */

_export
void scr_ui_clear(struct scr_ui_t *ui)
{
	if(!scr_resp_isnull(ui->resp)) {
		scr_edit_destroy(&ui->prompt);
		scr_resp_replace(&ui->resp, scr_resp_null);
	}

	scr_buf_replace(&ui->msg, NULL);
	scr_buf_replace(&ui->help, NULL);
}

/**
 * Provide a status to the UI widget.
 *   @ui: The UI widget.
 *   @status: The status chunk.
 */

_export
void scr_ui_status(struct scr_ui_t *ui, struct io_chunk_t status)
{
	//mem_delete(ui->status);
	//ui->status = io_chunk_proc_str(status);
}

/**
 * Retrieve the prompt input.
 *   @ui: The UI.
 *   &returns: The input or null.
 */

_export
const char *scr_ui_input(struct scr_ui_t *ui)
{
	return !scr_resp_isnull(ui->resp) ? ui->buf : NULL;
}


/**
 * Set the command handler.
 *   @ui: The UI widget.
 *   @handler: The handler.
 */

_export
void scr_ui_cmd(struct scr_ui_t *ui, struct scr_cmd_h handler)
{
	ui->cmd = handler;
}


/**
 * Process a windowing command.
 *   @ui: The UI widget.
 *   @cmd: The command.
 */

_export
void scr_ui_win(struct scr_ui_t *ui, const char *cmd)
{
	if(str_isequal(cmd, "s") || str_isequal(cmd, "split"))
		scr_ui_hsplit(ui);
	else if(str_isequal(cmd, "v") || str_isequal(cmd, "vs") || str_isequal(cmd, "vsplit"))
		scr_ui_vsplit(ui);
	else if(cmd[0] == 'w') {
	}
	else if(cmd[0] == 'h') {
	}
}

/**
 * Horizontal split the UI.
 *   @ui: The UI.
 */

_export
void scr_ui_hsplit(struct scr_ui_t *ui)
{
	struct scr_pane_t *front, *back;

	front = scr_pane_new(scr_pane_get(ui->cur));
	back = scr_pane_new(ui->func(ui->arg));
	scr_pane_set(ui->cur, scr_split_widget(scr_split_new(scr_split_horiz_e, 0.5, front, back)));
	ui->cur = front;
}

/**
 * Vertical split the UI.
 *   @ui: The UI.
 */

_export
void scr_ui_vsplit(struct scr_ui_t *ui)
{
	struct scr_pane_t *front, *back;

	front = scr_pane_new(scr_pane_get(ui->cur));
	back = scr_pane_new(ui->func(ui->arg));
	scr_pane_set(ui->cur, scr_split_widget(scr_split_new(scr_split_vert_e, 0.5, front, back)));
	ui->cur = front;
}

_export
bool scr_ui_close(struct scr_ui_t *ui)
{
	struct scr_split_t *split;

	split = scr_pane_split(ui->pane);
	if(split == NULL)
		return false;

	scr_split_close(&ui->pane, split);

	return true;
}


/**
 * Process a tab on the UI.
 *   @ui: The ui.
 */

_export
void scr_ui_tab(struct scr_ui_t *ui)
{
	ui->cur = scr_pane_tab(ui->pane) ?: ui->cur;
}

/**
 * Process a reverse tab on the UI.
 *   @ui: The ui.
 */

_export
void scr_ui_rtab(struct scr_ui_t *ui)
{
	ui->cur = scr_pane_rtab(ui->pane) ?: ui->cur;
}


/**
 * Response callback for commands.
 *   @key: The key.
 *   @context: The context.
 *   @complete: Auto-complete handler.
 *   @arg: The argument.
 *   &returns: True on keys other than newline and tab.
 */

static bool cmd_resp(int32_t key, struct scr_context_t context, struct scr_complete_h complete, void *arg)
{
	struct scr_ui_t *ui = arg;

	switch(key) {
	case '\r':
	case '\n':
		{
			char cmd[str_len(ui->buf) + 1];

			str_copy(cmd, ui->buf);

			scr_ui_clear(ui);
			scr_cmd_exec(ui->cmd, cmd, context);
		}

		return false;

	default:
		return true;
	}
}
