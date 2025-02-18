/***************************************************************************/
/* (C) 2016 Elettra - Sincrotrone Trieste S.C.p.A.. All rights reserved.   */
/*                                                                         */
/*                                                                         */
/* This file is part of Pore3D, a software library for quantitative        */
/* analysis of 3D (volume) images.                                         */
/*                                                                         */
/* Pore3D is free software: you can redistribute it and/or modify it       */
/* under the terms of the GNU General Public License as published by the   */
/* Free Software Foundation, either version 3 of the License, or (at your  */
/* option) any later version.                                              */
/*                                                                         */
/* Pore3D is distributed in the hope that it will be useful, but WITHOUT   */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   */
/* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License    */
/* for more details.                                                       */
/*                                                                         */
/* You should have received a copy of the GNU General Public License       */
/* along with Pore3D. If not, see <http://www.gnu.org/licenses/>.          */
/*                                                                         */
/***************************************************************************/

//
// Author: Francesco Brun
// Last modified: Sept, 28th 2016
//

#include <stdlib.h>
#include <stdio.h>

#include "p3dDoubleList.h"
#include "p3dUtils.h"

void double_list_init (double_list_t *list)
{
    *list = NULL;    
}

void double_list_add (double_list_t *list, double elem)
{
	double_list_elem_t* new_l;

	// Alloc memory for the new item:
	new_l = (double_list_elem_t*) malloc (sizeof(double_list_elem_t));

	// Push item into queue:
	new_l->ct = elem;
	new_l->next = *list;

	*list = new_l;
}


int double_list_isempty (double_list_t list)
{
    return ( ( list == NULL )? P3D_TRUE : P3D_FALSE );
}

// List is deleted after conversion:
double* double_list_toarray (double_list_t *list, int numel )
{
	double_list_elem_t* tmp_l;


	double* v;
	int i;

	v = (double*) malloc (numel*sizeof(double));

	// Convert list to array:	
	for (i = (numel - 1); i >= 0; i-- )
	{
		v[i] = (*list)->ct;

		// Perform deletion:
		tmp_l = *list;	
	
		(*list) = (*list)->next;

		free(tmp_l);	
	}

	return v;
}