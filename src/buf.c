#include "common.h"
#include "buf.h"


/*
 * global variables
 */

_export struct scr_pt_t scr_pt_blank = { ' ', { scr_default_e, scr_default_e, false, false, false } };


/**
 * Creat a new buffer.
 *   @box: The box.
 *   &returns: The buffer.
 */

_export
struct scr_buf_t *scr_buf_new(struct scr_box_t box)
{
	struct scr_buf_t *buf;
	unsigned int i, npts = box.size.width * box.size.height;

	buf = mem_alloc(sizeof(struct scr_buf_t) + npts * sizeof(struct scr_pt_t));
	buf->box = box;

	for(i = 0; i < npts; i++)
		buf->pt[i] = scr_pt_blank;

	return buf;
}

/**
 * Delete a buffer.
 *   @buf: The buffer.
 */

_export
void scr_buf_delete(struct scr_buf_t *buf)
{
	mem_free(buf);
}

/**
 * Replace a buffer.
 *   @dest: The destination buffer reference.
 *   @src: The source buffer.
 */

_export
void scr_buf_replace(struct scr_buf_t **dest, struct scr_buf_t *src)
{
	if(*dest != NULL)
		scr_buf_delete(*dest);

	*dest = src;
}


/**
 * Draw one buffer onto another.
 *   @dest: The destination buffer.
 *   @src: The source buffer.
 */

void scr_draw_buf(struct scr_buf_t *dest, struct scr_buf_t *src)
{
	struct scr_pt_t *pt;
	unsigned int x, y, left, right, top, bottom;

	left = src->box.coord.x;
	top = src->box.coord.y;
	right = left + src->box.size.width;
	bottom = top + src->box.size.height;

	for(x = left; x < right; x++) {
		for(y = top; y < bottom; y++) {
			pt = scr_buf_pt(dest, (struct scr_coord_t){ x, y });
			if(pt != NULL)
				*pt = scr_buf_get(src, (struct scr_coord_t){ x, y });
		}
	}
}

/**
 * Draw a buffer onto a view.
 *   @dest: The destination vview.
 *   @src: The source buffer.
 */

void scr_draw_view(struct scr_view_t dest, struct scr_buf_t *src)
{
	struct scr_coord_t coord;
	unsigned int left, right, top, bottom;

	left = src->box.coord.x;
	top = src->box.coord.y;
	right = left + src->box.size.width;
	bottom = top + src->box.size.height;

	for(coord.x = left; coord.x < right; coord.x++) {
		for(coord.y = top; coord.y < bottom; coord.y++)
			scr_view_set(dest, coord, scr_buf_get(src, coord));
	}
}
