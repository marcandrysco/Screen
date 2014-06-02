#ifndef WIDGET_HANDLER_H
#define WIDGET_HANDLER_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_context_t;


/**
 * Command function.
 *   @cmd: The command.
 *   @context: The context.
 *   @arg: The argument.
 */

typedef void (*scr_cmd_f)(const char *cmd, struct scr_context_t context, void *arg);

/**
 * Command handler.
 *   @func: The function.
 *   @arg: The argument.
 */

struct scr_cmd_h {
	scr_cmd_f func;
	void *arg;
};


/**
 * Auto-complete text.
 *   @str: The string.
 *   @arg: The argument.
 */

typedef void (*scr_complete_f)(const char *str, void *arg);

/**
 * Auto-complete handler.
 *   @func: The function.
 *   @arg: The argument.
 */

struct scr_complete_h {
	scr_complete_f func;
	void *arg;
};


/*
 * handler function declarations
 */

void scr_cmd_exec(struct scr_cmd_h cmd, const char *str, struct scr_context_t context);
void scr_select_exec(struct scr_select_h select, const char *entry, void *key, struct scr_context_t context);
void scr_complete_exec(struct scr_complete_h complete, const char *str);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
