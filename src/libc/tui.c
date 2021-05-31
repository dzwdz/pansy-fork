#include <tui.h>
#include <tty.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdio.h>

static struct termios orig_termios;
bool cursor_visible = true;

static void disable_raw_mode(void) {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enable_raw_mode(void) {
	tcgetattr(STDIN_FILENO, &orig_termios);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void get_window_size(int *height, int *width) {
	struct winsize ws;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

	if (height != NULL)
		*height = ws.ws_row ? ws.ws_row : 25;
	if (width != NULL)
		*width = ws.ws_col ? ws.ws_col : 80;
}

static void save_cursor(void) {
	write(STDOUT_FILENO, "\033[s", 3);
}

static void restore_cursor(void) {
	write(STDOUT_FILENO, "\033[u", 3);
}

/***************************************/

void tui_init(void) {
	enable_raw_mode();
	tui_set_cursor(0, 0);
	//get_window_size(&stdscr.height, &stdscr.width);
    // ^ we don't have a stdscr yet
}

void tui_shutdown(void) {
	disable_raw_mode();
	tui_set_cursor(0, 0);
    clear_screen();
	if (!cursor_visible)
		tui_show_cursor();
}

int tui_width(void) {
	int width;
	get_window_size(NULL, &width);
	return width;
}

int tui_height(void) {
	int height;
	get_window_size(&height, NULL);
	return height;
}

void
tui_hide_cursor(void)
{
	cursor_visible = false;
	write(STDOUT_FILENO, "\033[?25l", 6);
}

void
tui_show_cursor(void)
{
	cursor_visible = true;
	write(STDOUT_FILENO, "\033[?25h", 6);
}

void tui_set_cursor(int x, int y) {
	/* It looks like x and y has to be 1-indexed, so correct this by adding 1 to
	 * each of them. */
	printf("\033[%d;%dH", y+1, x+1);
}
