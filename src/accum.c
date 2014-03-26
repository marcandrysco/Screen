#include "common.h"
#include "accum.h"
#include "buf.h"


/**
 * Accumulator structure.
 *   @inst: Tree of instances.
 *   @box: Bounding box.
 */

struct scr_accum_t {
	int top, bottom, left, right;

	struct avltree_t inst;
};

/**
 * Coordinate instance.
 *   @pt: The point.
 *   @coord: The coordinates.
 */

struct inst_t {
	struct scr_pt_t pt;
	struct scr_coord_t coord;
};


/*
 * local function declarations
 */

static int scr_coord_cmp(const struct scr_coord_t *c1, const struct scr_coord_t *c2);


/**
 * Create an accumulator.
 *   &returns: The accumulator.
 */

_export
struct scr_accum_t *scr_accum_new()
{
	struct scr_accum_t *accum;
	
	accum = mem_alloc(sizeof(struct scr_accum_t));
	accum->top = INT_MAX;
	accum->bottom = INT_MIN;
	accum->left = INT_MAX;
	accum->right = INT_MIN;
	avltree_init(&accum->inst, (compare_f)scr_coord_cmp, mem_free);

	return accum;
}

/**
 * Delete an accumulator.
 *   @accum: The accumulator.
 */

_export
void scr_accum_delete(struct scr_accum_t *accum)
{
	avltree_destroy(&accum->inst);
	mem_free(accum);
}


/**
 * Set the point at a given coordinate.
 *   @accum: The accumulator.
 *   @coord: The coordinates.
 *   @pt: The point.
 */

_export
void scr_accum_set(struct scr_accum_t *accum, struct scr_coord_t coord, struct scr_pt_t pt)
{
	struct inst_t *inst;

	inst = avltree_lookup(&accum->inst, &coord);
	if(inst == NULL) {
		inst = mem_alloc(sizeof(struct inst_t));
		inst->coord = coord;
		inst->pt = pt;

		if(accum->left > coord.x)
			accum->left = coord.x;

		if(accum->right < coord.x)
			accum->right = coord.x;

		if(accum->top > coord.y)
			accum->top = coord.y;

		if(accum->bottom < coord.y)
			accum->bottom = coord.y;

		avltree_insert(&accum->inst, &inst->coord, inst);
	}
	else
		inst->pt = pt;
}


/**
 * Retrieve the accumulated width.
 *   @accum: The accumulator.
 *   &returns: The width.
 */

_export
unsigned int scr_accum_width(struct scr_accum_t *accum)
{
	return (accum->left <= accum->right) ? (accum->right - accum->left + 1) : 0;
}

/**
 * Retrieve the accumulated height.
 *   @accum: The accumulator.
 *   &returns: The height.
 */

_export
unsigned int scr_accum_height(struct scr_accum_t *accum)
{
	return (accum->top <= accum->bottom) ? (accum->bottom - accum->top + 1) : 0;
}

/**
 * Retrieve the accumulated size.
 *   @accum: The accumulator.
 *   &returns: The size.
 */

_export
struct scr_size_t scr_accum_size(struct scr_accum_t *accum)
{
	return (struct scr_size_t){ scr_accum_width(accum), scr_accum_height(accum) };
}


/**
 * Compare two coordinates.
 *   @c1: The first coordinates.
 *   @c2: The second coordinates.
 *   &returns: Their order.
 */

static int scr_coord_cmp(const struct scr_coord_t *c1, const struct scr_coord_t *c2)
{
	if(c1->y > c2->y)
		return -2;
	else if(c1->y < c2->y)
		return 2;
	else if(c1->x > c2->x)
		return -1;
	else if(c1->x < c2->x)
		return 1;
	else
		return 0;
}


/**
 * Create a buffer from the accumulator.
 *   @accum: The accumulator.
 *   &returns: The buffer.
 */

struct scr_buf_t *scr_accum_buf(struct scr_accum_t *accum)
{
	struct inst_t *inst;
	struct scr_box_t box;
	struct avltree_iter_t iter;
	struct scr_buf_t *buf;

	box.coord.x = accum->left;
	box.coord.y = accum->top;
	box.size.width = scr_accum_width(accum);
	box.size.height = scr_accum_height(accum);

	buf = scr_buf_new(box);

	avltree_iter_init(&iter, &accum->inst);
	while((inst = avltree_iter_next(&iter)) != NULL)
		*scr_buf_pt(buf, inst->coord) = inst->pt;

	return buf;
}
