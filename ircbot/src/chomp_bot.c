/* this is an example using the ircBOT-Library...
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
#include "string.h"
BOT *myBOT = NULL;
const char host[] 		="195.34.133.70";

#define CMD_TSCHAU "by by"
#define CMD_PART "part"
#define CMD_JOIN "join"
#define CMD_ACTION "action"

int cmd_tschau(MSG *msg){
	if( strcmp(msg->postfix, CMD_TSCHAU) == 0 )
		ircBOT_disconnect(myBOT);
	return IRC_OK;
}
int cmd_part(MSG *msg){
	if( strcmp(msg->postfix, CMD_PART) == 0 )
		ircBOT_part(myBOT, "#carpe_noctem");
	return IRC_OK;
}
int cmd_join(MSG *msg){
	if( strncmp(msg->postfix, CMD_JOIN, strlen(CMD_JOIN)) == 0){
		char * name = msg->postfix;
		name = &name[ strlen(CMD_JOIN) + 1 ];
		ircBOT_join(myBOT, name);
	}
	return IRC_OK;
}
int cmd_action(MSG *msg){
	if( strncmp(msg->postfix, CMD_ACTION, strlen(CMD_ACTION)) == 0) {
		/* format is: "action #channel:msg" */
		char tmp[512];
		char *chanstr = NULL, *smsg = NULL; /* send message */
		char *t = strdup(msg->postfix);
		CHAN *chan = NULL;
		chanstr = &t[ strlen(CMD_ACTION) + 1 ];
		smsg=index(t, ':');
		if(smsg){
			*smsg = '\0';
			smsg++;
			chan = irc_sys_find_chan(myBOT, chanstr);
			if(chan){
				sprintf(tmp,"\001ACTION %s\001", smsg);			
				ircBOT_msg(myBOT, chan, NULL, tmp);
			}
		}
		free(t);
	}
	return IRC_OK;
}

int main(int argc, char * argv[]){
	myBOT = ircBOT_new("chomp_bot", NULL);
	ircBOT_server(myBOT, host, 0);
	
	irc_add_cmd(myBOT, "PRIVMSG", CMD_SEARCH_CMD, -1, cmd_tschau);
	irc_add_cmd(myBOT, "PRIVMSG", CMD_SEARCH_CMD, -1, cmd_part);
	irc_add_cmd(myBOT, "NOTICE", CMD_SEARCH_CMD, -1, cmd_join);
	irc_add_cmd(myBOT, "NOTICE", CMD_SEARCH_CMD, -1, cmd_action);
	
	irc_eventloop();
	return 0;
}
