#include "ptfs.h"

int get_node(const char* path, tree* ret_tr, struct pars_obj** ret_pars_obj)
{
	int i;
	int status;
	int nd_choosen = 1;
	char symb_pos[4];
	char* tok;
	tree cur_tr = tr;
	node cur_nd;
	struct pars_obj* pars_obj;
	
	*ret_tr = tr;
	tok = strtok(path, "/");
	if(tok!=NULL) {
		pars_obj = g_hash_table_lookup(files, tok);
		*ret_pars_obj = pars_obj;
		if(pars_obj) {
			cur_tr = pars_obj->tr;
			*ret_tr = cur_tr;
		}
		else {
			status = unknown;
			goto out;
		}
	}
	tok = strtok(NULL, "/");
	//printf("tok is %s\n", tok);
	while(tok) {

		nd_choosen = 0;

		if(strcmp(tok, "text") == 0) {
			status = isText;
			goto out;
		}

		snprintf(symb_pos, 4, "%s", tok);
		
		cur_tr = cur_tr->t_element.t_node.n_children;
		if(!cur_tr) {
			status = isDir;
			goto out;
		}

		for(i = 0;
		    cur_tr; 
		    cur_tr = cur_tr->t_sibling, i++) {
			cur_nd = &(cur_tr->t_element.t_node);
			if(strcmp(tok+4, cur_nd->n_name) == 0 &&
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
