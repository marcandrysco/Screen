#include "../common.h"
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../buf.h"
#include "../iface.h"
#include "../scr.h"


/**
 * Implementation structure.
 *   @input, output: Input and output file descriptors.
 *   @seqi: The sequence index.
 *   @seq: Buffered input sequence.
 *   @attr: Previous terminal attributes.
 *   @buf: The buffer.
 */

struct scr_impl_t {
	int input, output;

	int8_t seqi;
	int32_t seq[3];
	struct termios attr;

	struct scr_buf_t *buf;
};


/*
 * local function declarations
 */

static void impl_delete(struct scr_impl_t *impl);

static int32_t impl_seq(struct scr_impl_t *impl, int32_t *ch, int8_t len);

static int16_t fdread(struct scr_impl_t *impl, int timeout);
static void fdwrite(struct scr_impl_t *impl, const char *restrict format, ...);

static void impl_init();
static void impl_destroy();

static void impl_add(struct scr_impl_t *impl);
static void impl_remove(struct scr_impl_t *impl);

/*
 * local variables
 */

static bool impl_kill = false;
static struct thread_once_t impl_once = THREAD_ONCE_INIT;
static struct thread_mutex_t impl_mutex = THREAD_MUTEX_INIT;
static struct avltree_t impl_set;


/**
 * Open an implementation on an input and output.
 *   @input: The input.
 *   @output: The output.
 *   &returns: The implementation.
 */

_export
struct scr_impl_t *scr_impl_open(struct io_input_t input, struct io_output_t output)
{
	struct termios attr;
	struct scr_impl_t *impl;

	thread_once(&impl_once, impl_init);

	impl = mem_alloc(sizeof(struct scr_impl_t));
	impl->seqi = -1;

	if(input.ref == io_stdin.ref)
		impl->input = STDIN_FILENO;

	if(output.ref == io_stdout.ref)
		impl->output = STDOUT_FILENO;

	tcgetattr(impl->input, &impl->attr);
	attr = impl->attr;
	attr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(impl->input, TCSANOW, &attr);

	fdwrite(impl, "\x1B[?25l");
	fdwrite(impl, "\x1B[?1049h");
	impl->buf = scr_buf_new((struct scr_box_t){ { 0 , 0 }, scr_impl_size(impl) });

	impl_add(impl);

	return impl;
}

/**
 * Close an implementation.
 *   @impl: The implementation.
 */

_export
void scr_impl_close(struct scr_impl_t *impl)
{
	scr_buf_delete(impl->buf);
	impl_remove(impl);
	impl_delete(impl);
}

/**
 * Delete an implementation.
 *   @impl: The implementation.
 */

static void impl_delete(struct scr_impl_t *impl)
{
	tcsetattr(impl->input, TCSANOW, &impl->attr);

	fdwrite(impl, "\x1B[?25h");
	fdwrite(impl, "\x1B[?1049l");

	mem_free(impl);
}


/**
 * Retrieve the next character.
 *   @impl: The implementation.
 *   @timeout: Timeout.
 *   &returns: The character.
 */

_export
int32_t scr_impl_read(struct scr_impl_t *impl, int timeout)
{
	int32_t ch[4] = { 0, 0, 0, 0 };

	if(impl->seqi != -1) {
		ch[0] = impl->seq[impl->seqi++];
		if(ch[0])
			return ch[0];
		else
			impl->seqi = -1;
	}

	ch[0] = fdread(impl, timeout);
	switch(ch[0]) {
	case '\x1B':
		ch[1] = fdread(impl, 10);
		if(ch[1] == '[') {
			ch[2] = fdread(impl, 10);
			switch(ch[2]) {
			case '3':
				ch[3] = fdread(impl, 10);
				switch(ch[3]) {
				case '\x7E': return scr_delete_e;
				default: return impl_seq(impl, ch, 4);
				}

			case 'A': return scr_up_e;
			case 'B': return scr_down_e;
			case 'C': return scr_right_e;
			case 'D': return scr_left_e;
			default: return impl_seq(impl, ch, 3);
			}
		}
		else
			return impl_seq(impl, ch, 2);

	case '\x7F': return scr_backspace_e;
	default: return ch[0];
	}
}

/**
 * Add a set of character to the sequence buffer.
 *   @impl: The implementation.
 *   @ch: The input character set.
 *   @len: The number of character.
 *   &returns: The current character.
 */

static int32_t impl_seq(struct scr_impl_t *impl, int32_t *ch, int8_t len)
{
	uint8_t i;

	impl->seqi = 0;
	for(i = 1; i < len; i++)
		impl->seq[i-1] = ch[i];

	return ch[0];
}


/**
 * Retrieve the implementation size.
 *   @impl: The implementation.
 *   &returns: The size.
 */

_export
struct scr_size_t scr_impl_size(struct scr_impl_t *impl)
{
	struct winsize size;

	ioctl(impl->output, TIOCGWINSZ, &size);

	return (struct scr_size_t){ size.ws_col, size.ws_row };
}

/**
 * Swap buffers.
 *   @impl: The implementation.
 *   @buf: The new buffer.
 */

_export
void scr_impl_swap(struct scr_impl_t *impl, struct scr_buf_t *buf)
{
	unsigned int x, y;
	struct scr_pt_t newpt, oldpt;
	bool skipmove = false, bold = false, underline = false, neg = false;
	unsigned short fg = scr_white_e, bg = scr_black_e;
	struct scr_size_t size = buf->box.size;

	fdwrite(impl, "\x1B[39;49m");
	fdwrite(impl, "\x1B[0m");

	for(y = 0; y < size.height; y++) {
		for(x = 0; x < size.width; x++) {
			newpt = scr_buf_get(buf, (struct scr_coord_t){ x, y });
			oldpt = scr_buf_get(impl->buf, (struct scr_coord_t){ x, y });

			if(newpt.code == '\x1B')
				newpt.code = ' ';

			if(!scr_pt_isequal(newpt, oldpt)) {
				if(newpt.prop.fg != fg)
					fdwrite(impl, "\x1B[3%um", fg = newpt.prop.fg);

				if(newpt.prop.bg != bg)
					fdwrite(impl, "\x1B[4%um", bg = newpt.prop.bg);

				if(newpt.prop.bold != bold)
					fdwrite(impl, "\x1B[%sm", (bold = newpt.prop.bold) ? "1" : "22");

				if(newpt.prop.underline != underline)
					fdwrite(impl, "\x1B[%sm", (underline = newpt.prop.underline) ? "4" : "24");

				if(newpt.prop.neg != neg)
					fdwrite(impl, "\x1B[%sm", (neg = newpt.prop.neg) ? "7" : "27");

				if(!skipmove) {
					fdwrite(impl, "\x1B[%u;%uH", y+1, x+1);
					skipmove = true;
				}

				fdwrite(impl, "%c", newpt.code);
			}
			else
				skipmove = false;
		}
	}

	scr_buf_delete(impl->buf);
	impl->buf = buf;
}


/**
 * Retrieve the next character.
 *   @impl: The implementation.
 *   @timeout: The timeout.
 *   &returns: The character, or end-of-stream on timeout.
 */

static int16_t fdread(struct scr_impl_t *impl, int timeout)
{
	char ch;
	struct pollfd fds[1];

	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	if(poll(fds, 1, timeout) < 1)
		return '\0';

	if(read(impl->input, &ch, 1) < 0)
		return '\0';
	else
		return ch;
}

/**
 * Write the string.
 *   @impl: The implementation.
 *   @str: The string.
 */

static void fdwrite(struct scr_impl_t *impl, const char *restrict format, ...)
{
	size_t rem;
	ssize_t ret;
	va_list args;

	va_start(args, format);
	rem = str_vlprintf(format, args);
	va_end(args);

	{
		char str[rem + 1], *ptr = str;

		va_start(args, format);
		str_vprintf(str, format, args);
		va_end(args);

		while(rem > 0) {
			ret = write(impl->output, ptr, rem);

			rem -= ret;
			ptr += ret;
		}
	}
}


/**
 * Initialize the implementation.
 */

static void impl_init()
{
	sys_atexit(impl_destroy);

	avltree_init(&impl_set, compare_ptr, (delete_f)impl_delete);
}

/**
 * Initialize the implementation.
 */

static void impl_destroy()
{
	thread_mutex_lock(&impl_mutex);

	if(!impl_kill)
		avltree_destroy(&impl_set);

	impl_kill = true;

	thread_mutex_unlock(&impl_mutex);
}

/**
 * Add an implementation.
 *   @impl: The implementation.
 */

static void impl_add(struct scr_impl_t *impl)
{
	thread_mutex_lock(&impl_mutex);
	avltree_insert(&impl_set, impl, impl);
	thread_mutex_unlock(&impl_mutex);
}

/**
 * Remove an implementation.
 *   @impl: The implementation.
 */

static void impl_remove(struct scr_impl_t *impl)
{
	thread_mutex_lock(&impl_mutex);
	avltree_remove(&impl_set, impl);
	thread_mutex_unlock(&impl_mutex);
}
