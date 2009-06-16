#include <glib.h>
#include <parse_tree.h>

#define isDir 1
#define isText 2
#define unknown 0

GHashTable* files;
tree tr;

struct pars_obj {
	char* input;
	unsigned int in_size;
	tree tr;
};

char* grammar;
char* map_file_to_str (char* pathname, int* size);
int parse_file(char* file_name);
int get_node(const char* path, tree* ret_tr, struct pars_obj** ret_pars_obj);
