#ifndef __IRCBOT_USER_H__
#define __IRCBOT_USER_H__
/* This is the user Manager for the ircBOT-Library.
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

#include "ircBOT.h" /* for USER-def's, IRC_* def's */
#include "tools.h"	/* for LIST * def, DEBUG and ERROR-Macros */

#include <assert.h>

#define DEFAULT_USER_FILEPATH "./"
typedef LIST USER_LIST;

extern int irc_user_set_list_global(USER_LIST *me);
extern int irc_user_add_list_global(USER_LIST *me);

/* -------------------------------------------------------- */ 
/* all these functions search in the global list if *me == NULL.
 * developers (you!) are able to handle own USER_LIST if
 * neccessary.                                              */
extern int irc_user_add(USER_LIST *me, const char* name, const char* host_user, const char *host, user_mask mask);
extern int irc_user_del(USER_LIST *me, const char* name, const char* host_user, const char *host, user_mask mask);

extern USER_LIST* irc_user_find(USER_LIST *me, const char* name, const char* host_user, const char *host, user_mask mask);

/* -------------------------------------------------------- */ 
/* used by the ircBOT-Library to init a global user-list    */
extern USER_LIST* irc_user_loadlist(const char *file_path);


#endif
