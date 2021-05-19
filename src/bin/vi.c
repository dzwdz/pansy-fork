#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <tty.h>
#include <unistd.h>

#define NDEBUG

#define ESC 27
#define DEL 127

/* structs, definitions */
struct termios old_config;

enum MODE { NORMAL, INSERT };

struct line {
    char *text;
    int16_t length;
};

struct editor_state {
    enum MODE mode;
    int x;
    int max_x;
    int y;
    int max_y;
    size_t offset;
    struct line *lines;
    char *filename;
};
#define NEW_EDITOR {NORMAL, 0, 0, 0, 0, 0, NULL, NULL}

#define CTRL_KEY(k) ((k) & 0x1f)

/* initialization, exiting */
void initialize(void) {
    tcgetattr(STDIN_FILENO, &old_config);
    struct termios raw = old_config;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void shutdown(struct editor_state *e) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_config);
    e->x = e->y = 0;
}

/* i/o */
void hide_cursor(void) {
    printf("\033[?25l");
}

void show_cursor(void) {
    printf("\033[?25h");
}

void draw_cursor(int x, int y) {
    printf("\033[%d;%dH", y+1, x+1);
}

// TODO: doesn't work on pansy, only on host system (???)
void get_dimensions(int *x, int *y) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

    *x = ws.ws_col;
    *y = ws.ws_row ? ws.ws_row-1 : 0;
}

struct line *load_file(char *filename) {
    int fd = open(filename, O_RDONLY);

    struct line *tb = malloc(sizeof(struct line) * 3000); // TODO: remove 3000 line limit
    char c;
    uint64_t linei, bufi;
    linei = bufi = 0;
    char buf[256] = {0}; // TODO: remove 256 char line length limit

    while (read(fd, &c, 1)) {
        /* note: only handles files properly formatted (ending in '\n') */
        if (c == '\n') {
            char *tmp = malloc(256);
            strcpy(tmp, buf);
            tb[linei].text = tmp;
            tb[linei].length = strlen(buf);
            for (int i = 0; i < 256; i++)
                buf[i] = '\0';
            bufi = 0;
            linei++;
        } else {
            /* the if clause is a bugfix for a situation where we would write
             *'\0' bytes to the file, making it impossible to parse */
            if (c)
                buf[bufi++] = c;
        }
    }
    /* last line is a newline, don't display it */
    tb[linei].length = -1;
    close(fd);
    return tb;
}

struct line *create_file(void) {
    struct line *tb = malloc(sizeof(struct line) * 3000); // TODO: remove 3000 line limit
    char buf[256] = {0};
    tb[0].length = 0;
    char *contents = malloc(256);
    strcpy(contents, buf);
    tb[0].text = contents;
    tb[1].length = -1;

    return tb;
}

bool write_lines(struct editor_state e) {
    creat(e.filename, 0666); // we don't have O_CREAT flag for open
    int fd = open(e.filename, O_WRONLY | O_TRUNC);

    if (fd < 0)
        return false;

    uint64_t bytes_written = 0;

    char newline = '\n';
    for (size_t i = 0; e.lines[i].length != -1; i++) {
        write(fd, e.lines[i].text, e.lines[i].length);
        write(fd, &newline, 1);
        bytes_written += e.lines[i].length + 1;
    }

    close(fd);
    return true;
}

size_t num_lines(struct line *lines) {
    size_t i;
    for (i = 0; lines[i].length != -1; i++);
    return i;
}

void free_lines(struct line *lines) {
    for (uint64_t i = 0; lines[i].length != -1; i++) {
        free(lines[i].text);
    }
    free(lines);
}

void toggle_echoing() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);

    t.c_lflag ^= ECHO;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
}

void display_lines(struct editor_state e) {
    // is this bad style?
    printf("\033[K");
#   define LINE_SUFFIX "\r\n\033[K"
    for (size_t i = 0 + e.offset; i < e.max_y + e.offset; i++) {
        if (e.lines[i].length != -1) {
            printf("%s" LINE_SUFFIX, e.lines[i].text);
            continue;
        } else {
            for (; i < e.max_y + e.offset - 1; i++)
                printf("~" LINE_SUFFIX);
        }
    }
#   undef LINE_SUFFIX
}

int y_pos(struct editor_state e) {
    return e.y + e.offset;
}

/* string handling */
// TODO: no boundary checks in place
void shiftright(char *str, size_t index) {
    size_t i;
    for (i = strlen(str); i > index; i--) {
        str[i] = str[i-1];
    }
    str[i] = ' ';
}

void shiftleft(char *str, size_t index) {
    size_t i;
    for (i = index; i < strlen(str); i++) {
        str[i] = str[i+1];
    }
}

/* input handling */
void goto_eol(struct editor_state *e) {
    e->x = e->lines[y_pos(*e)].length-1;
    if (e->x < 0) e->x = 0;
}

void delete_line(struct editor_state *e, size_t linenum) {
    size_t i;
    for (i = linenum; e->lines[i].length != -1; i++) {
        e->lines[i] = e->lines[i+1];
    }
    free(e->lines[i-1].text);
}

void insert_line(struct editor_state *e, size_t linenum) {
    size_t i = 0;
    while (e->lines[i].length != -1) i++;
    i++;

    for (; i > linenum; i--) {
        e->lines[i] = e->lines[i-1];
    }

    e->lines[linenum].text = malloc(256); // TODO: variable length lines
    char buf[256] = {0};
    strcpy(e->lines[linenum].text, buf);
    e->lines[linenum].length = 0;
}

void next_line(struct editor_state *e) {
    if (e->lines[y_pos(*e)+1].length != -1) {
        if (e->y+1 < e->max_y)
            e->y++;
        else
            e->offset++;
        if (e->x >= e->lines[y_pos(*e)].length)
            goto_eol(e);
    }
}

void prev_line(struct editor_state *e) {
    if (e->y) {
        e->y--;
        if (e->x >= e->lines[y_pos(*e)].length)
            goto_eol(e);
    } else if (y_pos(*e) > 0) {
        e->offset--;
    }
}

void delete_char(struct editor_state *e) {
    /* probably unecessary check */
    if (e->lines[y_pos(*e)].length) {
        shiftleft(e->lines[y_pos(*e)].text, e->x);
        e->lines[y_pos(*e)].length--;
        if (e->x > e->lines[y_pos(*e)].length-1)
            e->x--;
        if (e->x < 0)
            e->x = 0;
    }
}

/* motions */
void d_motion(struct editor_state *e, char c) {
    switch(c) {
    case 'd':
        delete_line(e, y_pos(*e));
        if (e->lines[y_pos(*e)].length == -1 && e->y)
            e->y--;
        if (num_lines(e->lines) == 0)
            insert_line(e, 0);
        if (e->x >= e->lines[y_pos(*e)].length)
            goto_eol(e);

        break;
    case 'k':
        prev_line(e);
        /* fallthrough */
    case 'j':
        delete_line(e, y_pos(*e));
        delete_line(e, y_pos(*e));

        if (e->lines[y_pos(*e)].length == -1)
            prev_line(e);
        break;
    }
}


/* :-menu */
bool parse_cmd(struct editor_state e, const char *c) {
    while (*c != '\0') {
        switch (*c++) {
        case 'w': {
            write_lines(e);
            break;
        }
        case 'q':
            return false;
        }
    }

    return true;
}

/* "keyhandlers" */
bool nm_keyhandler(struct editor_state *e, char key) {
    char c;
    switch (key) {
    case 'I':
        e->x = 0;
        /* fallthrough */
    case 'i':
        e->mode = INSERT;
        break;
    case 'A':
        goto_eol(e);
        /* fallthrough */
    case 'a':
        if (e->lines[y_pos(*e)].length)
            e->x++;
        e->mode = INSERT;
        break;
    case 'h':
        if (e->x) e->x--;
        break;
    case 'l':
        if (e->x+1 < e->lines[y_pos(*e)].length) e->x++;
        break;
    case 'k':
        prev_line(e);
        break;
    case 'j':
        next_line(e);
        break;
    case 'w':
        while ((e->lines[y_pos(*e)].text[e->x] != ' ') &&
               (e->lines[y_pos(*e)].text[e->x] != '\t')) {
            e->x++;
            if (e->x >= e->lines[y_pos(*e)].length) {
                e->x = 0;
                next_line(e);
                goto found_newline;
            }
        }
        e->x++;
    found_newline:
        break;
    case '$':
        goto_eol(e);
        break;
    case '0':
        e->x = 0;
        break;
    case 'x':
        delete_char(e);
        break;
    case 'r':
        read(STDIN_FILENO, &c, 1);
        if (isprint(c) || isspace(c))
            e->lines[y_pos(*e)].text[e->x] = c;
        break;
    case 'D': {
        int chars_to_remove = e->lines[y_pos(*e)].length - e->x;
        for (int i = 0; i < chars_to_remove; i++)
            shiftleft(e->lines[y_pos(*e)].text, e->x);
        e->lines[y_pos(*e)].length -= chars_to_remove;
        goto_eol(e);
        break;}
    case 'd':
        read(STDIN_FILENO, &c, 1);
        d_motion(e, c);
        break;
    case 'o':
        e->y++;
        /* quick hack to make the buffer scroll if we insert a line at the end
         * of the viewable buffer. */
        next_line(e);
        prev_line(e);

        insert_line(e, y_pos(*e));
        e->x = 0;
        e->mode = INSERT;
        break;
    case 'G':
        while (e->lines[y_pos(*e)+1].length != -1)
            next_line(e);
        break;
    case ':':
        draw_cursor(0, e->max_y);
        printf(":");
        char buf[80];
        toggle_echoing();
        readline(buf, 80);
        toggle_echoing();

        return parse_cmd(*e, buf);
    }

    return true;
}

bool im_keyhandler(struct editor_state *e, char key) {
    if (key )
    switch(key) {
    case DEL:
        if (e->x) {
            if (e->x == e->lines[y_pos(*e)].length) {
                e->x--;
                delete_char(e);
                e->x++;
                if (!e->lines[y_pos(*e)].length)
                    e->x--;
            } else {
                e->x--;
                delete_char(e);
            }
        } else if (e->y) {
            delete_line(e, e->y);
            prev_line(e);
            goto_eol(e);
            if (e->lines[y_pos(*e)].length)
                e->x++;
        }
        break;
    case '\r':
        e->y++;
        insert_line(e, y_pos(*e));
        e->x = 0;
        break;
    default:
        /* no special char found, return false signalling it should handle it
         * like text to be inserted */
        return false;
    }

    return true;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        dprintf(STDERR_FILENO, "vi: Missing file operand.\n");
        return 1;
    }

    initialize();

    struct editor_state E = NEW_EDITOR;
    E.filename = argv[1];

    {
        struct stat tmp;

        if (stat(E.filename, &tmp)) { /* file doesn't exist */
            E.lines = create_file();
        } else {
            E.lines = load_file(argv[1]);
        }
    }

    get_dimensions(&E.max_x, &E.max_y);

    if (!E.max_x)
        E.max_x = 80;
    if (!E.max_y)
        E.max_y = 25;

    /* initial draw */
    clear_screen();
    draw_cursor(0, 0);
    display_lines(E);
    draw_cursor(E.x, E.y);

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        /* input */
        if (c == ESC && E.mode == INSERT) {
            E.mode = NORMAL;
            if (E.x > 0)
                E.x--;
        } else if (E.mode == NORMAL) {
            if (!nm_keyhandler(&E, c))
                break;
        } else if (E.mode == INSERT && (isprint(c) || isspace(c))) {
            if (!im_keyhandler(&E, c)) {
                shiftright(E.lines[y_pos(E)].text, E.x);
                E.lines[y_pos(E)].text[E.x] = c;
                E.lines[y_pos(E)].length++;
                E.x++;
            }
        }

        /* drawing */
        hide_cursor();
        draw_cursor(0, 0);
        display_lines(E);

        #ifndef NDEBUG
        draw_cursor(0, 10);
        printf("%c - %d\r\n", c, c);
        printf("x pos: %d\r\ny pos: %d\r\n", E.x, E.y);
        printf("max x: %d\r\nmax y: %d\r\n", E.max_x, E.max_y);
        printf("line length: %d\r\n", E.lines[E.y].length);
        #endif

        draw_cursor(E.x, E.y);
        show_cursor();
    }

    free_lines(E.lines);
    clear_screen();
    shutdown(&E);
}
