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

#include "ircBOT.h"
#include <string.h>
#include <unistd.h>		/* für read() */
#include <errno.h>
#include <sys/socket.h>	/* für socket(), connect() */
#include <arpa/inet.h> 	/* für inet_pton() */ 

#ifndef TCP_NODELAY
	#define   TCP_NODELAY     0x01    /* don't delay send to coalesce packets */
#endif

/* zur Sicherung in strcmp (das hauts auf wenn ein parameter NULL is) */
#define SICHER_STR(x) (x ? x:"")

LIST * all_bots = NULL;
/* -------------------------------------------------------- */ 
int irc_add_cmd	(BOT *me, const char *cmd, cmd_property prop, int min_user_level, CMD_FUN *fun){
/* -------------------------------------------------------- */ 
	cmd_tab * ct = (cmd_tab*) CALLOC(1,sizeof(cmd_tab));
	ct->cmd 			= strdup(cmd);
	ct->min_user_level 	= min_user_level;
	ct->property 		= prop;
	ct->fun 			= fun;
	if( ! LIST_add(me->cmd_handler, ct) ){
		ERROR("irc_add_cmd: Error adding command '%s' to LIST.\n", cmd);
		return IRC_ERROR;
	}
	return IRC_OK;
};
/* -------------------------------------------------------- */ 
int irc_del_cmd	(BOT *me, const char *cmd, CMD_FUN *fun){
/* -------------------------------------------------------- */
	LIST * cmd_list = me->cmd_handler;
	cmd_tab *ct 	= NULL;
	char * thisCMD 	= NULL;
	while( (ct = LIST_next( cmd_list ) ) ){
		if( strcmp(ct->cmd, cmd) == 0 && ct->fun == fun ){
			thisCMD = ct->cmd;
			break;
		}
	}
	if(!thisCMD){
		ERROR("irc_del_cmd: command '%s' not found!\n", cmd);
		return IRC_ERROR;
	}
	if(!LIST_del(me->cmd_handler, ct)){
		ERROR("irc_del_cmd: error removing %p with data at %p from list at %p!\n",
				cmd_list, ct, me); 
		return IRC_ERROR;
	}
	return IRC_OK;
};
/* -------------------------------------------------------- */ 
int irc_add_tee	(BOT *me, int event_type, TEE_FUN *fun){
/* -------------------------------------------------------- */ 
	tee_tab * tt 	= (tee_tab*) CALLOC(1, sizeof(tee_tab));
	tt->event_type 	= event_type;
	tt->fun 		= fun;
	if( ! LIST_add(me->tee_handler, tt) ){
		ERROR("irc_add_tee: Error adding fun off event_type '%i'!\n", event_type);
		return IRC_ERROR;
	}
	return IRC_OK;
};
/* -------------------------------------------------------- */ 
int irc_del_tee	(BOT *me, TEE_FUN *fun){
/* -------------------------------------------------------- */ 
	LIST * tee_list 	= me->tee_handler;
	tee_tab * tt	 	= NULL;
	TEE_FUN * thisFUN 	= NULL;
	while( (tt = LIST_next( tee_list )) ){
		if( tt->fun == fun ){
			thisFUN = tt->fun;
			break;
		}
	}
	if(!thisFUN){
		ERROR("irc_del_tee: funktion at %p not found!\n", fun);
		return IRC_ERROR;
	}
	if(!LIST_del(me->tee_handler, tt)){
		ERROR("irc_del_tee: Error removing tee-function at %p from list at %p!\n",thisFUN, tt);
		return IRC_ERROR;
	}
	return IRC_OK;
};
/* -------------------------------------------------------- */ 
int irc_add_timer(BOT *me, time_t time_alarm, TIMER_FUN *fun, void *parameter){
/* -------------------------------------------------------- */ 
	ERROR("irc_add_timer: not implemented!\n");
	return IRC_ERROR;ERROR("ircBOR_part: not implemented!\n");
};
/* -------------------------------------------------------- */ 
int irc_del_timer(BOT *me, int timer_id){
/* -------------------------------------------------------- */ 
	ERROR("irc_del_timer: not implemented!\n");
	return IRC_ERROR;
};
/* -------------------------------------------------------- */ 
int irc_eventloop(){
/* -------------------------------------------------------- */ 
	fd_set rset, wset, allRset, allWset;
	int fdmax=0, nready;
	char buf[MAX_MSG];

	while( TRUE ){
		LIST *mybots= all_bots;
		BOT	*bot	= NULL;
		if(!mybots)
			ERROR_EXIT("PANIC ?, entering eventloop without a bot!!\n");
		
		memset(buf, '\0', MAX_MSG);
		FD_ZERO(&allRset);
		FD_ZERO(&allWset);
		while( (bot = LIST_next(mybots)) ){
			if( bot->srv.sendbuf )
				FD_SET(bot->srv.socket, &allWset);
			FD_SET(bot->srv.socket, &allRset);
			fdmax = fdmax > bot->srv.socket ? fdmax:bot->srv.socket;
		}
		rset = allRset;
		wset = allWset;
		/* select */
		nready = select(fdmax+1, &rset, &wset, NULL, NULL);
		if(nready==0) /* timeout (currently not usefull) */
			continue;
		if(nready<0)
			ERROR_EXIT("error in select: %s!\n", strerror(errno));
		mybots = all_bots;
		bot = NULL;
		while( (bot = LIST_next(mybots)) ){
			/* should we read from the server ? */
			if(FD_ISSET(bot->srv.socket, &rset)){
				bot->srv.state = IRC_SYS_READ_EVENT;
				irc_sys_state_maschine( bot );
			}
			/* should we write to the server ? */
			if(FD_ISSET(bot->srv.socket, &wset)){
				bot->srv.state = IRC_SYS_WRITE_EVENT;
				irc_sys_state_maschine( bot );
			}
		}
	}
	
	return IRC_ERROR;
};
/* -------------------------------------------------------- */ 
int irc_sys_state_maschine(BOT *me){
/* -------------------------------------------------------- */ 
	BOT *this 	= me;
	int i = 0, bufsize = MAX_MSG;
	MSG *msg 	= NULL;
	LIST* out_list	= this->srv.sendbuf;
	while( this->srv.state != IRC_SYS_IDLE ){
		switch( this->srv.state ){
			case IRC_SYS_READ_EVENT:
					msg = MSG_new(this);
					msg->raw_msg = CALLOC(1, MAX_MSG);
					this->srv.state = IRC_SYS_READ;
					break;
			case IRC_SYS_WRITE_EVENT:
					if( this->srv.sendbuf )
						this->srv.state = IRC_SYS_WRITE;
					else
						this->srv.state = IRC_SYS_IDLE; /* nix zum schreiben, daher retour */
					break;
			case IRC_SYS_READ:
					/* only read a line */	
					while ( (recv(this->srv.socket, &msg->raw_msg[i], 1, 0) == 1) ) {
						if (msg->raw_msg[i] == '\n' || bufsize == i + 1) {
							msg->raw_msg[i] = '\0';
							break;
						}
						if (msg->raw_msg[i] == '\r')
							msg->raw_msg[i] = '\0';
						i++;
					}
					/* if a line exist */
					if( i > 0 ){
						LIST_add(this->srv.history, msg);
						irc_sys_parse_msg(this, msg);
					
						fprintf(stdout, "%s\n", msg->raw_msg);
						printf("-------------------------------------\n");
						this->srv.state = IRC_SYS_TEELOOP;
					}
					else {
						DEBUG("nix gelesen ????\n");
						free(msg->raw_msg);
						free(msg);
						this->srv.state = IRC_SYS_IDLE;
						//if( i < 0 )
							ERROR_EXIT("Error recv: '%s'\n", strerror(errno));
					}
					break;
			case IRC_SYS_WRITE:
					if( (msg = LIST_next( out_list )) ){
						send(this->srv.socket, msg->raw_msg, strlen(msg->raw_msg), 0);
						DEBUG("---------->message written:%s", msg->raw_msg);
						LIST_add(this->srv.history, msg);
						LIST_del(this->srv.sendbuf, msg);
						this->srv.state = IRC_SYS_TEELOOP;
					}
					else if( this->srv.sendbuf ){
						free( this->srv.sendbuf );
						this->srv.sendbuf = NULL;
						this->srv.state = IRC_SYS_IDLE;
					}
					break;
			case IRC_SYS_TEELOOP:
					/* TODO: implement TEELOOP */
					this->srv.state = IRC_SYS_CMDLOOP;
					break;
			case IRC_SYS_CMDLOOP:
					irc_sys_cmd_loop(this, msg);
					this->srv.state = IRC_SYS_IDLE;
					break;
			default:
					return IRC_ERROR;
		}
	}
	return IRC_ERROR;
}
/* -------------------------------------------------------- */ 
void irc_sys_cmd_loop(BOT *me, MSG* msg){
/* -------------------------------------------------------- */ 		
	BOT *this 	= me;
	LIST * l 	= this->cmd_handler;
	BOOL found 	= FALSE;
	cmd_tab *ct = NULL;
	while( ( ct = LIST_next(l)) ){
		if(ct){
			found = FALSE;
			if( ct->property & CMD_SEARCH_CMD ){
				if( ct->property & CMD_POS_FLOAT ){
					if(strstr(msg->cmd, ct->cmd))
						found = TRUE;
				}
				else /* Default is CMD_POS_LEFT */
					if( strncmp(SICHER_STR(msg->cmd), ct->cmd, strlen(ct->cmd)) == 0 )
						found = TRUE;
				if(found == TRUE) {
					(*ct->fun)(msg);
				}
				/* TODO: check userlevel !!!!*/
			}
		}
	}
}
/* -------------------------------------------------------- */ 
BOT* ircBOT_new	(const char *nick, const char *pwd){
/* -------------------------------------------------------- */ 
	int x = 0;
	BOT *bot 	= (BOT*) CALLOC(1, sizeof(BOT));
	if(bot){
		if(!nick) 
			ERROR_EXIT("ircBOT_new: Error no NICK!!!\n");
		bot->nick 			= strdup(nick);
		if(pwd)
			bot->pwd 		= strdup(pwd);
		else
			bot->pwd 		= NULL;
		bot->srv.state 		= IRC_SYS_NEEDCONNECT;
		bot->srv.sendbuf	= NULL;
		bot->cmd_handler 	= LIST_new();
		bot->tee_handler 	= LIST_new();
		for(x=0;x<MAX_TIMER;x++)
			bot->timer[x].id = -1;
	}
	/* add this bot to the global ones */
	if(!all_bots)
		all_bots = LIST_new();
	if(!LIST_add(all_bots, bot))
		ERROR_EXIT("ircBOT_new: Error adding bot to global BOT-List!!\n");
	DEBUG("BOT with nick '%s' created.\n", bot->nick);
	return bot;
};

/* -------------------------------------------------------- */ 
int ircBOT_server(BOT *me, const char *host, int port){
/* -------------------------------------------------------- */
	IRC_SRV *srv = &me->srv;
	struct sockaddr_in servaddr;
	int on=1;
	int irc_port = port > 0 ? port:IRC_PORT;

	if(srv->state != IRC_SYS_NEEDCONNECT)
		ircBOT_disconnect(me);
	
	srv->socket = socket(AF_INET, SOCK_STREAM, 0);	
	setsockopt(srv->socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
   	servaddr.sin_family		= AF_INET;
	servaddr.sin_port		= htons(irc_port);
    inet_pton(AF_INET, host, &servaddr.sin_addr);	

	if( connect(srv->socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0 ){
		ERROR("ircBOT_server: Error in connect: '%s' !\n", strerror(errno));
		return IRC_ERROR;
	}
	srv->name 	= strdup(host);
	srv->state 	= IRC_SYS_IDLE;
	if(!srv->channels)
		srv->channels = LIST_new();
	if(!srv->history)
		srv->history = LIST_new();
	DEBUG("ircBOT_server: connected to '%s' on port '%i'.\n", srv->name, irc_port);
	/* ************************************** */
	/* prepare to send NICK command to server */
	//MSG_out(me, NULL, irc_build_msg(NULL, "PASS", "", NULL));
	MSG_out(me, NULL, irc_build_msg(NULL, "NICK", me->nick, NULL));
	MSG_out(me, NULL, irc_build_msg(NULL, "USER", irc_build_param("%s 0 *",me->nick), me->nick)) ;
	/* ************************************** */
	return IRC_OK;
}
/* -------------------------------------------------------- */ 
int ircBOT_disconnect(BOT *me){
/* -------------------------------------------------------- */
	MSG_out(me, NULL, irc_build_msg(NULL, "QUIT", "", "tschau...")) ;
	return IRC_ERROR;
}
/* -------------------------------------------------------- */ 
int ircBOT_join	(BOT *me, const char *channel){
/* -------------------------------------------------------- */
	CHAN *chan 	= CALLOC(1,sizeof(CHAN));
	chan->name 	= strdup(channel);
	LIST_add(me->srv.channels, chan);
	
	MSG_out(me, NULL, irc_build_msg(NULL, "JOIN", channel, NULL));
	return IRC_OK;
}
/* -------------------------------------------------------- */ 
int ircBOT_part	(BOT *me, const char *channel){
/* -------------------------------------------------------- */ 
	MSG_out(me, NULL, irc_build_msg(NULL, "PART", channel, NULL));
	return IRC_OK;
}
/* -------------------------------------------------------- */ 
int ircBOT_msg_raw	(BOT *me, const char *msg){
/* -------------------------------------------------------- */ 
	MSG_out(me, NULL, irc_build_msg(NULL, msg, "", NULL));
	return IRC_OK;
}
/* -------------------------------------------------------- */ 
int ircBOT_msg	(BOT *me, const CHAN *chan, const USER *user, const char *msg){
/* -------------------------------------------------------- */ 
	if( user ){
		MSG_out(me, NULL, irc_build_msg( NULL, "NOTICE", user->name, msg));
		return IRC_OK;
	}
	if( chan ){
		MSG_out(me, NULL, irc_build_msg( NULL, "PRIVMSG", chan->name, msg));
		return IRC_OK;
	}
	ERROR("ircBOT_msg: no USER AND no CHAIN in parameter !!!\n");
	return IRC_ERROR;
};
/* -------------------------------------------------------- */ 
/* -------------------------------------------------------- */ 
/* FIND - Commands                                          */
/* -------------------------------------------------------- */ 
CHAN* irc_sys_find_chan	(BOT *me, const char *chan){
/* -------------------------------------------------------- */ 		
	LIST *l = me->srv.channels;
	CHAN *this = NULL;
	while ( (this = LIST_next(l)) ){
		DEBUG("irc_sys_find_chan: test '%s' vs. '%s'\n",chan, this->name);
		if(strcmp( this->name, chan ) == 0)
			return this;
	}
	return this;
}
/* -------------------------------------------------------- */ 
USER* irc_sys_find_username	(BOT *me, const char *user){
/* -------------------------------------------------------- */ 		
	ERROR("irc_sys_find_username: not implemented!\n");
	return NULL;
};

/* -------------------------------------------------------- */ 
USER* irc_sys_find_user_or_add	(BOT *me, USER *this_user){
/* -------------------------------------------------------- */ 		
	LIST *l = me->srv.users;
	USER *user = NULL;
	if(!this_user){
		ERROR("irc_sys_find_user_or_add: NO 'this_user' in Parameter\n");
		return NULL;
	}
	while( (user = LIST_next(l)) ){
		if(	strcmp(SICHER_STR(this_user->name), 	SICHER_STR(user->name)) == 0 &&
			strcmp(SICHER_STR(this_user->host_user),SICHER_STR(user->host_user)) == 0 &&
			strcmp(SICHER_STR(this_user->host), 	SICHER_STR(user->host)) == 0
			)
			return user;
	}
	if(!me->srv.users)
		me->srv.users = LIST_new();
	LIST_add(me->srv.users, this_user);
	return NULL;
};
/* -------------------------------------------------------- */ 
/* -------------------------------------------------------- */ 
/* BUILD - Commands                                         */
/* -------------------------------------------------------- */ 
char * irc_build_msg(const char *prefix, const char *command, const char *param, const char *trailing){
/* -------------------------------------------------------- */ 
	char *ret = CALLOC(1, MAX_MSG);
	snprintf(ret, MAX_MSG, "%s %s %s%s\r\n", 
					prefix ? prefix:command, 
					prefix ? command:param,
					prefix ? param:( trailing ? ":":"" ),
					trailing ? trailing:"");
	return ret;
}
/* -------------------------------------------------------- */ 
char * irc_build_param(char *fmt, ...){
/* -------------------------------------------------------- */ 		
	static char ret[MAX_MSG];
	va_list az;
	memset(ret, '\0', MAX_MSG);
	
	va_start(az, fmt);
	vsprintf(ret,fmt, az);
	va_end(az);
	return ret;
}
/* -------------------------------------------------------- */ 
/* -------------------------------------------------------- */ 
/* PARSE - Commands                                         */
/* -------------------------------------------------------- */ 
char * strndup(char *s, char * end){
/* -------------------------------------------------------- */ 		
	char save = '\0';
	char *ret = NULL;
	if( end ){
		save = *end;
		*end = '\0';
	}
	ret = strdup(s);
	if(end)
		*end = save;
	return ret;
}
/* -------------------------------------------------------- */ 
int irc_sys_parse_msg(BOT *me, MSG *msg){
/* -------------------------------------------------------- */ 		
	char *raw = msg->raw_msg;
	if( *raw == ':' ){
		USER * tmp_user = NULL;
		char *i = NULL;
		if( !msg->user )
			msg->user = CALLOC(1, sizeof(USER));

		raw++;
		if( (i = index( raw, '!' )) ){
			msg->user->name = strndup(raw, i);
			raw = ++i;
			if( (i = index( raw, '@')) ){ 
				msg->user->host_user = strndup(raw, i);
				raw = ++i;
			}
		}
		if( (i=index(raw, ' ')) ){
			msg->user->host = strndup(raw, i);
			raw = ++i;
		}
		if( (i=index(raw, ' ')) ){
			msg->cmd = strndup(raw, i);
			raw = ++i;
		}
		if( (i=index(raw, ':')) ){
			msg->parameter = strndup(raw, i);
			raw = ++i;
			msg->postfix = strdup(raw);
		}
		else
			msg->parameter = strdup(raw);

		printf("\nuser: '%s', host_user: '%s', host: '%s', cmd: '%s', param: '%s', postfix: '%s'\n",
						msg->user->name, msg->user->host_user, msg->user->host, 
						msg->cmd, msg->parameter, msg->postfix);
		if( (tmp_user = irc_sys_find_user_or_add(me, msg->user)) ){
			//DEBUG("<<<<<<<< same USER found, freeing this one >>>>>>>>>>>>>>\n");
			free( msg->user->name );
			free( msg->user->host_user );
			free( msg->user->host );
			free( msg->user );
			msg->user = tmp_user;
		}
	}
	else {			
		if( strncmp(raw, strPING, strlen(strPING)) == 0 ){
			raw = &raw[ strlen(strPING) + 1 ];
			MSG_out(me, NULL, irc_build_msg(NULL, "PONG", raw, NULL));			
		}
	}
	return IRC_OK;	
}
/* -------------------------------------------------------- */ 
/* -------------------------------------------------------- */ 
/* MSG Tool-Commands                                        */
/* -------------------------------------------------------- */ 
MSG * MSG_new(BOT *me){
/* -------------------------------------------------------- */ 		
	MSG *ret = CALLOC(1, sizeof(MSG));
	if(!ret)
		ERROR_EXIT("MSG_new: error by calloc !!\n");    	
	if(me)
		ret->reciver = me;
	return ret;
}
/* -------------------------------------------------------- */ 
MSG* MSG_out(BOT *me, MSG * mymsg, char * raw_msg){
/* -------------------------------------------------------- */ 
	MSG * msg = mymsg;
	if( !msg )
		msg = MSG_new(me);
	msg->raw_msg = raw_msg;
	if(!me->srv.sendbuf)
		me->srv.sendbuf = LIST_new();
	LIST_add(me->srv.sendbuf, msg);
	return msg;
}
