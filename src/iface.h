#ifndef IFACE_H
#define IFACE_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_impl_t;

/*
 * implementation function declarations
 */

struct scr_impl_t *scr_impl_open(struct io_input_t input, struct io_output_t output);
void scr_impl_close(struct scr_impl_t *impl);

int32_t scr_impl_read(struct scr_impl_t *impl, int timeout);
struct scr_size_t scr_impl_size(struct scr_impl_t *impl);
void scr_impl_swap(struct scr_impl_t *impl, struct scr_buf_t *buf);

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
