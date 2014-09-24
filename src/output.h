#ifndef OUTPUT_H
#define OUTPUT_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/**
 * Text wrap enumerator.
 *   @scr_wrap_none_e: No wrapping.
 *   @scr_wrap_char_e: Wrap on character.
 *   @scr_wrap_word_e: Wrap on word.
 */

enum scr_wrap_e {
	scr_wrap_none_e,
	scr_wrap_char_e,
	scr_wrap_word_e
};

/**
 * Output function.
 *   @coord: The coordinate.
 *   @pt: The point.
 *   @arg: The argument.
 */

typedef void (*scr_output_f)(struct scr_coord_t coord, struct scr_pt_t pt, void *arg);

/**
 * Output structure.
 *   @func: The output function.
 *   @arg: The argument.
 *   @coord: The current coordinates.
 *   @prop: The current property set.
 *   @wrap: Text wrapping mode.
 */

struct scr_output_t {
	scr_output_f func;
	void *arg;

	struct scr_coord_t coord;
	struct scr_prop_t prop;

	enum scr_wrap_e wrap;
};

/**
 * Render structure.
 *   @func: The output function.
 *   @arg: The argument.
 *   @box: The target box.
 *   @prop: The current property set.
 */

struct scr_render_t {
	scr_output_f func;
	void *arg;

	struct scr_box_t box;
	struct scr_prop_t prop;
};


/**
 * Create an output.
 *   @func: The function.
 *   @arg: The argument.
 *   &returns: The output.
 */

static inline struct scr_output_t scr_output_new(scr_output_f func, void *arg)
{
	return (struct scr_output_t){ func, arg, { 0, 0 }, { scr_default_e, scr_default_e, false, false, false }, scr_wrap_none_e };
}

/**
 * Create a view that covers all space after the output.
 *   @output: The output.
 *   @view: The original view.
 */

static inline void scr_output_next(struct scr_output_t output, struct scr_view_t *view)
{
	view->box.coord.y += output.coord.y;
	view->box.size.height -= output.coord.y;
}

/**
 * Create an renderer.
 *   @func: The function.
 *   @arg: The argument.
 *   @box: The target box.
 *   &returns: The renderer.
 */

static inline struct scr_render_t scr_render_new(scr_output_f func, void *arg, struct scr_box_t box)
{
	return (struct scr_render_t){ func, arg, box, { scr_default_e, scr_default_e, false, false, false } };
}


/*
 * output function declarations
 */

void scr_output_write(struct scr_output_t *output, const void *restrict buf, size_t nbytes);

void scr_printf(struct scr_output_t *output, const char *restrict format, ...);
void scr_vprintf(struct scr_output_t *output, const char *restrict format, va_list args);

struct io_chunk_t scr_chunk_bold(bool value);
struct io_chunk_t scr_chunk_neg(bool value);
struct io_chunk_t scr_chunk_uline(bool value);
struct io_chunk_t scr_chunk_error(bool value);

/*
 * view function declarations
 */

void scr_view_fill(struct scr_view_t view, struct scr_pt_t pt);
void scr_view_fill_code(struct scr_view_t view, uint32_t code);

/*
 * render function declarations
 */

void scr_render_set(struct scr_render_t *render, struct scr_coord_t coord, char ch);
void scr_render_fill(struct scr_render_t *render, char ch);
void scr_render_border(struct scr_render_t *render, char ch);

/*
 * output implementation functions
 */

void scr_view_output(struct scr_coord_t coord, struct scr_pt_t pt, void *arg);
void scr_accum_output(struct scr_coord_t coord, struct scr_pt_t pt, void *arg);

/*
 * output implementation macros
 */

#define scr_output_view(view) scr_output_new(scr_view_output, &(union { struct scr_view_t v; }){ .v = view }.v)
#define scr_output_accum(accum) scr_output_new(scr_accum_output, (union { struct scr_accum_t *v; }){ .v = accum }.v)

#define scr_render_view(view) _scr_render_view(&(union { struct scr_view_t v; }){ .v = view }.v)
#define scr_render_accum(accum, box) scr_render_new(scr_accum_output, (union { struct scr_accum_t *v; }){ .v = accum }.v, box)


/**
 * View reference renderer helper.
 *   @view: The view.
 *   &returns: The renderer.
 */

static inline struct scr_render_t _scr_render_view(struct scr_view_t *view)
{
	return (struct scr_render_t){ scr_view_output, view, view->box, { scr_default_e, scr_default_e, false, false, false } };
}

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
