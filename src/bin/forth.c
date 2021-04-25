/* NOTES:
 * pansy linux forth has several restrictions since opfez is quite lazy.
 *  - Stuff that usually is multiline in other forth systems, i.e. word
 *    declarations with ':', printing text with '."' and creating comments with
 *    '(' and ')'.
 *  - There's no variables (yet).
 *  - When redifining a word, the old code is overwritten by the new
 *    declaration. This means we do not support stuff like the 'forget' word in
 *    other forth systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include "fs.h"
#include "tty.h"
#include "utility.h"

#define MAX_LEN 256
#define STACK_SIZE 300

/* stolen from sh.c */
char **split_args(const char* str) {
    /* shared_buf is static in sh.c since our malloc isn't properly made yet.
     * this doesn't work in forth since some words may consist of other forth
     * words which also has to be split up. following the static approach would
     * mean making separate split functions for every single "recursion", which
     * is impossible/would restrict the abstraction of forth words.
     */
    // TODO: Fix malloc/free!!!
    void *shared_buf = malloc(64 * sizeof(char*) + MAX_LEN + 64);

    char** parts_start = shared_buf;
    char** parts       = parts_start;

    char* buf_start    = shared_buf + 64 * sizeof(char*);
    char* buf          = buf_start;

    while (*str != '\0') {
        switch (*str) {
            // WHITESPACE
            case ' ':
            case '\n':
            case '\t':
                // ignore repeating whitespace
                if (buf == buf_start) break;

                *buf++ = '\0';
                *parts++ = buf_start;
                buf_start = buf;
                break;

            // ESCAPED CHARS
            case '\\': {
                char c = *++str;
                switch (c) {
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;
                    case 'n': c = '\n'; break;
                    case '0': c = '\0'; break;
                }
                *buf++ = c;
                break;
            }

            default:
                *buf++ = *str;
        }
        str++;
    }

    if (buf != buf_start)  {
        *buf++ = '\0';
        *parts++ = buf_start;
    }

    *parts = NULL;
    return parts_start;
}

/* global stuff */
int stack[STACK_SIZE] = {0};  // TODO: variable size stack
int *sp = stack; // stack pointer

/* essential functions */
void push(int n) {
    if (sp == stack + STACK_SIZE) {
        puts("stack overflow!");
        return; // TODO: should error out loudly, stopping execution
    }
    *sp++ = n;
}

int pop() {
    if (sp == stack) {
        puts("stack underflow!");
        return 0; // TODO: don't return 0
                  // should error out loudly, stopping execution
    }
    return *--sp;
}

/* dictionary */
typedef struct word word;
struct word {
    char name[64]; // TODO: variable size names
    bool c_code;
    char **(* c_func)(); // NULL if c_code == false
    char *forth_code; // NULL if c_code == true
    word *next; // NULL if this is the last word in the dictionary
};

void free_dictionary(word *w) {
    if (w == NULL)
        return;
    if (!w->c_code)
        free(w->forth_code);
    word *next_word = w->next;
    free(w);
    if (next_word != NULL)
        free_dictionary(next_word);
}

void add_word(word *dictionary, word w) {
    word *new_word = malloc(sizeof(word));
    word *traverse_word = dictionary;
    while (1) {
        if (traverse_word->next == NULL) {
            strcpy(new_word->name, w.name);
            new_word->c_code = w.c_code;
            new_word->c_func = w.c_func;
            new_word->forth_code = w.forth_code;
            new_word->next = NULL;
            traverse_word->next = new_word;
            return;
        } else {
            traverse_word = traverse_word->next;
        }
    }
}

/* basic words written in C */

/* Words are defined with a custom macro, worddef.
 * These words have to return tokens, since args in eval() is assigned to the
 * return value of these functions. This is because some words, like :, does
 * their own reading, which consumes some args we don't want to get used again
 * in eval.
 */
#define worddef(name) char** name(word *dict __attribute__((unused)), char** tokens __attribute__((unused)))

worddef(print_pop) {
    printf("%d ", pop());

    return tokens;
}

worddef(add) {
    push(pop() + pop());

    return tokens;
}

worddef(subtract) {
    int x = pop();
    push(pop() - x);

    return tokens;
}

worddef(multiply) {
    push(pop() * pop());

    return tokens;
}

worddef(divide) {
    int x = pop();
    push(pop() / x);

    return tokens;
}

worddef(modulo) {
    int x = pop();
    push(pop() % x);

    return tokens;
}

worddef(divmod) {
    int x = pop();
    int y = pop();
    push(y % x);
    push(y / x);

    return tokens;
}

worddef(morethan) {
    push(pop() < pop() ? -1 : 0);

    return tokens;
}

worddef(lessthan) {
    push(pop() > pop() ? -1 : 0);

    return tokens;
}

worddef(equal) {
    push(pop() == pop() ? -1 : 0);

    return tokens;
}

/* a lot of the following can probably be moved when we add variables to the interpreter */

worddef(swap) {
    int x = pop();
    int y = pop();
    push(x);
    push(y);

    return tokens;
}

worddef(two_swap) {
    int x = pop();
    int y = pop();
    int x2 = pop();
    int y2 = pop();

    push(y);
    push(x);
    push(y2);
    push(x2);

    return tokens;
}

worddef(duplicate) {
    int x = pop();
    push(x);
    push(x);

    return tokens;
}

worddef(two_duplicate) {
    int x = pop();
    int y = pop();

    push(y);
    push(x);
    push(y);
    push(x);

    return tokens;
}

worddef(over) {
    int x = pop();
    int y = pop();

    push(y);
    push(x);
    push(y);

    return tokens;
}

worddef(two_over) {
    int x = pop();
    int x2 = pop();
    int y = pop();
    int y2 = pop();

    push(y2);
    push(y);
    push(x2);
    push(x);
    push(y2);
    push(y);

    return tokens;
}

worddef(drop) {
    pop();
    
    return tokens;
}

worddef(rotate) {
    int x = pop();
    int y = pop();
    int z = pop();

    push(y);
    push(x);
    push(z);

    return tokens;
}

// TODO: allow multiple line word declarations
worddef(colon) {
    word tmp = {"", false, NULL, NULL, NULL};

    tokens++;
    strcpy(tmp.name, *tokens++);
    tmp.forth_code = malloc(MAX_LEN);  // TODO: variable width new words

    char *buf = malloc(MAX_LEN);
    strcpy(buf, *tokens++); // initial token
    while (1) {
        // again, this should be removed for when multiple line word declarations are added.
        // also shouldn't crash the interpreter
        if (*tokens == NULL) {
            puts("error: unexpected new line");
            exit(1);
        }
        if (strcmp(*tokens, ";") == 0)
            break;
        buf = strcat(buf, " ");
        buf = strcat(buf, *tokens);

        tokens++;
    }

    word *traverse_word = dict;
    while (traverse_word != NULL) {
        if (strcmp(traverse_word->name, tmp.name) == 0) {
            /* word already exists, just modify its code and return */
            traverse_word->c_code = false;
            traverse_word->c_func = NULL;
            if (traverse_word->forth_code == NULL)
                traverse_word->forth_code = malloc(MAX_LEN);
            strcpy(traverse_word->forth_code, buf);

            return tokens;
        }
        else {
            traverse_word = traverse_word->next;
        }
    }

    /* word doesn't exist, add a new */
    strcpy(tmp.forth_code, buf);
    add_word(dict, tmp);
    free(buf);

    return tokens;
}

worddef(print) {
    char buf[MAX_LEN] = {0};
    char *outptr = buf;
    tokens++;
    strcpy(outptr, *tokens);

    if ((*tokens == NULL) || ((*(tokens - 1))[strlen(*(tokens - 1)) - 1] == '\"')) {
        goto print_copy_done;
    }

    tokens++;
    while (1) {
        outptr = strcat(outptr, " ");
        outptr = strcat(outptr, *tokens);
        if ((*tokens)[strlen(*tokens) - 1] == '\"')
            break;
        tokens++;
    }

print_copy_done:
    buf[strlen(buf) - 1] = '\0';
    printf("%s ", buf);

    return tokens;
}

worddef(emit) {
    putchar(pop());

    return tokens;
}

worddef(dump_dictionary) {
    word *traverse_word = dict;
    while (1) {
        if (traverse_word == NULL) {
            return tokens;
        } else {
            printf("%s - %s\n",
                   traverse_word->name,
                   traverse_word->c_code ? "C function" : traverse_word->forth_code);
            traverse_word = traverse_word->next;
        }
    }

    return tokens;
}

worddef(show_stack) {
    int elems = sp - stack;
    printf("<%d> ", elems);
    for (int i = 0; i < elems; i++) {
        printf("%d ", stack[i]);
    }

    return tokens;
}

worddef(noop) {
    return tokens;
}

/* end of word declarations */

#define c_word(dictionary, name, func)  add_word(dictionary, (word){name, true, func, NULL, NULL});

word *init_c_words() {
    word *w = malloc(sizeof(word));
    strcpy(w->name, ".");
    w->c_code = true;
    w->c_func = &print_pop;
    w->forth_code = NULL;
    w->next = NULL;

    /*** words defined in C ***/
    /* math */
    c_word(w, "+", &add);
    c_word(w, "-", &subtract);
    c_word(w, "*", &multiply);
    c_word(w, "/", &divide);
    c_word(w, "mod", &modulo);
    c_word(w, "/mod", &divmod);
    c_word(w, ">", &morethan);
    c_word(w, "<", &lessthan);
    c_word(w, "=", &equal);
    /* stack manipulation and information */
    c_word(w, "swap", &swap);
    c_word(w, "2swap", &two_swap);
    c_word(w, "dup", &duplicate);
    c_word(w, "2dup", &two_duplicate);
    c_word(w, "over", &over);
    c_word(w, "2over", &two_over);
    c_word(w, "drop", &drop);
    c_word(w, "rot", &rotate);
    c_word(w, ".s", &show_stack);
    /* i/o */
    c_word(w, ".\"", &print);
    c_word(w, "emit", &emit);
    /* meta */
    c_word(w, ":", &colon);
    c_word(w, ";", &noop);
    c_word(w, "(", &noop);
    c_word(w, ")", &noop);
    c_word(w, "bye", &noop);
    c_word(w, "dump", &dump_dictionary);

    return w;
}

/* central functions */
char *words_exist(char **args, word *dictionary) {
    bool printing_string = false;
    bool in_comment = false;

    while (*args) {
        bool found_word = false;

        if (is_number(*args) || in_comment) {
            found_word = true;
        } else if (printing_string) {
            if ((*args)[strlen(*args) - 1] == '"')
                printing_string = false;
            found_word = true;
        } else if ((*(args - 1) != NULL) && // bounds checking
                   (strcmp(*(args - 1), ":") == 0)) {
            /* this token is a new word, of course it doesn't exist yet */
            found_word = true;
        } else if (!strcmp(*args, "(") || !strcmp(*args, ")")) {
            in_comment = !in_comment;
            found_word = true;
        } else if (!strcmp(*args, ".\"")) {
            printing_string = true;
            found_word = true;
        } else {
            word *traverse_word = dictionary;
            while (traverse_word != NULL) {
                if (strcmp(traverse_word->name, *args) == 0) {
                    found_word = true;
                    break;
                } else {
                    traverse_word = traverse_word->next;
                }
            }
        }
        if (!found_word)
            return *args;
        args++;
    }
    return NULL;
}

int eval(char **args, word *dictionary) {
    char *nonexistent = words_exist(args, dictionary);
    if (nonexistent != NULL) {
        printf("word not found in dictionary: %s\n", nonexistent);
        return 1;
    }

    while (*args) {
        if (is_number(*args)) {
            push(atoi(*args));
        } else if (!strcmp("bye", *args)) {
            return 2;
        } else {
            word *traverse_word = dictionary;
            while (traverse_word != NULL) {
                if (strcmp(traverse_word->name, *args) == 0) {
                    if (traverse_word->c_code) {
                        args = traverse_word->c_func(dictionary, args);
                    } else {
                        char **fargs = split_args(traverse_word->forth_code);
                        eval(fargs, dictionary);
                        free(fargs);
                     }
                    break;
                } else {
                    traverse_word = traverse_word->next;
                }
            }
        }
        args++;
    }
    return 0;
}

void cleanup(word *dictionary) {
    free_dictionary(dictionary);
}

int execute(char *buf, word *dictionary, bool silent) {
    char **args = split_args(buf);

    /* main execution */
    int code = eval(args, dictionary);

    free(args);

    switch (code) {
        /* successful execution */
    case 0:
        if (!silent)
            puts("ok");
        break;
        /* 'bye' word encountered */
    case 2:
        if (!silent)
            puts("see you later!");
        /* fallthrough */
        /* disastrous error */
    case -1:
        return code;
        break;
        /* other error, don't show 'ok' */
    default:
        break;
    }

    return 0;
}

void run_file(int fd, word *dictionary) {
    char c;
    size_t i = 0;
    char buf[MAX_LEN] = {0};
    while (1) {
        int status = read(fd, &c, 1);
        buf[i++] = c;
        if (status == 0) { // EOF
            execute(buf, dictionary, true);
            for (int j = 0; j < MAX_LEN; j++)
                buf[j] = '\0';

            break;
        } else if (c == '\n') {
            execute(buf, dictionary, true);
            for (int j = 0; j < MAX_LEN; j++)
                buf[j] = '\0';
            i = 0;
        } 
    }
}

int main(int argc __attribute__((unused)),
         char *argv[] __attribute__((unused))) {
    word *dictionary = init_c_words();

    /* header */
    puts("pansy linux forth");

    char buf[MAX_LEN] = {0};

    const char *startup_file = "/etc/forth/startup.fs";
    if (file_exists(startup_file)) {
        int input_file = -1;
        input_file = open(startup_file, O_RDONLY);
        if (input_file < 0)
            puts("failed to open startup file");
        else
            run_file(input_file, dictionary);
        close(input_file);
    }

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            int input_file = -1;
            input_file = open(argv[i], O_RDONLY);
            if (input_file < 0) {
                puts("failed to open file!");
                return 1;
            } else {
                run_file(input_file, dictionary);
            }
            close(input_file);
        }
    }
    while (1) {
        readline(buf, MAX_LEN);
        
        if (buf[0] == '\0')
            continue;

        int status = execute(buf, dictionary, false);
        if (status) {
            cleanup(dictionary);
            return status == 2 ? 0 : status;
        }
    }
}
