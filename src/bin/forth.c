#include <stdio.h>
#include <stdlib.h> // atoi
#include <string.h>
#include <stdbool.h>
#include "tty.h"
#include "utility.h" // is_number


/* forth.c
 * a forth interpreter for pansy linux
 */


/* global stuff */
#define MAX_LEN 256
#define STACK_SIZE 300
int stack[STACK_SIZE] = {0};  // todo: variable size stack
int *sp = stack; // stack pointer

/* essential functions */
void push(int n) {
	if (sp == stack + STACK_SIZE) {
		puts("stack overflow!");
		return;
	}
	*sp++ = n;
}

int pop() {
	if (sp == stack) {
		puts("stack underflow!");
		return 0;
	}
	return *--sp;
}

/* dictionary */
typedef struct word word;
struct word {
	char name[64]; // todo: variable size names
	bool c_code;
	void (* c_func)(); // NULL if c_code == false
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

void print_pop() {
	printf("%d ", pop());
}

void add() {
	push(pop() + pop());
}

void subtract() {
	int x = pop();
	push(pop() - x);
}

void multiply() {
	push(pop() * pop());
}

void dup() {
	int x = pop();
	push(x);
	push(x);
}

void show_stack() {
	int elems = sp - stack;
	printf("<%d> ", elems);
	for (int i = elems; i > 0; i--) {
		printf("%d ", stack[i-1]);
	}
}

word *init_dictionary() {
	word *w = malloc(sizeof(word));
	strcpy(w->name, ".");
	w->c_code = true;
	w->c_func = &print_pop;
	w->forth_code = NULL;
	w->next = NULL;

	// words defined in C
	add_word(w, (word){"+", true, &add, NULL, NULL});
	add_word(w, (word){"-", true, &subtract, NULL, NULL});
	add_word(w, (word){"*", true, &multiply, NULL, NULL});
	add_word(w, (word){".s", true, &show_stack, NULL, NULL});

	// words defined in Forth
	word tmp = {"", false, NULL, NULL, NULL};

	strcpy(tmp.name, "");
	tmp.forth_code = malloc(64);
	strcpy(tmp.forth_code, "2 3 * .");
	add_word(w, tmp);

	return w;
}

void print_words(word *w) {
	printf("%s\n", w->name);
	if (w->next == NULL) {
		printf("NULL\n");
		return;
	}
	print_words(w->next);
}

// stolen from sh.c
char **split_args(const char* str) {
	// shared_buf is static in sh.c since our malloc isn't properly made yet.
	// this doesn't work in forth since some words may consist of other forth
	// words which also has to be split up. following the static approach would
	// mean making separate split functions for every single "recursion", which
	// is impossible/would restrict the abstraction of forth words.
	// TODO: fix malloc/free!!
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


int eval(char **args, word *dictionary) {
	while (*args) {
		if (is_number(*args)) {
			push(atoi(*args));
		} else {
			word *traverse_word = dictionary;
			bool found_word = false;
			while (traverse_word != NULL) {
				if (strcmp(traverse_word->name, *args) == 0) {
					if (traverse_word->c_code) {
						traverse_word->c_func();
					} else {
						/* puts("evaling forth word..."); */
						char **backup = args;
						char **fargs = split_args(traverse_word->forth_code);
						eval(fargs, dictionary);
						free(fargs);
						args = backup;
					}
					found_word = true;
					break;
				} else {
					traverse_word = traverse_word->next;
				}
			}

			if (!found_word) {
				puts("word not found in dictionary!");
				return 1;
			}
		}
		args++;
	}
	return 0;
}

int main(int argc __attribute__((unused)),
		 char *argv[] __attribute__((unused))) {
	char buf[MAX_LEN];
	word *dictionary = init_dictionary();
	
	while (1) {
		readline(buf, MAX_LEN);
		if (buf[0] == '\0')
			continue;
		char **args = split_args(buf);
		int code;
		if ((code = eval(args, dictionary)) != 0) {
			free(args);
			free_dictionary(dictionary);
			return code;
		}
		puts("ok");
		free(args);
	}
}
