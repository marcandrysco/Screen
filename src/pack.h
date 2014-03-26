#ifndef PACK_H
#define PACK_H

/*
 * start header: scr.h
 */

/* %scr.h% */


/**
 * View pair structure.
 *   @front: The front.
 *   @back: The back.
 */

struct scr_pair_t {
	struct scr_view_t front, back;
};


/**
 * Pack a view into internally packed box.
 *   @view: The view.
 *   @box: The internal box.
 *   &returns: The sub-view.
 */

static inline struct scr_view_t scr_pack_sub(struct scr_view_t view, struct scr_box_t box)
{
	struct scr_view_t pack;

	pack.buf = view.buf;
	pack.box.coord.x = view.box.coord.x + box.coord.x;
	pack.box.coord.y = view.box.coord.y + box.coord.y;
	pack.box.size.width = box.size.width;
	pack.box.size.height = box.size.height;

	return pack;
}

/**
 * Pack a view into padded box.
 *   @view: The view.
 *   @horiz: Horizontal padding.
 *   @vert: Vertical padding.
 *   &returns: The packed view.
 */

static inline struct scr_view_t scr_pack_pad(struct scr_view_t view, unsigned int horiz, unsigned vert)
{
	struct scr_view_t pack;

	pack.buf = view.buf;
	pack.box.coord.x = view.box.coord.x + horiz;
	pack.box.coord.y = view.box.coord.y + vert;
	pack.box.size.width -= 2 * horiz;
	pack.box.size.height -= 2 * vert;

	return pack;
}


/**
 * Pack a view horizontally, moving the view to the right.
 *   @view: The view reference.
 *   @width: The width of the pack.
 *   &returns: The packed view.
 */

static inline struct scr_view_t scr_pack_horiz(struct scr_view_t *view, unsigned int width)
{
	struct scr_view_t pack;

	if(width > view->box.size.width)
		width = view->box.size.width;

	pack.buf = view->buf;
	pack.box.coord = view->box.coord;
	pack.box.size.width = width;
	pack.box.size.height = view->box.size.height;

	view->box.coord.x += width;
	view->box.size.width -= width;

	return pack;
}

/**
 * Pack a view vertically, moving the view to the right.
 *   @view: The view reference.
 *   @width: The width of the pack.
 *   &returns: The packed view.
 */

static inline struct scr_view_t scr_pack_vert(struct scr_view_t *view, unsigned int height)
{
	struct scr_view_t pack;

	if(height > view->box.size.height)
		height = view->box.size.height;

	pack.buf = view->buf;
	pack.box.coord = view->box.coord;
	pack.box.size.width = view->box.size.width;
	pack.box.size.height = height;

	view->box.coord.y += height;
	view->box.size.height -= height;

	return pack;
}


/**
 * Pack a pair of views, creating a front element with given height and back
 * element with the remaining space.
 *   @view: The view.
 *   @height: The height.
 *   &returns: The pair.
 */

static inline struct scr_pair_t scr_pack_top(struct scr_view_t view, unsigned int height)
{
	struct scr_pair_t pair;

	if(height > view.box.size.height)
		height = view.box.size.height;

	pair.front.buf = view.buf;
	pair.front.box.coord.x = view.box.coord.x;
	pair.front.box.coord.y = view.box.coord.y;
	pair.front.box.size.width = view.box.size.width;
	pair.front.box.size.height = height;

	pair.back.buf = view.buf;
	pair.back.box.coord.x = view.box.coord.x;
	pair.back.box.coord.y = view.box.coord.y + height;
	pair.back.box.size.width = view.box.size.width;
	pair.back.box.size.height = view.box.size.height - height;

	return pair;
}

/**
 * Pack a pair of views, creating a back element with given height and front
 * element with the remaining space.
 *   @view: The view.
 *   @height: The height.
 *   &returns: The pair.
 */

static inline struct scr_pair_t scr_pack_bottom(struct scr_view_t view, unsigned int height)
{
	struct scr_pair_t pair;

	if(height > view.box.size.height)
		height = view.box.size.height;

	pair.front.buf = view.buf;
	pair.front.box.coord.x = view.box.coord.x;
	pair.front.box.coord.y = view.box.coord.y;
	pair.front.box.size.width = view.box.size.width;
	pair.front.box.size.height = view.box.size.height - height;

	pair.back.buf = view.buf;
	pair.back.box.coord.x = view.box.coord.x;
	pair.back.box.coord.y = view.box.coord.y + view.box.size.height - height;
	pair.back.box.size.width = view.box.size.width;
	pair.back.box.size.height = height;

	return pair;
}

/**
 * Pack a pair of views, creating a front element with given width and back
 * element with the remaining space.
 *   @view: The view.
 *   @width: The width.
 *   &returns: The pair.
 */

static inline struct scr_pair_t scr_pack_left(struct scr_view_t view, unsigned int width)
{
	struct scr_pair_t pair;

	if(width > view.box.size.width)
		width = view.box.size.width;

	pair.front.buf = view.buf;
	pair.front.box.coord.x = view.box.coord.x;
	pair.front.box.coord.y = view.box.coord.y;
	pair.front.box.size.width = width;
	pair.front.box.size.height = view.box.size.height;

	pair.back.buf = view.buf;
	pair.back.box.coord.x = view.box.coord.x + width;
	pair.back.box.coord.y = view.box.coord.y;
	pair.back.box.size.width = view.box.size.width - width;
	pair.back.box.size.height = view.box.size.height;

	return pair;
}

/**
 * Pack a pair of views, creating a back element with given width and front
 * element with the remaining space.
 *   @view: The view.
 *   @width: The width.
 *   &returns: The pair.
 */

static inline struct scr_pair_t scr_pack_right(struct scr_view_t view, unsigned int width)
{
	struct scr_pair_t pair;

	if(width > view.box.size.width)
		width = view.box.size.width;

	pair.front.buf = view.buf;
	pair.front.box.coord.x = view.box.coord.x;
	pair.front.box.coord.y = view.box.coord.y;
	pair.front.box.size.width = view.box.size.width - width;
	pair.front.box.size.height = view.box.size.height;

	pair.back.buf = view.buf;
	pair.back.box.coord.x = view.box.coord.x + view.box.size.width - width;
	pair.back.box.coord.y = view.box.coord.y;
	pair.back.box.size.width = width;
	pair.back.box.size.height = view.box.size.height;

	return pair;
}

/**
 * Pack a pair of view, creating a back element of height one and front
 * element of the remaining space.
 *   @view: The view.
 *   &returns: THe pair.
 */

static inline struct scr_pair_t scr_pack_status(struct scr_view_t view)
{
	return scr_pack_bottom(view, 1);
}


/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
