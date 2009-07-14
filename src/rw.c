/* 
 LICENSE: GNU General Public License version 3 
          as published by the Free Software Foundation.
          See the file 'gpl-3.0.txt' for details.
*/

/* This file contains reuse notes. See the file 'REUSE' for details. */

#include <glib.h>
#include <sys/types.h>

#include "ptfs.h"

static size_t put_to_buf (const char* from, size_t len, 
			  struct rw_op_context* read_op_context) 
{
	char* buf = read_op_context->buf;
	size_t size = read_op_context->size;
	off_t offset = read_op_context->offset;

	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, from + offset, size);
	} 
	else
		size = 0;
	return size;
}

int ptfs_read_from_parsed(struct rw_op_context* read_op_context)
{
	int my_len = 0;
	int my_offset = 0;
	char fparsed_buf[fparsed_size];
	GList* flist = g_hash_table_get_values(files);
	for(; flist; flist=flist->next) {
		struct pars_obj* pars_obj = flist->data;
		my_len = pars_obj->full_path_size;
		memcpy(fparsed_buf+my_offset, pars_obj->full_path, my_len);
		my_offset += my_len;
		fparsed_buf[my_offset] = '\n';
		my_offset += 1;
	}
	return put_to_buf(fparsed_buf, fparsed_size, read_op_context);
}

int ptfs_read_from_text(struct rw_op_context* read_op_context)
{
	char* path = read_op_context->path;
	size_t len;
	int s_begin;
	tree cur_tr;
	struct pars_obj* pars_obj;
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

	char substr_buf[len];
	memcpy(substr_buf, pars_obj->input + s_begin, len);

	int retval = put_to_buf(substr_buf, len, read_op_context);
	return retval;
}


int ptfs_write_to_parsed(struct rw_op_context* write_op_context)
{
	char* buf = write_op_context->buf;	
	size_t size = write_op_context->size;

	char fname[size];
	memcpy(fname, buf, size);
	fname[size-1] = '\0';
	parse_file(fname);
	return size;
}

int ptfs_write_to_unparse(struct rw_op_context* write_op_context)
{
	char* buf = write_op_context->buf;	
	size_t size = write_op_context->size;

	char fname[size];
	memcpy(fname, buf, size);
	fname[size-1] = '\0';
	unparse_file(fname);
	return size;
}
