#ifndef __IRCBOT_H__
#define __IRCBOT_H__
/* ircBOT is a library that provides basic irc-Robot funktionality
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
#include <time.h>
#include "tools.h"

#define IRC_PORT 6667
/*  ircBOT-Maschine States:
 *  
 *  allowed state-transitions
 *	0 1 4 6 7 8
 *	0 1 4 6 7 0
 *	0 2 5 6 7 0
 * ***************************/
#define IRC_SYS_NEEDCONNECT -1
#define IRC_SYS_IDLE 		0
#define IRC_SYS_READ_EVENT	1
#define IRC_SYS_WRITE_EVENT	2
//#define IRC_SYS_FINDBOT		3
#define IRC_SYS_READ		4
#define IRC_SYS_WRITE		5
#define IRC_SYS_TEELOOP		6
#define IRC_SYS_CHECK_CMD	7
#define IRC_SYS_CMDLOOP		8

/* ircBOT Error-Codes */
#define IRC_OK		100
#define IRC_ERROR	-100
#define IRC_ERR_UNKN_HOST 	IRC_ERROR-1
#define IRC_ERR_UNKN_CHAN	IRC_ERROR-2
#define IRC_ERR_UNKN_NICK	IRC_ERROR-3
#define IRC_ERR_UNKN_CMD	IRC_ERROR-4
#define IRC_ERR_AUTH		IRC_ERROR-5
#define IRC_ERR_PARAMETER	IRC_ERROR-6

/* maximum usable timer (per Bot) */
#define MAX_TIMER 	256
/* max Message buffer (see rfc2812) */
#define MAX_MSG		512*10

#define strPING "PING"
#define strPOMG "PONG"

typedef struct _USER 		USER;
typedef struct _CHAN 		CHAN;
typedef struct _IRC_SRV		IRC_SRV;
typedef struct _BOT			BOT;
typedef struct _cmd_tab		cmd_tab;
typedef struct _tee_tab		tee_tab;
typedef struct _timer_tab 	timer_tab;
typedef enum _cmd_property	cmd_property;
typedef struct _MSG			MSG;
/* Library Callbacks */
typedef int CMD_FUN (MSG*);
typedef int TEE_FUN (MSG*);
typedef int TIMER_FUN (void *);

/* ircBOT command properties*/
enum _cmd_property {
	CMD_SEARCH_CMD			= 0x1, // =     1
	CMD_SEARCH_PARAMETER	= 0x2, // =    10
	CMD_SEARCH_POSTFIX  	= 0x4, // =   100
	CMD_POS_LEFT			= 0x8, // =  1000
	CMD_POS_FLOAT			= 0x10 // =  1010
};

struct _cmd_tab {
	char 			*cmd;
	int				min_user_level;
	cmd_property 	property;
	CMD_FUN			*fun;
};

struct _tee_tab {
	int 		event_type;	//IRC_SYS_*_EVENT
	TEE_FUN		*fun;
};

struct _timer_tab {
	int			id;
	time_t		alarm_time;
	TIMER_FUN	*fun;
};

struct _USER {
	char 	*name;
	char 	*host_user;
	char 	*host;
	int		user_level;
};

struct _CHAN {
	char	*name;
	LIST	*users;	
	char	*topic;
	long	chan_state;
};

struct _IRC_SRV {
	char	*name;
	int 	socket;
	LIST	*channels;	// *CHAN
	LIST 	*users;		// *USER
	int 	state;
	LIST	*sendbuf;	// *MSG
	LIST	*history;	// *MSG
};

struct _MSG {
	BOT 	*reciver;
	CHAN 	*chan;
	USER 	*user;
	//char 	*msg;
	char 	*cmd;
	char 	*parameter;
	char	*postfix;
	char 	*raw_msg;
};

struct _BOT {
	char	*nick;
	char	*pwd;
	IRC_SRV	srv;
	LIST	*cmd_handler;	// cmd_tab
	LIST	*tee_handler;	// tee_tab
	timer_tab	timer[MAX_TIMER];
};

extern int irc_add_cmd	(BOT *me, const char *cmd, cmd_property prop, int min_user_level, CMD_FUN *fun);
extern int irc_del_cmd	(BOT *me, const char *cmd, CMD_FUN *fun);
extern int irc_add_tee	(BOT *me, int event_type, TEE_FUN *fun);
extern int irc_del_tee	(BOT *me, TEE_FUN *fun);
extern int irc_add_timer(BOT *me, time_t time_alarm, TIMER_FUN *fun, void *parameter);
extern int irc_del_timer(BOT *me, int timer_id);
extern int irc_eventloop();

extern BOT* ircBOT_new	(const char *nick, const char *pwd);
extern int ircBOT_server(BOT *me, const char *host, int port);
extern int ircBOT_disconnect(BOT *me);
extern int ircBOT_join	(BOT *me, const char *channel);
extern int ircBOT_part	(BOT *me, const char *channel);
extern int ircBOT_msg	(BOT *me, const CHAN *chan, const USER *user, const char *msg);
extern int ircBOT_msg_raw(BOT *me, const char *msg);

/* -------------------------------------------------------- */ 
/* These commands may only be used by the Library !!        */
/* -------------------------------------------------------- */ 
extern int irc_sys_state_maschine(BOT *me);

extern CHAN* irc_sys_find_chan	(BOT *me, const char *chan);
extern USER* irc_sys_find_username(BOT *me, const char *user);
extern USER* irc_sys_find_user_or_add	(BOT *me, USER *user);

extern char * irc_build_msg(const char *prefix, const char *command, const char *param, const char *trailing);
extern char * irc_build_param(char *fmt, ...);

extern int irc_sys_parse_msg(BOT *me, MSG *msg);

extern MSG* MSG_new	(BOT *);
extern MSG* MSG_out(BOT *me, MSG * msg, char * raw_msg);

extern void irc_sys_cmd_loop(BOT *me, MSG* msg);
#endif
