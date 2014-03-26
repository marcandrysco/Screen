#ifndef BUF_H
#define BUF_H

/*
 * Start Header Creation: scr.h
 */

/* %scr.h% */

/**
 * Buffer structure.
 *   @box: The buffer box.
 *   @pt: The point array.
 */

struct scr_buf_t {
	struct scr_box_t box;

	struct scr_pt_t pt[];
};


/*
 * buffer variables
 */

extern struct scr_pt_t scr_pt_blank;

/*
 * buffer function declarations
 */

struct scr_buf_t *scr_buf_new(struct scr_box_t box);
void scr_buf_delete(struct scr_buf_t *buf);
void scr_buf_replace(struct scr_buf_t **dest, struct scr_buf_t *src);

/*
 * draw function declarations
 */

void scr_draw_buf(struct scr_buf_t *dest, struct scr_buf_t *src);
void scr_draw_view(struct scr_view_t dest, struct scr_buf_t *src);


/**
 * Add two coordinate together.
 *   @c1: The first coordinate.
 *   @c2: The seoncd coordinate.
 *   &returns: The coordinate.
 */

static inline struct scr_coord_t scr_coord_add(struct scr_coord_t c1, struct scr_coord_t c2)
{
	return (struct scr_coord_t){ c1.x + c2.x, c1.y + c2.y };
}


/**
 * Determine if the coordinate is inside the size.
 *   @size: The size.
 *   @coord: The corodiante.
 *   &return: True if inside, false otherwise.
 */

static inline bool scr_size_inside(struct scr_size_t size, struct scr_coord_t coord)
{
	return (coord.x < size.width) && (coord.y < size.height);
}


/**
 * Retrieve the index for a coordinate.
 *   @buf: The buffer.
 *   @coord: The coordinate.
 *   &returns: The index.
 */

static inline unsigned int scr_buf_index(struct scr_buf_t *buf, struct scr_coord_t coord)
{
	return (coord.y - buf->box.coord.y) * buf->box.size.width + (coord.x - buf->box.coord.x);
}

/**
 * Determine if the given coordinate is inside the buffer.
 *   @buf: The buffer.
 *   @coord: The corodinate.
 *   &returns: True if inside the buffer, false otherwise.
 */

static inline bool scr_buf_inside(struct scr_buf_t *buf, struct scr_coord_t coord)
{
	coord.x -= buf->box.coord.x;
	coord.y -= buf->box.coord.y;

	return (coord.x >= 0) && (coord.y >= 0) && (coord.x < buf->box.size.width) && (coord.y < buf->box.size.height);
}

/**
 * Retrieve a point.
 *   @buf: The buffer.
 *   @coord: The coordinate.
 *   &return: The point.
 */

static inline struct scr_pt_t scr_buf_get(struct scr_buf_t *buf, struct scr_coord_t coord)
{
	return scr_buf_inside(buf, coord) ? buf->pt[scr_buf_index(buf, coord)] : scr_pt_blank;
}

/**
 * Retrieve a point inside a buffer.
 *   @buf: The buffer.
 *   @coord: The coordinate.
 *   &returns: The point, or null if outside the buffer.
 */

static inline struct scr_pt_t *scr_buf_pt(struct scr_buf_t *buf, struct scr_coord_t coord)
{
	return scr_buf_inside(buf, coord) ? &buf->pt[scr_buf_index(buf, coord)] : NULL;
}


/**
 * Determine if the coordinate is inside the box.
 *   @box: The box.
 *   @coord: The corodiante.
 *   &return: True if inside, false otherwise.
 */

static inline bool scr_box_inside(struct scr_box_t box, struct scr_coord_t coord)
{
	return (coord.x >= box.coord.x) && (coord.y >= box.coord.y) && (coord.x < (box.coord.x + box.size.width)) && (coord.y < (box.coord.y + box.size.height));
}


/**
 * Create a fiew for the buffer.
 *   @buf: The buffer.
 *   &returns: The view.
 */

static inline struct scr_view_t scr_view_new(struct scr_buf_t *buf)
{
	return (struct scr_view_t){ buf, { { 0, 0 }, buf->box.size } };
}

/**
 * Retrieve a reference to a point on the view.
 *   @view: The view.
 *   @coord: The coordinates.
 *   &returns: The point or null.
 */

static inline struct scr_pt_t *scr_view_ref(struct scr_view_t view, struct scr_coord_t coord)
{
	return scr_buf_pt(view.buf, scr_coord_add(view.box.coord, coord));
}

/**
 * Set a point on the view.
 *   @view: The view.
 *   @coord: The coordinate.
 *   @pt: The point.
 *   &returns: True if the point is within the view, false otherwise.
 */

static inline bool scr_view_set(struct scr_view_t view, struct scr_coord_t coord, struct scr_pt_t pt)
{
	struct scr_pt_t *ref;

	ref = scr_buf_pt(view.buf, scr_coord_add(view.box.coord, coord));
	if(ref == NULL)
		return false;

	*ref = pt;

	return true;
}

/**
 * Set a code value on the view.
 *   @view: The view.
 *   @coord: The coordinate.
 *   @code: The code value.
 *   &returns: True if the coordinate is within the view, false otherwise.
 */

static inline bool scr_view_set_code(struct scr_view_t view, struct scr_coord_t coord, uint32_t code)
{
	struct scr_pt_t *ref;

	ref = scr_buf_pt(view.buf, scr_coord_add(view.box.coord, coord));
	if(ref == NULL)
		return false;

	ref->code = code;

	return true;
}

/* %~scr.h% */

/*
 * End Header Creation: scr.h
 */

#endif
