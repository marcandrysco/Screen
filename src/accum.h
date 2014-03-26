#ifndef ACCUM_H
#define ACCUM_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_accum_t;

/*
 * accumulator function declarations
 */

struct scr_accum_t *scr_accum_new();
void scr_accum_delete(struct scr_accum_t *accum);

void scr_accum_set(struct scr_accum_t *accum, struct scr_coord_t coord, struct scr_pt_t pt);

struct scr_buf_t *scr_accum_buf(struct scr_accum_t *accum);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
