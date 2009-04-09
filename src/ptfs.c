/*
    This file is part of PTFS.

    PTFS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/* 
 Date: 2009-04-09.

 Parser related code from Cocom project was removed.
 New parser related code was derived from "cnog/check.c" of
 the Aurochs distribution version 1.0.93.
 The web site of the Aurochs project: http://aurochs.fr
*/

/* 
 Date: 2008-11-14.

 This file was created using three sources by Stanislav Bobykin.
 
 - Parser related code was derived from "AMMUNITION/earlyey.c" of 
   the Cocom distribution version 0.996. 
   The web site of the Cocom project: http://cocom.sourceforge.net
 
 - FUSE related code was derived from 
   http://fuse.sourceforge.net/helloworld.html
   http://apps.sourceforge.net/mediawiki/fuse/index.php?title=Hello_World_%28fuse_opt.h%29
 
 - File reading (mmap) code was drived from 
   http://beej.us/guide/bgipc/output/html/multipage/mmap.html
*/

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

#include <parse_tree.h>
#include <cnog.h>
#include <peg_lib.h>
#include <pack.h>
#include <alloc.h>
#include <staloc.h>

unsigned char* compile(char*, int*);

const char* program_name = "ptfs";

static tree tr;
static int in_size;

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
	PTFS_OPT_KEY("--version", version, 1),
	PTFS_OPT_KEY("-g %s", grammar, 0),
	PTFS_OPT_KEY("--grammar=%s", grammar, 0),
	PTFS_OPT_KEY("-i %s", input, 0),
	PTFS_OPT_KEY("--input=%s", input, 0),
	FUSE_OPT_END
};


#define isDir 1
#define isText 2
#define unknown 0

static char* input;
static char* grammar;

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
	printf("ptfs version 0.0.2.1\n");
	exit(0);
}


int get_node(const char* path, tree* ret_tr)
{
	int i;
	int status;
	int nd_choosen = 1;
	char symb_pos[3];
	char* tok;
	tree cur_tr = tr;
	node cur_nd;
	
	*ret_tr = tr;
	tok = strtok(path, "/");
	while(tok) {

		nd_choosen = 0;

		if(strcmp(tok, "text") == 0) {
			status = isText;
			goto out;
		}

		snprintf(symb_pos, 3, "%s", tok);
		
		cur_tr = cur_tr->t_element.t_node.n_children;
		if(!cur_tr) {
			status = isDir;
			goto out;
		}

		for(i = 0;
		    cur_tr; 
		    cur_tr = cur_tr->t_sibling, i++) {
			cur_nd = &(cur_tr->t_element.t_node);
			if(strcmp(tok+3, cur_nd->n_name) == 0 &&
			    i == atoi(symb_pos) ) {
				*ret_tr = cur_tr;
				nd_choosen = 1;
				break;
			}
		}


		if(!nd_choosen) {
			status = unknown;
			goto out;
		}

		
		tok = strtok(NULL, "/");
	}
	status = isDir;

out:
	return status;
}

static int ptfs_getattr(const char *path, struct stat *stbuf)
{
	int retval;
	int status;
	substring* text;
	tree cur_tr;
	retval = 0;
	
	memset(stbuf, 0, sizeof(struct stat));

	status = get_node(path, &cur_tr);
	switch(status) {
	case isText:
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		if(cur_tr->t_element.t_node.n_attributes) {
			text = &(cur_tr->t_element.t_node.n_attributes[0].a_value);
			stbuf->st_size = text->s_end - text->s_begin + 1;
		}
		else
			stbuf->st_size = in_size + 1;
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
	tree cur_tr;
	node cur_nd;
	char str[2048];
	int status;
	int retval;

	retval = 0;

	status = get_node(path, &cur_tr);

	if(status != isDir) {
		retval = -ENOENT;
		goto out;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, "text", NULL, 0);

	if(!cur_tr)
		goto out;
	
	cur_nd = &(cur_tr->t_element.t_node);

	

	for(cur_tr = cur_nd->n_children, i = 0; 
	    cur_tr; 
	    cur_tr = cur_tr->t_sibling, i++) {
		cur_nd = &(cur_tr->t_element.t_node);
		snprintf(str+i, 2048, "/%.2d_%s", i, 
			 cur_nd->n_name);

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
	int s_begin;
	tree cur_tr;
	int status = get_node(path, &cur_tr);
	substring* text;

	/*char* tok;
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
	}*/

	if(cur_tr->t_element.t_node.n_attributes) {
		text = &(cur_tr->t_element.t_node.n_attributes[0].a_value);
		len = text->s_end - text->s_begin;
		s_begin = text->s_begin;
	}
	else {
		len = in_size;
		s_begin = 0;
	}

	memcpy(buf, input + s_begin, len);
	buf[len] = '\n';

	return len + 1;
}

static struct fuse_operations hello_oper = {
	.getattr = ptfs_getattr,
	.readdir = ptfs_readdir,
	//.open	= hello_open,
	.read	= ptfs_read,
};

static char* map_file_to_str (char* pathname, int* size)
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

	if(size)
		*size = sbuf.st_size;

	return data;
}

int main(int argc, char** argv)
{
	caml_startup(argv);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	memset(&options, 0, sizeof(struct options));
	if (fuse_opt_parse(&args, &options, ptfs_opts, NULL) == -1)
		print_usage(stderr, -1);
	
	if(options.version)
		print_version();
	
	if(options.help)
		print_usage(stdout, 0);

	if(options.grammar && options.input) {
		/* read grammar */
		grammar = map_file_to_str(options.grammar, NULL);
		/* read input */
		input = map_file_to_str(options.input, &in_size);
	}
	else
		print_usage(stderr, -1);


	/* Reused code from aurochs/cnog/check.c (begin)*/
	unsigned char *peg_data;
	char *peg_fn;
	size_t peg_data_size;
	nog_program_t *pg;
	packer_t pk;
	staloc_t *st;
	int rc;

	rc = 0;


	peg_data = compile(grammar, &peg_data_size);
	if(!peg_data) {
		//printf("Can't load peg data.\n");
		exit(EXIT_FAILURE);
	}

	/* Create a stack allocator */

	st = staloc_create(&alloc_stdlib);
	if(!st) {
		//printf("Can't create stack allocator.\n");
		exit(EXIT_FAILURE);
	}

	if(pack_init_from_string(&pk, peg_data, peg_data_size)) {
		//printf("peg_data[0] = %d\n", peg_data[0]);
		pg = cnog_unpack_program(&st->s_alloc, &pk);
		//printf("Unpacked to %p\n", pg);
		if(pg) {
			peg_context_t *cx;
			size_t m = in_size;
			int i;
			int error_pos;
			char *fn;
			unsigned char *buf;
			int rc;

			rc = 0;

			//fn = argv[i];
			peg_builder_t pb;
			staloc_t *s2;

			s2 = staloc_create(&alloc_stdlib);

			buf = input;
			ptree_init(&pb, &s2->s_alloc);
			cx = peg_create_context(&alloc_stdlib, pg, &pb, &s2->s_alloc, buf, m);
			//printf("Created context %p\n", cx);
			if(cx) {

				if(cnog_execute(cx, pg, &tr)) {
					//printf("Parsed as %p.\n", tr);
					//ptree_dump_tree(cx->cx_builder_info, stdout, buf, tr, 0);
					fuse_main(args.argc, args.argv, &hello_oper);

				} else {
					printf("Doesn't parse.\n");
					error_pos = cnog_error_position(cx, pg);
					printf("Error at %d\n", error_pos);
				}

				peg_delete_context(cx);
			}
			staloc_dispose(s2);
			//free(buf);

			/* cnog_free_program(&st->s_alloc, pg); */
			staloc_dispose(st);
		}
	}

	fuse_opt_free_args(&args);
	return rc;
	/* Reused code from aurochs/cnog/check.c (end) */
}
