#include <glib.h>
#include <parse_tree.h>

GHashTable* files;
tree tr;

struct pars_obj {
	char* input;
	unsigned int in_size;
	tree tr;
};

char* grammar;
char* map_file_to_str (char* pathname, int* size);

int in_size;
char* input;

int parse_file(char* file_name);
