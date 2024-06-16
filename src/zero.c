#define ZERO_VERSION "0.0"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Keywords
#define Z_TRUE "true"
#define Z_FALSE "false"
#define Z_NULLSET "⦰"
#define Z_WHILE "while"
#define Z_WRAP_LIST "[]"
#define Z_WRAP_SET "()"
#define Z_WRAP_MAP "{}"
#define Z_SCOPE_OPENER "[({"
#define Z_SCOPE_CLOSER "})]"
#define Z_MEANINGLESS_CHARS " \t\n"

static char * depth_hack = "\t\t\t\t\t\t\t\t\t";

// abstract syntax item
struct asi;

struct asi {
    struct asi * children; // optional
    size_t child_count;
    char * token; // optional, probably not null terminated
    size_t token_length;
};

void debug_print_recurse(struct asi * item, size_t depth) {
    if (item->token != NULL)
        printf("%.*s%.*s\n", depth, depth_hack, item->token_length, item->token);
    else
        printf("<EMPTY TOKEN>\n");
    for (size_t i = 0; i < item->child_count; i++) {
        debug_print_recurse(item->children + i, depth + 1);
    }
}

char get_scope_closer(char opener) {
    if (opener == Z_WRAP_LIST[0]) return Z_WRAP_LIST[1];
    if (opener == Z_WRAP_SET[0]) return Z_WRAP_SET[1];
    if (opener == Z_WRAP_MAP[0]) return Z_WRAP_MAP[1];
    return '\0';
}

// Returns the new child count of root
int zero_parse_scope(struct asi * root) {
    if (root->child_count == 0)
        return 0;
    
    // Push inner scooes to their own part of the heap
    size_t depth_inc_index = 0;
    size_t depth = 0;
    for (size_t i = 0; i < root->child_count; i++) {
        struct asi it = root->children[i];

        if (it.token == NULL || it.token_length == 0)
            continue;

        if (strchr(Z_SCOPE_OPENER, it.token[0])) {
            if (depth == 0)
                depth_inc_index = i;
            depth++;
        }

        if (strchr(Z_SCOPE_CLOSER, it.token[0])) {
            // this may need to be strcmp for multichar closer tokens
            assert(depth > 0);
            assert(it.token[0] == get_scope_closer(root->children[depth_inc_index].token[0]));

            if (--depth == 0) {
                while (--i > depth_inc_index) {
                    struct asi swap = root->children[i];
                    size_t j = i;
                    while (j < root->child_count) {
                        root->children[j] = root->children[j+1]; j++;
                    }
                    root->child_count -= 1;
                    root->children[root->child_count] = swap;
                    root->children[depth_inc_index].child_count += 1;
                }

                root->children[depth_inc_index].children = root->children + root->child_count;
                i++; // We have already parsed token i
            }
        }
    }

    // Lastly, scope the children recursively.
    for (size_t i = 0; i < root->child_count; i++)
        zero_parse_scope(root->children + i);

    return root->child_count;
}

int zero_tokenize(char * input) {
    struct asi root;

    // This is a very liberal allocation of the maximum number of tokens input could contain. In the future we should try and estimate it.
    struct asi * child_heap = malloc((sizeof root) * strlen(input));
    root.children = child_heap;
    root.child_count = 0;
    root.token = NULL; // no nullptr til C23 so we'll use NULL as standard

    size_t token_offset = 0;
    for (size_t i = 0; i < strlen(input); i++) {
        if (strchr(Z_MEANINGLESS_CHARS, input[i])) // todo this is inefficient
            token_offset++;

        if (input[i] == ';' || strchr(Z_SCOPE_OPENER, input[token_offset])) {
            child_heap[root.child_count].children = NULL;
            child_heap[root.child_count].child_count = 0;
            child_heap[root.child_count].token = input + token_offset;
            child_heap[root.child_count].token_length = i - token_offset + 1;
            token_offset = i + 1;
            root.child_count++;
        }
    }

    zero_parse_scope(&root);

    debug_print_recurse(&root, 0);

    printf("Children parsed: %d\n", root.child_count);

    free(child_heap);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1) return zero_tokenize(argv[1]);
    return 1;
}
