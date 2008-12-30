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

 This file was created by Stanislav Bobykin.
*/ 


#ifndef PT_H
#define PT_H

struct pt_node {
	struct symb;
	struct pt_node** childs;
	struct pt_node* parent;
	int childs_num;
	char* repr;
	int type;
	int b;
	int e;
};

struct pt_node* pt_node;
struct pt_node* pt_child_node;
struct pt_node* pt_root_node;

os_t pt_pos;
int rhs_len;

#endif
