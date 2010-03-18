#include <glib.h>
#include <parse_tree.h>
#define FUSE_USE_VERSION 25
#include <fuse.h>

#define isDir 1
#define isText 2
#define unknown 0

GHashTable* files;
GHashTable* read_op_hash;

//tree tr;

struct pars_obj {
	GMappedFile* mf;
	char* input;
	char* full_path;
	size_t full_path_size;
	unsigned int in_size;

	staloc_t *s2;
	staloc_t *st;

	tree tr;
};

char* grammar;

size_t fparsed_size;

char* map_file_to_str (char* pathname, int* size);
int parse_file(char* file_name);
void unparse_file(char* file_name);
int get_node(const char* path, tree* ret_tr, struct pars_obj** ret_pars_obj);
unsigned char* compile(char*, int*);


/* fuse operations */
int ptfs_getattr(const char *path, struct stat *stbuf);
int ptfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi);
int ptfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi);
int ptfs_write(const char *path, const char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi);

struct fuse_operations ptfs_ops;

/* Handlers for read and write operations */
static struct rw_op_context {
	char* path;
	char* buf;
	size_t size;
	off_t offset;
};

int ptfs_read_from_parsed(struct rw_op_context* read_op_context);
int ptfs_read_from_text(struct rw_op_context* read_op_context);

int ptfs_write_to_parsed(struct rw_op_context* write_op_context);
int ptfs_write_to_unparse(struct rw_op_context* write_op_context);
int ptfs_write_to_text(struct rw_op_context* write_op_context);
