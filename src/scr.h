#ifndef SCR_H
#define SCR_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_t;

/*
 * screen function declarations
 */

struct scr_t *scr_open(struct io_input_t input, struct io_output_t output);
void scr_close(struct scr_t *scr);

int32_t scr_read(struct scr_t *scr, int timeout);
struct scr_size_t scr_size(struct scr_t *scr);
struct scr_buf_t *scr_buf(struct scr_t *scr);
void scr_swap(struct scr_t *scr, struct scr_buf_t *buf);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
