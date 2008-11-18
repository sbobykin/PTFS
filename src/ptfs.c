/*
    This file is part of PTFS.

    PTFS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/* 
 Date: 2008-11-14.

 This file is created using three sources by Stanislav Bobykin.
 
 - Parser related code is derived from "AMMUNITION/earlyey.c" of 
   cocom distribution version 0.996. 
   Web site of cocom project: http://cocom.sourceforge.net
 
 - FUSE related code is derived from 
   http://fuse.sourceforge.net/helloworld.html
   http://apps.sourceforge.net/mediawiki/fuse/index.php?title=Hello_World_%28fuse_opt.h%29
 
 - File reading (mmap) code is drived from 
   http://beej.us/guide/bgipc/output/html/multipage/mmap.html
*/

#include "earley.h"
#include "objstack.h"
#include "pt.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#define FUSE_USE_VERSION 25
#include <errno.h>
#include <fuse.h>

const char* program_name = "ptfs";

static struct options {
	char* grammar;
	char* input;
	int help;
	int version;
} options;

#define PTFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }

static struct fuse_opt ptfs_opts[] = {
	PTFS_OPT_KEY("-h", help, 1),
	PTFS_OPT_KEY("-help", help, 1),
	PTFS_OPT_KEY("-v", version, 1),
	PTFS_OPT_KEY("-version", version, 1),
	PTFS_OPT_KEY("-g %s", grammar, 0),
	PTFS_OPT_KEY("--grammar=%s", grammar, 0),
	PTFS_OPT_KEY("-i %s", input, 0),
	PTFS_OPT_KEY("--input=%s", input, 0),
	FUSE_OPT_END
};

static const char *description;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

static int ntok;
static os_t mem_os;

static char* input;
static int input_len;

void print_usage(FILE *F, int exit_code)
{
	fprintf(F,"\
Usage: %s -g GRAMMARFILE -i INPUTFILE  mountpoint\n\
  -h, --help			Print help\n\
  -v, --version			Print version\n\
  -g, --grammar=GRAMMARFILE	Set file name with a grammar\n\
  -i, --input=INPUTFILE		Set file name with an input text\n", 
    program_name);
	
	exit(exit_code);
}

void print_version()
{
	printf("ptfs version 0.0.1.0\n");
}



static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	int len = strlen(path);
	if (strcmp(path + len - 4, "text") == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = ntok;
	}
	else
		stbuf->st_mode = S_IFDIR | 0755;


	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	int i;

	(void) offset;
	(void) fi;

	char* tok;
	struct pt_node* cur_node = pt_root_node;
	char str[2048];

	tok = strtok(path, "/");
	while(tok) {
		for(i = 0; i < cur_node->childs_num; i++) {
			if( strcmp(tok+5, cur_node->childs[i]->repr) == 0 ) {
				cur_node = cur_node->childs[i];
				break;
			}
		}
		tok = strtok(NULL, "/");
	}


	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, "text", NULL, 0);
	for(i = 0; i < cur_node->childs_num; i++) {
		if(cur_node->childs[i]->type)
			snprintf(str+i, 2048, "/%.2d_T_%s", i, 
					cur_node->childs[i]->repr);
		else
			snprintf(str+i, 2048, "/%.2d_N_%s", i, 
					cur_node->childs[i]->repr);

		filler(buf, str+i+1, NULL, 0);
	}


	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	len = ntok;
	if (offset < len) {
		if (offset + size > len)
	    		size = len - offset;

		memcpy(buf, input + offset, size);
	} 
	else
		size = 0;

	return size;
}


static struct fuse_operations hello_oper = {
	.getattr = hello_getattr,
	.readdir = hello_readdir,
	//.open	= hello_open,
	.read	= hello_read,
};

static char* map_file_to_str (char* pathname)
{
	int fd, offset;
	char *data;
	struct stat sbuf;

	if ((fd = open(pathname, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}

	if (stat(pathname, &sbuf) == -1) {
		perror("stat");
		exit(1);
	}
	
	if ((data = mmap((caddr_t)0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (caddr_t)(-1)) {
		perror("mmap");
		exit(1);
	}

	return data;
}

static int ptfs_read_token (void **attr)
{
	*attr = NULL;
	char tok;

	if (input[ntok+1] != '\0')
		tok = input[ntok];
	else
		tok = -1;
	ntok++;
	return tok;
}

static void
test_syntax_error (int err_tok_num, void *err_tok_attr,
		   int start_ignored_tok_num, void *start_ignored_tok_attr,
		   int start_recovered_tok_num, void *start_recovered_tok_attr)
{
  if (start_ignored_tok_num < 0)
    fprintf (stderr, "Syntax error on token %d\n", err_tok_num);
  else
    fprintf
      (stderr,
       "Syntax error on token %d:ignore %d tokens starting with token = %d\n",
       err_tok_num, start_recovered_tok_num - start_ignored_tok_num,
       start_ignored_tok_num);
}

static void *
test_parse_alloc (int size)
{
  void *result;

  OS_TOP_EXPAND (mem_os, size);
  result = OS_TOP_BEGIN (mem_os);
  OS_TOP_FINISH (mem_os);
  return result;
}


int main(int argc, char** argv)
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	memset(&options, 0, sizeof(struct options));
	if (fuse_opt_parse(&args, &options, ptfs_opts, NULL) == -1)
		print_usage(stderr, -1);
	
	if(options.grammar && options.input) {
		/* read grammar */
		description = map_file_to_str(options.grammar);
		/* read input */
		input = map_file_to_str(options.input);
		ntok = 0;
	}
	else
		print_usage(stderr, -1);


	/* prepare to parsing */
	pt_node = malloc(sizeof(struct pt_node));
	pt_node->parent = NULL;
	pt_root_node = pt_node;
	OS_CREATE(pt_pos, 0);

	struct grammar *g;
	struct earley_tree_node *root;
	int ambiguous_p;
	
	OS_CREATE (mem_os, 0);

	if ((g = earley_create_grammar ()) == NULL) {
		fprintf (stderr, "earley_create_grammar: No memory\n");
		OS_DELETE (mem_os);
		exit (1);
	}
	
	earley_set_one_parse_flag (g, FALSE);	
	earley_set_debug_level (g, 0);

	if (earley_parse_grammar (g, TRUE, description) != 0) {
		fprintf (stderr, "%s\n", earley_error_message (g));
		OS_DELETE (mem_os);
		exit (1);
	}


	if (earley_parse (g, ptfs_read_token, test_syntax_error, 
				test_parse_alloc, NULL, &root, &ambiguous_p)) {
		fprintf (stderr, "earley_parse: %s\n", 
				earley_error_message (g));
		exit(-1);
	}

	fuse_main(args.argc, args.argv, &hello_oper);
	
	fuse_opt_free_args(&args);
	
	return 0;
}
