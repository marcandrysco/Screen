#ifndef MATCH_H
#define MATCH_H

/*
 * Start Header Creation: scr.h
 */

/* %scr.h% */

/**
 * Create an iterator for matching.
 *   @arg: The argument.
 *   &returns: The iterator.
 */

typedef struct iter_t (*scr_match_iter_f)(void *arg);

/**
 * Process the matched input.
 *   @input: The input.
 *   @arg: The argument.
 */

typedef void (*scr_match_proc_f)(const char *input, void *arg);


/*
 * matching function declarations
 */

struct scr_resp_t scr_match(scr_match_iter_f iter, scr_match_proc_f proc, void *arg);

/* %~scr.h% */

/*
 * End Header Creation: scr.h
 */

#endif
