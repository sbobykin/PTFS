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

struct fuse_operations ptfs_ops = {
	.getattr = ptfs_getattr,
	.readdir = ptfs_readdir,
	//.open	= ptfs_open,
	.read	= ptfs_read,
	//.write = ptfs_write,
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
		for(; flist; flist=flist->next)
			filler(buf, basename(flist->data), NULL, 0);
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

int ptfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	//int i;
	int s_begin;
	tree cur_tr;
	struct pars_obj* pars_obj;
	int status = get_node(path, &cur_tr, &pars_obj);
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
