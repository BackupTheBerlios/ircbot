/* these are tools for the ircBOT-Library,
 * ircBOT is a library that provides basic irc-Robot funktionality
 * Copyright (C) 2001 Helmut Djurkin 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include  <stdlib.h>
#include  "tools.h"
/* -------------------------------------------------------- */ 
/* --- Generic List handling ------------------------------ */ 
LIST * LIST_new() {
	LIST *ret = calloc(1, sizeof(LIST));	
	if(ret) {
		ret->next = NULL;
		ret->data = NULL;
	}
	else
		return NULL;
	return ret;
}
LIST * LIST_add(LIST *me, void *data){
	LIST *ret = NULL;
	if(me){
		while( me->next ) me = me->next;
		ret = LIST_new();
		if(ret){
			ret->next = NULL;
			ret->data = data;
			me->next = ret;
		}
		else
			ERROR_EXIT("LIST_add: error in LIST_new()!\n");
	}
	else 
		ERROR("LIST_add: called without a LIST in Parameter!\n");
	return ret;
}
int LIST_count(LIST *me){
	int cnt=0;
	if(me)
		while( (me = me->next) )
			cnt++;
	return cnt;
}
#if 0
BOOL LIST_del(LIST *me, void *data){
	LIST *act = me;
	LIST *prev = NULL;
	while(act->data != data && act){
		prev = act;
		act = act->next;
	}
	if(!act)
		return FALSE;
	if(!prev)
		me = me->next;
	else
		prev->next = act->next;
	free(act);
	return TRUE;
}
#endif
BOOL LIST_del(LIST *me, void *data){
    if (me) {
		LIST *previous;
		while (me->next) {
	    	previous = me;
		    me = me->next;
		    if (me->data == data) {
				previous->next = me->next;
				free(me);
				return TRUE;	/* Success */
		    }
		}
    }
    return FALSE;			/* object not found or NULL list */
}
void LIST_reset(LIST *me){
	if(me){
		LIST *tmp;
		while( (tmp = me->next) ){
			me->next = tmp->next ;
			free(tmp);
		}
	}
}
/* -------------------------------------------------------- */ 
/* --- Allocation memory ---------------------------------- */ 
void * CALLOC(int elem, size_t sz ){
	void *ret = calloc(elem,sz);
	if(!ret){
		fprintf(stderr,"error allocating %i memory-chunk\n", sz * elem);
		exit(-1);
	}
	return ret;
}
/* -------------------------------------------------------- */ 
/* --- Error messages ------------------------------------- */
void DEBUG(char *fmt, ...){
	va_list az;
	va_start(az, fmt);
	vfprintf(stdout,fmt, az);
	va_end(az);
}
void ERROR(char *fmt, ...){
	va_list az;
	va_start(az, fmt);
	vfprintf(stderr,fmt, az);
	va_end(az);
}
void ERROR_EXIT(char *fmt, ...){
	va_list az;
	va_start(az, fmt);
	vfprintf(stderr,fmt, az);
	va_end(az);
	exit(-1);
}
/* -------------------------------------------------------- */

