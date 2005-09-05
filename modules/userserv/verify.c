/*
 * Copyright (c) 2005 Atheme Development Group
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains code for the NickServ VERIFY function.
 *
 * $Id: verify.c 2171 2005-09-05 21:22:11Z jilles $
 */

#include "atheme.h"

DECLARE_MODULE_V1
(
	"nickserv/verify", FALSE, _modinit, _moddeinit,
	"$Id: verify.c 2171 2005-09-05 21:22:11Z jilles $",
	"Atheme Development Group <http://www.atheme.org>"
);

static void ns_cmd_verify(char *origin);

command_t ns_verify = { "VERIFY", "Verifies a nickname registration.", AC_NONE, ns_cmd_verify };

list_t *ns_cmdtree;

void _modinit(module_t *m)
{
	ns_cmdtree = module_locate_symbol("nickserv/main", "ns_cmdtree");
	command_add(&ns_verify, ns_cmdtree);
}

void _moddeinit()
{
	command_delete(&ns_verify, ns_cmdtree);
}

static void ns_cmd_verify(char *origin)
{
	myuser_t *mu;
	metadata_t *md;
	user_t *u = user_find(origin);
	char *op = strtok(NULL, " ");
	char *nick = strtok(NULL, " ");
	char *key = strtok(NULL, " ");

	if (!op || !nick || !key)
	{
		notice(nicksvs.nick, origin, "Insufficient parameters specified for \2VERIFY\2.");
		notice(nicksvs.nick, origin, "Syntax: VERIFY <operation> <nickname> <key>");
		return;
	}

	if (!(mu = myuser_find(nick)))
	{
		notice(nicksvs.nick, origin, "\2%s\2 is not registered.", nick);
		return;
	}

	/* forcing users to log in before we verify
	 * prevents some information leaks
	 */
	if (!(u->myuser == mu))
	{
		notice(nicksvs.nick, origin, "Please log in before attempting to verify your registration.");
		return;
	}

	if (!strcasecmp(op, "REGISTER"))
	{
		metadata_t *md;

		if (!(mu->flags & MU_WAITAUTH) || !(md = metadata_find(mu, METADATA_USER, "private:verify:register:key")))
		{
			notice(nicksvs.nick, origin, "\2%s\2 is not awaiting authorization.", nick);
			return;
		}

		if (!strcasecmp(key, md->value))
		{
			mu->flags &= ~MU_WAITAUTH;

			snoop("REGISTER:VS: \2%s\2 by \2%s\2", mu->email, origin);

			metadata_delete(mu, METADATA_USER, "private:verify:register:key");
			metadata_delete(mu, METADATA_USER, "private:verify:register:timestamp");

			notice(nicksvs.nick, origin, "\2%s\2 has now been verified.", mu->name);
			notice(nicksvs.nick, origin, "Thank you for verifying your e-mail address! You have taken steps in "
				"ensuring that your registrations are not exploited.");
			ircd_on_login(origin, mu->name, NULL);

			return;
		}

		snoop("REGISTER:VF: \2%s\2 by \2%s\2", mu->email, origin);
		notice(nicksvs.nick, origin, "Verification failed. Invalid key for \2%s\2.", mu->name);

		return;
	}
	else if (!strcasecmp(op, "EMAILCHG"))
	{
		if (!(md = metadata_find(mu, METADATA_USER, "private:verify:emailchg:key")))
		{
			notice(nicksvs.nick, origin, "\2%s\2 is not awaiting authorization.", nick);
			return;
		}

		if (!strcasecmp(key, md->value))
                {
			md = metadata_find(mu, METADATA_USER, "private:verify:emailchg:newemail");

			strlcpy(mu->email, md->value, EMAILLEN);

			snoop("SET:EMAIL:VS: \2%s\2 by \2%s\2", mu->email, origin);

			metadata_delete(mu, METADATA_USER, "private:verify:emailchg:key");
			metadata_delete(mu, METADATA_USER, "private:verify:emailchg:newemail");
			metadata_delete(mu, METADATA_USER, "private:verify:emailchg:timestamp");

			notice(nicksvs.nick, origin, "\2%s\2 has now been verified.", mu->email);

			return;
                }

		snoop("REGISTER:VF: \2%s\2 by \2%s\2", mu->email, origin);
		notice(nicksvs.nick, origin, "Verification failed. Invalid key for \2%s\2.", mu->name);

		return;
	}
	else
	{
		notice(nicksvs.nick, origin, "Invalid operation specified for \2VERIFY\2.");
		notice(nicksvs.nick, origin, "Please double-check your verification e-mail.");
		return;
	}
}
