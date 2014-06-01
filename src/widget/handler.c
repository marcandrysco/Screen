#include "../common.h"
#include "handler.h"
#include "widget.h"


/**
 * Execute a command handler.
 *   @cmd: The command handler.
 *   @str: The comman stringd
 *   @key: The key.
 *   @context: The context.
 */

_export
void scr_cmd_exec(struct scr_cmd_h cmd, const char *str, struct scr_context_t context)
{
	if(cmd.func != NULL)
		cmd.func(str, context, cmd.arg);
}

/**
 * Execute a select handler.
 *   @select: The select handler.
 *   @entry: The entry.
 *   @key: The key.
 *   @context: The context.
 */

_export
void scr_select_exec(struct scr_select_h select, const char *entry, void *key, struct scr_context_t context)
{
	if(select.func != NULL)
		select.func(entry, key, context, select.arg);
}

/**
 * Execute the auto-complete handler.
 *   @complete: The auto-complete handler.
 *   @str: The string.
 */

_export
void scr_complete_exec(struct scr_complete_h complete, const char *str)
{
	if(complete.func != NULL)
		complete.func(str, complete.arg);
}
