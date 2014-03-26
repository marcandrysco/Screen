#ifndef PT_H
#define PT_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/**
 * Create a default point.
 *   @code: The code.
 *   &returns: The point.
 */

static inline struct scr_pt_t scr_pt_default(uint32_t code)
{
	return (struct scr_pt_t){ code, { scr_default_e, scr_default_e, false, false, false} };
}

/**
 * Create a negated point.
 *   @code: The code.
 *   &returns: The point.
 */

static inline struct scr_pt_t scr_pt_neg(uint32_t code)
{
	return (struct scr_pt_t){ code, { scr_default_e, scr_default_e, false, false, true } };
}

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
