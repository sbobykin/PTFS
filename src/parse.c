/* 
 LICENSE: GNU General Public License version 3 
          as published by the Free Software Foundation.
          See the file 'gpl-3.0.txt' for details.
*/

/*
 Date: 2009-06-10.

 This file contains the code for parsing, that was moved from 'src/ptfs.c'.
 The code that uses Aurochs parsing API was reused 
 from 'cnog/check.c' of the Aurochs distribution version 1.0.93.
 The web site of the Aurochs project: http://aurochs.fr
*/

#include <stdio.h>
#include <stdlib.h>

#include <parse_tree.h>
#include <cnog.h>

#include "ptfs.h"

int parse_file(char* file_name)
{
	input = map_file_to_str(file_name, &in_size);
	struct pars_obj* pars_obj = malloc( sizeof(struct pars_obj));
	pars_obj->input = input;
	pars_obj->in_size = in_size;
	
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
					pars_obj->tr = tr;
					g_hash_table_insert(files, 
							basename(file_name), pars_obj);

				} else {
					printf("Doesn't parse.\n");
					error_pos = cnog_error_position(cx, pg);
					printf("Error at %d\n", error_pos);
				}

				//?peg_delete_context(cx);
			}
			//?staloc_dispose(s2);
			//free(buf);

			/* cnog_free_program(&st->s_alloc, pg); */
			//?staloc_dispose(st);
		}
	}
	return rc;
	/* Reused code from aurochs/cnog/check.c (end) */
}
