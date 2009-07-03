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
			stbuf->st_size = text->s_end - text->s_begin + 1;
		}
		else
			stbuf->st_size = pars_obj->in_size + 1;
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

int ptfs_read_from_parsed(char *buf, size_t size, off_t offset)
{
	int my_len = 0;
	int my_offset = 0;
	GList* flist = g_hash_table_get_values(files);
	for(; flist; flist=flist->next) {
		struct pars_obj* pars_obj = flist->data;
		my_len = strlen(pars_obj->full_path);
		memcpy(buf+my_offset, pars_obj->full_path, my_len);
		my_offset += my_len;
		memcpy(buf+my_offset, "\n", 2);
		my_offset += 2;
	}
	return fparsed_size;
}

int ptfs_read(const char *path, char *buf, size_t size, off_t offset,
	      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	int s_begin;
	tree cur_tr;
	struct pars_obj* pars_obj;

	if(strcmp(path, "/__parsed") == 0) {
		return ptfs_read_from_parsed(buf, size, offset);
	}

	int status = get_node(path, &cur_tr, &pars_obj);
	substring* text;

	if(cur_tr->t_element.t_node.n_attributes) {
		text = &(cur_tr->t_element.t_node.n_attributes[0].a_value);
		len = text->s_end - text->s_begin;
		s_begin = text->s_begin;
	}
	else {
		len = pars_obj->in_size;
		s_begin = 0;
	}

	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, pars_obj->input + s_begin + offset, size);
	} 
	else {
		buf[len] = '\n';
		size = 0;
	}

	//memcpy(buf, input + s_begin + offset, len);
	//buf[len] = '\n';

	return size;
}

int ptfs_write (const char* path, const char* buf, size_t size, off_t offset,
	   struct fuse_file_info* fi)
{
	(void) fi;
	if(strcmp(path, "/__unparse") == 0)
	{
		char fname[size];
		memcpy(fname, buf, size);
		fname[size-1] = '\0';
		unparse_file(fname);
		return size;
	}

	if(strcmp(path, "/__parsed") == 0)
	{
		char* fname = malloc( size );
		memcpy(fname, buf, size);
		fname[size-1] = '\0';
		parse_file(fname);
		return size;
	}
}
