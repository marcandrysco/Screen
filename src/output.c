#include "common.h"
#include "output.h"
#include "accum.h"
#include "buf.h"


/*
 * local function declarations
 */

static size_t output_write(void *ref, const void *buf, size_t nbytes);
static bool output_ctrl(struct scr_output_t *output, unsigned int cmd, void *arg);

static void bold_proc(struct io_output_t output, void *arg);
static void neg_proc(struct io_output_t output, void *arg);
static void error_proc(struct io_output_t output, void *arg);

/*
 * local variables
 */

static const struct io_output_i output_iface = { { (io_ctrl_f)output_ctrl, NULL }, output_write };


/**
 * Write data to the output.
 *   @output: Output.
 *   @buf: The buffer.
 *   @nbytes: The number of bytes.
 */

_export
void scr_output_write(struct scr_output_t *output, const void *restrict buf, size_t nbytes)
{
	const char *text = buf;

	switch(output->wrap) {
	case scr_wrap_none_e:
		while(nbytes > 0) {
			if(*text == '\n') {
				text++;
				output->coord.x = 0;
				output->coord.y++;
				nbytes--;
			}
			else if((uint8_t)*text >= 0x80) {
				uint8_t len;
				uint32_t code;
				uint8_t val = (uint8_t)(*text++);

				len = m_uintclz(~val << (8 * (sizeof(unsigned int) - 1)));
				code = val & ~(((1 << len) - 1) << (8 - len));

				nbytes -= len;
				for(len--; len > 0; len--)
					code = (code << 6) | ((uint8_t)(*text++) & 0x3F);

				output->func(output->coord, (struct scr_pt_t){ code, output->prop }, output->arg);
				output->coord.x++;
			}
			else {
				uint8_t val = *text++;

				output->func(output->coord, (struct scr_pt_t){ val, output->prop }, output->arg);
				//scr_view_set(output->view, output->coord, (struct scr_pt_t){ *text, output->prop });

				output->coord.x++;
				nbytes--;
			}
		}

		break;

	default:
		_fatal("stub, haven't implemented wrapping");
	}
}


/**
 * Print formatted text to an output.
 *   @output: The output.
 *   @format: Format string.
 *   @...: The printf-style arguments.
 */

_export
void scr_printf(struct scr_output_t *output, const char *restrict format, ...)
{
	va_list args;

	va_start(args, format);
	scr_vprintf(output, format, args);
	va_end(args);
}

/**
 * Print formatted text to an output using a variable argument list.
 *   @output: The output.
 *   @format: Format string.
 *   @...: The printf-style variable argument list.
 */

_export
void scr_vprintf(struct scr_output_t *output, const char *restrict format, va_list args)
{
	struct io_output_t internal = { output, &output_iface };

	io_vprintf(internal, format, args);
}


/**
 * Handle a control signal.
 *   @output: The output.
 *   @cmd: The command.
 *   @arg: The argument.
 */

static bool output_ctrl(struct scr_output_t *output, unsigned int cmd, void *arg)
{
	switch(cmd) {
	case scr_propget_e: *(struct scr_prop_t *)arg = output->prop; break;
	case scr_propset_e: output->prop = *(struct scr_prop_t *)arg; break;
	default: return false;
	}

	return true;
}

/**
 * Callback for writing to an internal string.
 *   @ref: The reference.
 *   @buf: The buffer.
 *   @nbytes: The number of bytes to write.
 *   &returns: The number of bytes written.
 */

static size_t output_write(void *ref, const void *buf, size_t nbytes)
{
	scr_output_write(ref, buf, nbytes);

	return nbytes;
}


/**
 * Create a chunk for modifying the bold flag.
 *   @value: The bold flag.
 *   &returns: The chunk.
 */

_export
struct io_chunk_t scr_chunk_bold(bool value)
{
	return (struct io_chunk_t){ bold_proc, value ? (void *)1 : (void *)0 };
}

/**
 * Process a bold flag chunk.
 *   @output: The output.
 *   @arg: The argument.
 */

static void bold_proc(struct io_output_t output, void *arg)
{
	struct scr_prop_t prop;

	io_output_ctrl(output, scr_propget_e, &prop);
	prop.bold = (arg != (void *)0);
	io_output_ctrl(output, scr_propset_e, &prop);
}


/**
 * Create a chunk for modifying the negative flag.
 *   @value: The negative flag.
 *   &returns: The chunk.
 */

_export
struct io_chunk_t scr_chunk_neg(bool value)
{
	return (struct io_chunk_t){ neg_proc, value ? (void *)1 : (void *)0 };
}

/**
 * Process a negative flag chunk.
 *   @output: The output.
 *   @arg: The argument.
 */

static void neg_proc(struct io_output_t output, void *arg)
{
	struct scr_prop_t prop;

	io_output_ctrl(output, scr_propget_e, &prop);
	prop.neg = (arg != (void *)0);
	io_output_ctrl(output, scr_propset_e, &prop);
}


/**
 * Create a chunk for displaying errors.
 *   @value: The error flag.
 *   &returns: The chunk.
 */

_export
struct io_chunk_t scr_chunk_error(bool value)
{
	return (struct io_chunk_t){ error_proc, value ? (void *)1 : (void *)0 };
}

/**
 * Process a error flag chunk.
 *   @output: The output.
 *   @arg: The argument.
 */

static void error_proc(struct io_output_t output, void *arg)
{
	struct scr_prop_t prop;
	bool flag = (arg != (void *)0);

	io_output_ctrl(output, scr_propget_e, &prop);
	prop.bold = flag;
	prop.fg = flag ? scr_white_e : scr_default_e;
	prop.bg = flag ? scr_red_e : scr_default_e;
	io_output_ctrl(output, scr_propset_e, &prop);
}


static void render_pt(struct scr_render_t *render, struct scr_coord_t coord, struct scr_pt_t pt)
{
	render->func(coord, pt, render->arg);
}

/**
 * Set a point.
 *   @render: The renderer.
 *   @coord: The coordinates.
 *   @ch: The fill character.
 */

_export
void scr_render_set(struct scr_render_t *render, struct scr_coord_t coord, char ch)
{
	render_pt(render, coord, (struct scr_pt_t){ ch, render->prop });
}

/**
 * Render a fill.
 *   @render: The renderer.
 *   @ch: The fill character.
 */

_export
void scr_render_fill(struct scr_render_t *render, char ch)
{
	unsigned int x, y;
	struct scr_box_t box = render->box;
	struct scr_pt_t pt = { ch, render->prop };

	for(y = 0; y < box.size.height; y++) {
		for(x = 0; x < box.size.width; x++)
			render_pt(render, (struct scr_coord_t){ x, y }, pt);
	}
}

/**
 * Render a border.
 *   @render: The renderer.
 *   @ch: The border character.
 */

_export
void scr_render_border(struct scr_render_t *render, char ch)
{
	struct scr_box_t box = render->box;
	struct scr_pt_t pt = { ch, render->prop };
	unsigned int i, right = box.size.width - 1, bottom = box.size.height - 1;

	for(i = 0; i < box.size.width; i++) {
		render_pt(render, (struct scr_coord_t){ i, 0 }, pt);
		render_pt(render, (struct scr_coord_t){ i, bottom }, pt);
	}

	for(i = 1; i < bottom; i++) {
		render_pt(render, (struct scr_coord_t){ 0, i }, pt);
		render_pt(render, (struct scr_coord_t){ right, i}, pt);
	}
}


/**
 * View output implementation.
 *   @coord: The coordinate.
 *   @pt: The point.
 *   @arg: The argument.
 */

_export
void scr_view_output(struct scr_coord_t coord, struct scr_pt_t pt, void *arg)
{
	scr_view_set(*(struct scr_view_t *)arg, coord, pt);
}

/**
 * Accumulator output implementation.
 *   @coord: The coordinate.
 *   @pt: The point.
 *   @arg: The argument.
 */

_export
void scr_accum_output(struct scr_coord_t coord, struct scr_pt_t pt, void *arg)
{
	scr_accum_set(arg, coord, pt);
}
