#include "common.h"
#include "buf.h"
#include "iface.h"


/**
 * Screen structure.
 *   @impl: The implementation.
 */

struct scr_t {
	struct scr_impl_t *impl;
};


/**
 * Open a screen on an input and output.
 *   @input: The input.
 *   @output: The output.
 *   &returns: The screen.
 */

_export
struct scr_t *scr_open(struct io_input_t input, struct io_output_t output)
{
	struct scr_t *scr;

	scr = mem_alloc(sizeof(struct scr_t));
	scr->impl = scr_impl_open(input, output);

	return scr;
}

/**
 * Close a screen.
 *   @scr: The screen.
 */

_export
void scr_close(struct scr_t *scr)
{
	scr_impl_close(scr->impl);
	mem_free(scr);
}


/**
 * Read an input from the screen.
 *   @scr: The screen.
 *   @timeout: The timeout.
 *   &returns: The next key.
 */

_export
int32_t scr_read(struct scr_t *scr, int timeout)
{
	return scr_impl_read(scr->impl, timeout);
}

/**
 * Retrieve the screen size.
 *   @scr: The screen.
 *   &returns: The size.
 */

_export
struct scr_size_t scr_size(struct scr_t *scr)
{
	return scr_impl_size(scr->impl);
}

/**
 * Generate a blank buffer for the screen.
 *   @scr: The screen.
 */

_export
struct scr_buf_t *scr_buf(struct scr_t *scr)
{
	return scr_buf_new((struct scr_box_t){ { 0, 0 }, scr_size(scr) });
}

/**
 * Swap buffers.
 *   @impl: The implementation.
 *   @buf: The new buffer.
 */

_export
void scr_swap(struct scr_t *scr, struct scr_buf_t *buf)
{
	scr_impl_swap(scr->impl, buf);
}
