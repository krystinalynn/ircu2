/*
 * IRC - Internet Relay Chat, ircd/m_proto.c
 * Copyright (C) 1990 Jarkko Oikarinen and
 *                    University of Oulu, Computing Center
 *
 * See file AUTHORS in IRC package for additional names of
 * the programmers.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 */
#include "ircd_reply.h"
#include "client.h"
#include "ircd.h"
#include "ircd_snprintf.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_debug.h"
#include "send.h"

#include <assert.h>

int need_more_params(struct Client* cptr, const char* cmd)
{
  send_reply(cptr, ERR_NEEDMOREPARAMS, cmd);
  return 0;
}

/*
 * send_error_to_client - send an error message to a client
 * I don't know if this function is any faster than the other version
 * but it is a bit easier to use. It's reentrant until it hits vsendto_one
 * at least :) --Bleep
 */
int send_error_to_client(struct Client* cptr, int error, ...)
{
  va_list               vl;
  char                  buf[BUFSIZE];
  char*                 dest = buf;
  const char*           src  = me.name;
  const struct Numeric* num  = get_error_numeric(error);

  assert(0 != cptr);
  assert(0 != num);
  /*
   * prefix
   */
  *dest++ = ':';
  while ((*dest = *src++))
    ++dest;
  *dest++ = ' ';
  /*
   * numeric
   */
  src = num->str;
  while ((*dest = *src++))
    ++dest;
  *dest++ = ' ';
  /*
   * client name (nick)
   */
  src = cptr->name;
  while ((*dest = *src++))
    ++dest;
  *dest++ = ' ';
  /*
   * reply format
   */
  strcpy(dest, num->format);

#if 0
  Debug((DEBUG_INFO, "send_error_to_client: format: ->%s<-", buf));
#endif

  va_start(vl, error);
  vsendto_one(cptr, buf, vl);
  va_end(vl);
  return 0;
}


int send_reply(struct Client *to, int reply, ...)
{
  struct VarData vd;
  char sndbuf[IRC_BUFSIZE];
  const struct Numeric *num;

  assert(0 != to);
  assert(0 != reply);

  num = get_error_numeric(reply); /* get information about reply... */

  vd.vd_format = num->format; /* select format... */

  /* build buffer */
  va_start(vd.vd_args, reply);
  if (MyUser(to))
    ircd_snprintf(to, sndbuf, sizeof(sndbuf) - 2, ":%s %s %C %v", me.name,
		  num->str, to, &vd);
  else
    ircd_snprintf(to, sndbuf, sizeof(sndbuf) - 2, "%C %s %C %v", &me, num->str,
		  to, &vd);
  va_end(vd.vd_args);

  /* send it to the user */
  send_buffer(to, sndbuf);

  return 0; /* convenience return */
}

int send_admin_info(struct Client* sptr, const struct ConfItem* admin)
{
  assert(0 != sptr);
  if (admin) {
    send_reply(sptr, RPL_ADMINME,    me.name);
    send_reply(sptr, RPL_ADMINLOC1,  admin->host);
    send_reply(sptr, RPL_ADMINLOC2,  admin->passwd);
    send_reply(sptr, RPL_ADMINEMAIL, admin->name);
  }
  else
    send_reply(sptr, ERR_NOADMININFO, me.name);
  return 0;
}


