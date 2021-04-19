#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tty.h"
#include "utility.h"

#define MAX_LEN 256
#define STACK_SIZE 300

/* forth.c
 * a forth interpreter for pansy linux
 */

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
int stack[STACK_SIZE] = {0};  // todo: variable size stack
int *sp = stack; // stack pointer

/* essential functions */
void push(int n) {
	if (sp == stack + STACK_SIZE) {
		puts("stack overflow!");
		return; // todo: should error out loudly, stopping execution
	}
	*sp++ = n;
}

int pop() {
	if (sp == stack) {
		puts("stack underflow!");
		return 0; // todo: don't return 0
		          // should error out loudly, stopping execution
	}
	return *--sp;
}

/* dictionary */
typedef struct word word;
struct word {
	char name[64]; // todo: variable size names
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

worddef(dup) {
	int x = pop();
	push(x);
	push(x);

	return tokens;
}

// todo: allow multiple line word declarations
worddef(colon) {
	word tmp = {"", false, NULL, NULL, NULL};

	tokens++;
	strcpy(tmp.name, *tokens++);
	tmp.forth_code = malloc(MAX_LEN);  // todo: variable width new words

	char *buf = malloc(MAX_LEN);
	strcpy(buf, *tokens++); // initial token
	for (; strcmp(*tokens, ";") != 0; tokens++) {
		// again, this should be removed for when multiple line word declarations are added.
		if (tokens == NULL) {
			puts("error: unexpected new line");
			exit(1);
		}
		buf = strcat(buf, " ");
		buf = strcat(buf, *tokens);
	}

	strcpy(tmp.forth_code, buf);

	add_word(dict, tmp);

	return tokens;
}

worddef(dump_dictionary) {
	word *traverse_word = dict;
	while (1) {
		if (traverse_word->next == NULL) {
			return tokens;
		} else {
			puts(traverse_word->name);
			traverse_word = traverse_word->next;
		}
	}

	return tokens;
}

worddef(show_stack) {
	int elems = sp - stack;
	printf("<%d> ", elems);
	for (int i = elems; i > 0; i--) {
		printf("%d ", stack[i-1]);
	}

	return tokens;
}

word *init_dictionary() {
	word *w = malloc(sizeof(word));
	strcpy(w->name, ".");
	w->c_code = true;
	w->c_func = &print_pop;
	w->forth_code = NULL;
	w->next = NULL;

	/* words defined in C */
	add_word(w, (word){"+", true, &add, NULL, NULL});
	add_word(w, (word){"-", true, &subtract, NULL, NULL}); // todo: can't use - as identifier
	add_word(w, (word){"*", true, &multiply, NULL, NULL});
	add_word(w, (word){".s", true, &show_stack, NULL, NULL});
	add_word(w, (word){"dup", true, &dup, NULL, NULL});
	add_word(w, (word){":", true, &colon, NULL, NULL});
	add_word(w, (word){"dump", true, &dump_dictionary, NULL, NULL});

	/* words defined in Forth */
	word tmp = {"", false, NULL, NULL, NULL}; // template word used for defining forth words

	return w;
}

#ifdef DEBUG
void print_words(word *w) {
	printf("%s\n", w->name);
	if (w->next == NULL) {
		printf("NULL\n");
		return;
	}
	print_words(w->next);
}
#endif /* DEBUG */

/* central functions */
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
						args = traverse_word->c_func(dictionary, args);
					} else {
						char **fargs = split_args(traverse_word->forth_code);
						eval(fargs, dictionary);
						free(fargs);
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

	puts("pansy linux forth");
	puts("pre-alpha");

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
