/*
 *              readconf.cpp
 *
 * Routines for reading talkd.conf and .talkdrc
 *
 * Copyright 1999-2002, David Faure, faure@kde.org
 *
 *  Redistribution and use in source and binary forms, with or
 *    without modification, are permitted provided that the following
 *    conditions are met:
 *
 *    - Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *    PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 * (BSD License, from kdelibs/doc/common/bsd-license.html)
 *
 */

#include <pwd.h>
#include <sys/types.h> 
#include <fcntl.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "proto.h"
#include "readconf.h"
#include "defs.h"

FILE * fd;

int booleanresult(const char * s)
{
     if (strlen(s)==1)
	  { return atoi(s); }
     else if ((!strncasecmp(s,"on",2))||(!strncasecmp(s,"true",4))) {return 1;}
     else if ((!strncasecmp(s,"off",3))||(!strncasecmp(s,"false",5))){return 0;}
     else {
	  syslog(LOG_ERR,"Wrong boolean value %s in %s",s,TALKD_CONF);
	  return 0;
     }
}

/* obsolete routine, but still used for .talkdrc */
int read_user_config(const char * key, char * result, int max)
{
     char * value;
     char * buff = new char[S_CFGLINE];
     char * ret;
     fseek(fd,0,SEEK_SET);
     do {
	  ret = fgets(buff,S_CFGLINE,fd);
     } while ((ret) && (strncasecmp(buff,key,strlen(key))));
     if (ret) {
	  value = strchr(buff,':')+1;
	  while (isspace(*value)) value++; /* get rid of spaces, tabs... */
	  strncpy(result,value,max);
          result[max-1]='\0'; /* in case it was longer than max chars */
          char * lastchar = result + strlen(result) - 1; /* points to last char */
	  if (*lastchar=='\n') *lastchar = '\0'; /* get rid of \n */
     }
     delete buff;
     if (Options.debug_mode) {
	if (ret)
          syslog(LOG_DEBUG,"read_user_config : %s, %s",key,result);
        else 
          syslog(LOG_DEBUG,"read_user_config : %s -> not found",key);
     }
     return (ret) ? 1 : 0;
}

int read_bool_user_config(const char * key, int * result)
{
    char msgtmpl[S_CFGLINE];
    int ret = read_user_config(key, msgtmpl, S_CFGLINE); // ret=1 if found
    if (ret!=0) *result = booleanresult(msgtmpl);
    return ret;
}

int init_user_config(const char * l_name)
{
#define S_MSGPATH 50
     char msgpath[S_MSGPATH];
     struct passwd * pw = getpwnam(l_name);
     if (!pw) return 0;
     else {
       snprintf(msgpath, S_MSGPATH, "%s/.talkdrc", pw->pw_dir);
       endpwent();
       fd=fopen(msgpath, "r");
       return (fd != 0);
     }
}

void end_user_config()
{
  fclose(fd);
}

/* routine for reading talkd.conf */

int process_config_file(void)
{
     FILE * fd = fopen(TALKD_CONF,"r");
     char * ret;
     char buff[S_CFGLINE];
     char * result;

#define found(k) (!strncasecmp(buff,k,strlen(k)))

     if (!fd) { return 0; }
     do {
	  ret = fgets(buff,S_CFGLINE,fd);
	  if ((ret) && (*buff!='#') && ((result = strchr(buff,':')))) {
	       result++;
	       while (isspace(*result)) 
		    result++; /* get rid of spaces, tabs... */
	       result[strlen(result)-1]='\0'; /* get rid of \n */
		 
	       if (found("AnswMach:")) {
		    Options.answmach=booleanresult(result); 
		    debug("AnswMach : %d",Options.answmach);}
		 
	       if (found("XAnnounce:")) {
		    Options.XAnnounce=booleanresult(result); 
		    debug("XAnnounce : %d",Options.XAnnounce); }
	    
	       if (found("Time:")) { 
		    Options.time_before_answmach=atoi(result); 
		    debug("Time : %d",Options.time_before_answmach); }

	       if (found("Sound:")) { 
		    Options.sound=booleanresult(result);
		    debug("Sound : %d",Options.sound); }

	       if (found("SoundFile:")) { 
		    strncpy(Options.soundfile,result,S_CFGLINE);
		    debug("SoundFile =");debug(Options.soundfile); }

	       if (found("SoundPlayer:")) { 
                   strncpy(Options.soundplayer,result,S_CFGLINE);
		    debug("SoundPlayer ="); debug(result); }

	       if (found("SoundPlayerOpt:")) { 
                    strncpy(Options.soundplayeropt,result,S_CFGLINE); 
		    debug("SoundPlayerOpt ="); debug(result); }

	       if (found("MailProg:")) { 
		    strncpy(Options.mailprog,result,S_CFGLINE);
		    debug("Mail prog ="); debug(result); }

               /* text based announcement */
               if (found("Announce1")) { strncpy(Options.announce1,result,S_CFGLINE); }
               if (found("Announce2")) { strncpy(Options.announce2,result,S_CFGLINE); }
               if (found("Announce3")) { strncpy(Options.announce3,result,S_CFGLINE); }
               
               if (found("NEUUser"))   { strncpy(Options.NEU_user,result,S_INVITE_LINES); }
               if (found("NEUBehaviour")) { Options.NEU_behaviour=booleanresult(result); }
               if (found("NEUBanner1")) { strncpy(Options.NEUBanner1,result,S_CFGLINE); }
               if (found("NEUBanner2")) { strncpy(Options.NEUBanner2,result,S_CFGLINE); }
               if (found("NEUBanner3")) { strncpy(Options.NEUBanner3,result,S_CFGLINE); }
               if (found("NEUForwardMethod")) { strncpy(Options.NEU_forwardmethod,result,5); }

	  }
     } while (ret);
     fclose(fd);
     return 1; 
}
