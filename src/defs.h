#ifndef DEFS_H
#define DEFS_H

/*
 * start header: scr.h
 */

/* %scr.h% */

/*
 * structure prototypes
 */

struct scr_buf_t;


/**
 * Coordinate structure.
 *   @x, y: The x and y coordinates.
 */

struct scr_coord_t {
	int x, y;
};

/**
 * Size structure.
 *   @width, height: With and height.
 */

struct scr_size_t {
	unsigned int width, height;
};

/**
 * Box structure.
 *   @coord: The coordinate.
 *   @size: The size.
 */

struct scr_box_t {
	struct scr_coord_t coord;
	struct scr_size_t size;
};

/**
 * View structure.
 *   @buf: The buf.
 *   @box: The target box.
 */

struct scr_view_t {
	struct scr_buf_t *buf;

	struct scr_box_t box;
};


/**
 * Color enumerator.
 *   @scr_black_e: Black.
 *   @scr_red_e: Red.
 *   @scr_green_e: Green.
 *   @scr_yellow_e: Yellow.
 *   @scr_blue_e: Blue.
 *   @scr_magenta_e: Magenta.
 *   @scr_cyan_e: Cyan.
 *   @scr_white_e: White.
 *   @scr_default_e: Default color.
 */

enum scr_color_e {
	scr_black_e = 0,
	scr_red_e = 1,
	scr_green_e = 2,
	scr_yellow_e = 3,
	scr_blue_e = 4,
	scr_magenta_e = 5,
	scr_cyan_e = 6,
	scr_white_e = 7,
	scr_default_e = 9,
};


/**
 * Property structure.
 *   @fg, bg: The foreground and background color.
 *   @bold, underline, neg: Bold, underline, and negative flag.
 */


struct scr_prop_t {
	enum scr_color_e fg, bg;
	bool bold, underline, neg;
};

/**
 * Point structure.
 *   @code: The text code.
 *   @prop: Property set.
 */

struct scr_pt_t {
	uint32_t code;
	struct scr_prop_t prop;
};

/**
 * Determine if two properties are equal.
 *   @prop1: First property.
 *   @prop2: Second property.
 *   &returns; True if equal, false otherwise.
 */

static inline bool scr_prop_isequal(struct scr_prop_t prop1, struct scr_prop_t prop2)
{
	return (prop1.fg == prop2.fg) && (prop1.bg == prop2.bg) && (prop1.bold == prop2.bold) && (prop1.underline == prop2.underline) && (prop1.neg == prop2.neg);
}

/**
 * Determine if two points are equal.
 *   @pt1: First point.
 *   @pt2: Second point.
 *   &returns: True if equal, false otherwise.
 */

static inline bool scr_pt_isequal(struct scr_pt_t pt1, struct scr_pt_t pt2)
{
	return (pt1.code == pt2.code) && scr_prop_isequal(pt1.prop, pt2.prop);
}


/**
 * Special key enumerator.
 *   @scr_esc_e: Escape.
 *   @scr_ctrl_e: Control modifier.
 *   @scr_up_e: Up.
 *   @scr_down_e: Down.
 *   @scr_right_e: Right.
 *   @scr_left_e: Left.
 *   @scr_backspace_e: Backspace.
 *   @scr_delete_e: Delete.
 *   @scr_tab_e: Tab.
 *   @scr_rtab_e: Reverse tab.
 */

enum scr_key_e {
	scr_esc_e = '\x1B',
	scr_ctrl_e = 0x40000000,
	scr_up_e = -0x10000,
	scr_down_e = -0x10001,
	scr_right_e = -0x10002,
	scr_left_e = -0x10003,
	scr_backspace_e = -0x10010,
	scr_delete_e = -0x10011,
	scr_tab_e = '\t',
	scr_rtab_e = -0x10012
};


/**
 * Control signal enumerator.
 *   @scr_propget_e: Retrieve the property.
 *   @scr_propset_e: Set the property.
 */

enum scr_ctrl_e {
	scr_propget_e = 0xb1ca0000,
	scr_propset_e = 0xb1ca0001,
};

/* %~scr.h% */

/*
 * end header: scr.h
 */

#endif
