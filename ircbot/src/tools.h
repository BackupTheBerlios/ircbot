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
#ifndef __IRCBOT_TOOLS_H__
#define __IRCBOT_TOOLS_H__

#include <stdio.h> 	/* for printf(..) */
#include <stdlib.h>	/* for calloc(..) */
#include <stdarg.h> /* for va_list and friends */

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif
typedef int BOOL;
/* -------------------------------------------------------- */ 
/* --- LIST ----------------------------------------------- */ 
typedef struct _LIST		LIST;
struct _LIST {
	void * data;
	struct _LIST *next;
};
extern LIST * 	LIST_new();
extern LIST * 	LIST_add(LIST *, void *);
extern int 		LIST_count(LIST *);
extern BOOL 	LIST_del(LIST*, void *);
extern void 	LIST_reset(LIST *me);
#define		LIST_next(me) \
		((me) && ((me) = (me)->next) ? (me)->data : NULL)
/* -------------------------------------------------------- */ 
extern void * CALLOC(int elem, size_t sz );
extern void ERROR(char *fmt, ...);
extern void ERROR_EXIT(char *fmt, ...);
extern void DEBUG(char *fmt, ...);
/* -------------------------------------------------------- */ 

#endif
