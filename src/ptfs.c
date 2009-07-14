/* 
 LICENSE: GNU General Public License version 3 
          as published by the Free Software Foundation.
          See the file 'gpl-3.0.txt' for details.
*/

/* This file contains reuse notes. See the file 'REUSE' for details. */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#define FUSE_USE_VERSION 25
#include <errno.h>
#include <fuse.h>

#include "ptfs.h"

const char* program_name = "ptfs";

struct options {
	char* grammar;
	int help;
	int version;
} options;

static GList* input_files = NULL;

#define PTFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }

/** keys for FUSE_OPT_ options */
enum
{
   KEY_VERSION,
   KEY_HELP,
   KEY_FULL_HELP
};


static struct fuse_opt ptfs_opts[] = {
	PTFS_OPT_KEY("-g %s", grammar, 0),
	PTFS_OPT_KEY("--grammar=%s", grammar, 0),

	FUSE_OPT_KEY("-V",             KEY_VERSION),
	FUSE_OPT_KEY("--version",      KEY_VERSION),
	FUSE_OPT_KEY("-h",             KEY_HELP),
	FUSE_OPT_KEY("--help",         KEY_HELP),
	FUSE_OPT_KEY("-fh",            KEY_FULL_HELP),
	FUSE_OPT_KEY("--fhelp",        KEY_FULL_HELP),

	FUSE_OPT_END
};

void print_usage(FILE *F, int exit_code)
{
	fprintf(F,"\
usage: %s -g GRAMMARFILE [INPUTFILES] MOUNTPOINT\n\
\n\
general options:\n\
    -o opt,[opt...]        mount options\n\
    -h   --help            print short help\n\
    -fh  --fhelp           print full help\n\
    -V   --version         print version\n\
\n\
PTFS options:\n\
    -g, --grammar=GRAMMARFILE	Set file name with a grammar\n\n",
    program_name);

}

void print_version()
{
	fprintf(stderr, "PTFS version 0.0.2.1\n");
}

static int ptfs_opt_proc(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
	(void) data;
	static int is=0;

	switch (key) {
	case FUSE_OPT_KEY_OPT:
		return 1;

	case FUSE_OPT_KEY_NONOPT:
		input_files = g_list_append(input_files, arg);
		return 0;
		
	case KEY_FULL_HELP:
                print_usage(stderr, 1);
		fuse_opt_add_arg(outargs, "-ho");
		fuse_main(outargs->argc, outargs->argv, &ptfs_ops);
		exit(0);
	
	case KEY_HELP:
                print_usage(stderr, 1);
		exit(0);

        case KEY_VERSION:
		print_version();
		fuse_opt_add_arg(outargs, "--version");
		fuse_main(outargs->argc, outargs->argv, &ptfs_ops);
		exit(0);

	default:
		fprintf(stderr, "internal error\n");
		abort();
	}

}


char* map_file_to_str (char* pathname, int* size, GMappedFile** retmf)
{
	GError* error = NULL;
	GMappedFile* mf = g_mapped_file_new(pathname, FALSE, &error);
	char* data = g_mapped_file_get_contents(mf);
	if(!data) {
		if(error)
			fprintf(stderr, "%s\n", error->message);
		else
			fprintf(stderr, 
				"map_file_to_str %s: unkown error\n", 
				pathname);
		return NULL;
	}

	if(size)
		*size = g_mapped_file_get_length(mf); 

	if(retmf)
		*retmf = mf;

	return data;
}

int main(int argc, char** argv)
{
	caml_startup(argv);

	files = g_hash_table_new (g_str_hash, g_str_equal);
	read_op_hash = g_hash_table_new (g_int_hash, g_int_equal);

	fparsed_size = 0;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	memset(&options, 0, sizeof(struct options));
	if (fuse_opt_parse(&args, &options, ptfs_opts, ptfs_opt_proc) == -1)
		print_usage(stderr, -1);

	GList* mount_point = g_list_last (input_files);
	if(options.grammar) {
		/* read grammar */
		grammar = map_file_to_str(options.grammar, NULL, NULL);

		/* read input */
		GList* glist_iter = input_files;
		for(; glist_iter != mount_point; 
		      glist_iter = glist_iter->next) {

			parse_file(glist_iter->data);
		}
	}
	else {
		fprintf(stderr, "no grammar\n");
		fprintf(stderr, "try 'ptfs -h' for usage\n");
		exit(1);
	}

	if(mount_point)
		fuse_opt_add_arg(&args, mount_point->data);

	g_list_free(input_files);
	
	fuse_main(args.argc, args.argv, &ptfs_ops);
	fuse_opt_free_args(&args);
	return 0;
}
