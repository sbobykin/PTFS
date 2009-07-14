/* 
 LICENSE: GNU General Public License version 3 
          as published by the Free Software Foundation.
          See the file 'gpl-3.0.txt' for details.
*/

/* This file contains reuse notes. See the file 'REUSE' for details. */

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "ptfs.h"


int ptfs_truncate (const char* path, off_t size) 
{
	return 0;
}

struct fuse_operations ptfs_ops = {
	.truncate = ptfs_truncate,
	.getattr = ptfs_getattr,
	.readdir = ptfs_readdir,
	.read	= ptfs_read,
	.write = ptfs_write,
};

int ptfs_getattr(const char *path, struct stat *stbuf)
{
	int retval;
	int status;
	substring* text;
	tree cur_tr;
	struct pars_obj* pars_obj;
	retval = 0;
	
	memset(stbuf, 0, sizeof(struct stat));

	if(strcmp(path, "/__parsed") == 0) {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;	
		stbuf->st_size = fparsed_size;
		return 0;
	}

	if(strcmp(path, "/__unparse") == 0) {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;	
		stbuf->st_size = 0;
		return 0;
	}

	status = get_node(path, &cur_tr, &pars_obj);
	switch(status) {
	case isText:
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		if(cur_tr->t_element.t_node.n_attributes) {
			text = &(cur_tr->t_element.t_node.n_attributes[0].a_value);
			stbuf->st_size = text->s_end - text->s_begin;
		}
		else
			stbuf->st_size = pars_obj->in_size;
		break;
	case isDir:
		stbuf->st_mode = S_IFDIR | 0755;
		break;
	default:
		retval = -ENOENT;

	}

	return retval;
}

int ptfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	int i;
	tree cur_tr;
	struct pars_obj* pars_obj;
	node cur_nd;
	char str[2048];
	int status;
	int retval;

	retval = 0;

	if(strcmp(path, "/")==0) {
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		GList* flist = g_hash_table_get_keys(files);
		for(; flist; flist=flist->next) {
			filler(buf, flist->data, NULL, 0);
		}
		filler(buf, "__parsed", NULL, 0);
		filler(buf, "__unparse", NULL, 0);
		goto out;
	}

	status = get_node(path, &cur_tr, &pars_obj);

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
		snprintf(str, 2048, "%.3d_%s", i, 
			 cur_nd->n_name);

		filler(buf, str, NULL, 0);
	}
		

out:
	return retval;
}



int ptfs_read (const char *path, char *buf, size_t size, off_t offset,
	       struct fuse_file_info *fi)
{
	(void) fi;


	struct rw_op_context read_op_context = {
		.path = path,
		.buf = buf,
		.size = size,
		.offset = offset,
	};

	if(strcmp(path, "/__parsed") == 0) {
		return ptfs_read_from_parsed(&read_op_context);
	}

	if(strcmp(path, "/__unparse") == 0)
		return 0;

	return ptfs_read_from_text(&read_op_context);

}


int ptfs_write (const char* path, const char* buf, size_t size, off_t offset,
	   struct fuse_file_info* fi)
{
	(void) fi;

	struct rw_op_context write_op_context = {
		.path = path,
		.buf = buf,
		.size = size,
		.offset = offset,
	};

	if(strcmp(path, "/__unparse") == 0)
	{
		return ptfs_write_to_unparse (&write_op_context);
	}

	if(strcmp(path, "/__parsed") == 0)
	{
		return ptfs_write_to_parsed (&write_op_context);
	}
}
