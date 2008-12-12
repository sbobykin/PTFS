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

#define isDir 1
#define isText 2
#define unknown 0

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

static void map_nodes_to_subtext () 
{
	int next;
	int end;
	struct pt_node* cur_node;
	struct pt_node* cur_tmp_node;
	pt_root_node->b = 0;
	pt_root_node->e = 0;
	cur_node = pt_root_node;

	do {
		if(cur_node->type) {
			//cur_node->b = cur_node->parent->b;
			if(cur_node->type == 1)
				end = cur_node->b + 1;
			else {
				//cur_node->type = 1;
				end = cur_node->b;
			}
			cur_node->e = end;
			//printf("TERM: %s b: %d\n", cur_node->repr, cur_node->b);
			while( cur_node->parent->e == cur_node->parent->childs_num) {

				cur_node = cur_node->parent;
				cur_node->e = end;
				//printf("NTERM_UP, END_POS: %s, %d\n", cur_node->repr, cur_node->e);
				if(cur_node->parent == NULL)
					break;

			}

			if(cur_node->parent != NULL) {
				next = cur_node->parent->e;
				cur_node->parent->e++;
				cur_node = cur_node->parent->childs[next];
				//printf("hi repr: %s\n", cur_node->repr);
				cur_node->b = end;
			}
			//printf("end: %d\n", end);
		}
		else {
			//printf("NTERM: %s, %d\n", cur_node->repr, cur_node->e);
			if(cur_node->childs_num == 0) {
				//printf("ch_num: %d\n", cur_node->childs_num);
				cur_node->childs = malloc( sizeof(struct pt_node*) );
				cur_node->childs[0] = malloc( sizeof(struct pt_node) );
				cur_node->childs[0]->parent = cur_node; 
				cur_node->childs[0]->type = 2;
				cur_node->childs[0]->childs_num = 0;
				cur_node->childs[0]->repr = malloc( 2*sizeof(char) );
				snprintf(cur_node->childs[0]->repr, 2, "e");
				cur_node->childs_num = 1;
			}
			cur_node = cur_node->childs[cur_node->e];
			cur_node->parent->e++;
			//printf("hii\n");
			cur_node->b = cur_node->parent->b;
		}
	} while (cur_node->parent != NULL);
}

int get_node(const char* path, struct pt_node** node)
{
	int i;
	int cur_childs_num;
	int status;
	char symb_pos[3];
	char* tok;
	struct pt_node* cur_node = pt_root_node;

	tok = strtok(path, "/");
	while(tok) {
		cur_childs_num = cur_node->childs_num;
		snprintf(symb_pos, 3, "%s", tok);

		for(i = atoi(symb_pos); i < cur_childs_num; i++) {
			if( strcmp(tok+5, cur_node->childs[i]->repr) == 0 ) {
				cur_node = cur_node->childs[i];
				break;
			}
		}

		if(i == cur_childs_num) {
			if(strcmp(tok, "text") == 0)
				status = isText;
			else
				status = unknown;
			goto out;
		}
		tok = strtok(NULL, "/");
	}
	status = isDir;

out:
	*node = cur_node;
	return status;
}

static int ptfs_getattr(const char *path, struct stat *stbuf)
{
	int retval;
	int status;
	struct pt_node* cur_node;
	retval = 0;
	
	memset(stbuf, 0, sizeof(struct stat));

	status = get_node(path, &cur_node);
	switch(status) {
	case isText:
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = cur_node->e - cur_node->b + 1;
		break;
	case isDir:
		stbuf->st_mode = S_IFDIR | 0755;
		break;
	default:
		retval = -ENOENT;

	}

	return retval;
}

static int ptfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	int i;
	struct pt_node* cur_node;
	char str[2048];
	int status;
	int retval;

	retval = 0;

	status = get_node(path, &cur_node);
	if(status != isDir) {
		retval = -ENOENT;
		goto out;
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

out:
	return retval;
}

static int ptfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	int i;

	char* tok;
	struct pt_node* cur_node = pt_root_node;

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

	len = cur_node->e - cur_node->b;
	memcpy(buf, input + cur_node->b, len);
	buf[len] = '\n';

	return len + 1;
}


static struct fuse_operations hello_oper = {
	.getattr = ptfs_getattr,
	.readdir = ptfs_readdir,
	//.open	= hello_open,
	.read	= ptfs_read,
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
	

	map_nodes_to_subtext ();

	fuse_main(args.argc, args.argv, &hello_oper);
	
	fuse_opt_free_args(&args);
	
	return 0;
}
