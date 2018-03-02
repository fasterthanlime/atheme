/*
 * Copyright (c) 2005-2006 Atheme Development Group
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains functionality which implements the HelpServ SERVICES command.
 */

#include "atheme.h"
#include "uplink.h"

static void helpserv_cmd_services(struct sourceinfo *si, int parc, char *parv[]);

static struct command helpserv_services = { "SERVICES", N_("List all services currently running on the network."), AC_NONE, 1, helpserv_cmd_services, { .path = "helpserv/services" } };

static void
helpserv_cmd_services(struct sourceinfo *si, int parc, char *parv[])
{
	struct service *sptr;
	mowgli_patricia_iteration_state_t state;

	command_success_nodata(si, _("Services running on %s:"), me.netname);

	MOWGLI_PATRICIA_FOREACH(sptr, &state, services_nick)
	{
		if (!sptr->botonly)
			command_success_nodata(si, "%s", sptr->nick);
	}

	command_success_nodata(si, _("More information on each service is available by messaging it like so: /msg service help"));

	return;
}

static void
mod_init(struct module ATHEME_VATTR_UNUSED *const restrict m)
{
	service_named_bind_command("helpserv", &helpserv_services);
}

static void
mod_deinit(const enum module_unload_intent ATHEME_VATTR_UNUSED intent)
{
	service_named_unbind_command("helpserv", &helpserv_services);
}

SIMPLE_DECLARE_MODULE_V1("helpserv/services", MODULE_UNLOAD_CAPABILITY_OK)
