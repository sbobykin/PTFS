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

void right_down(tree cur_tr, int dlt)
{
	substring* text;
	tree r_tr;
	node_t* node;


	for(r_tr = cur_tr; r_tr; r_tr=r_tr->t_sibling) {
		node = &(r_tr->t_element.t_node);
		text = &(node->n_attributes[0].a_value);

		text->s_begin += dlt;
		text->s_end += dlt;
		if(node->n_children)
			right_down(node->n_children, dlt);
	}
}

void up_right_down(tree cur_tr, int dlt, int border)
{
	substring* text;
	tree r_tr;
	node_t* node;
	tree u_tr;

	for(u_tr = cur_tr; u_tr->t_parent; u_tr = u_tr->t_parent) {
		node = &(u_tr->t_element.t_node);
		text = &(node->n_attributes[0].a_value);
		text->s_end+=dlt;
	
		right_down(u_tr->t_sibling, dlt);
	}
}

int ptfs_write_to_text(struct rw_op_context* write_op_context)
{
	char* buf = write_op_context->buf;	
	char* path = write_op_context->path;
	size_t size = write_op_context->size;
	off_t offset = write_op_context->offset;
	struct pars_obj* pars_obj;
	tree cur_tr;
	tree tr;
	node_t* node;
	int i,j;
	substring* text;
	size_t len, str_len;
	int border;
	char* new_input;

	int status = get_node(path, &cur_tr, &pars_obj);
	char* cur_input = pars_obj->input;
	
	node_t* cur_node = &(cur_tr->t_element.t_node);
	if(cur_node->n_attributes) {
		text = &(cur_node->n_attributes[0].a_value);
		int dlt = size - (text->s_end - text->s_begin - offset);
		int new_size = pars_obj->in_size - (text->s_end - text->s_begin) + size + offset;
		new_input = malloc(new_size + 1);

		memcpy(new_input, cur_input, text->s_begin + offset);
		memcpy(new_input + text->s_begin + offset, buf, size);
		memcpy(new_input + text->s_begin + offset + size, 
		       cur_input + text->s_end,
		       pars_obj->in_size - text->s_end);

		cur_node->n_children = NULL;

		up_right_down(cur_tr, dlt, border);

		new_input[new_size]='\0';
		pars_obj->input = new_input;
		pars_obj->in_size = new_size;
		free(cur_input);
	}
	else {
		new_input = malloc(size);
		memcpy(new_input, buf, size);

		pars_obj->input = new_input;
		pars_obj->in_size = size-1;
		free(cur_input);

		cur_node->n_children = NULL;
	}

	return size;
}
