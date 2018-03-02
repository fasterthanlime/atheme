/*
 * Copyright (c) 2005 William Pitcock, et al.
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains code for the CService TOPIC functions.
 */

#include "atheme.h"

static void cs_cmd_topic(struct sourceinfo *si, int parc, char *parv[]);
static void cs_cmd_topicappend(struct sourceinfo *si, int parc, char *parv[]);
static void cs_cmd_topicprepend(struct sourceinfo *si, int parc, char *parv[]);
static void cs_cmd_topicswap(struct sourceinfo *si, int parc, char *parv[]);

static struct command cs_topic = { "TOPIC", N_("Sets a topic on a channel."),
                        AC_NONE, 2, cs_cmd_topic, { .path = "cservice/topic" } };
static struct command cs_topicappend = { "TOPICAPPEND", N_("Appends a topic on a channel."),
                        AC_NONE, 2, cs_cmd_topicappend, { .path = "cservice/topicappend" } };
static struct command cs_topicprepend = { "TOPICPREPEND", N_("Prepends a topic on a channel."),
                        AC_NONE, 2, cs_cmd_topicprepend, { .path = "cservice/topicprepend" } };
static struct command cs_topicswap = { "TOPICSWAP", N_("Swap part of the topic on a channel."),
                        AC_NONE, 2, cs_cmd_topicswap, { .path = "cservice/topicswap" } };

static void
cs_cmd_topic(struct sourceinfo *si, int parc, char *parv[])
{
	char *chan = parv[0];
	char *topic = parv[1];
	struct mychan *mc;
	struct channel *c;
	const char *topicsetter;
	time_t prevtopicts;

	if (!chan || !topic)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "TOPIC");
		command_fail(si, fault_needmoreparams, _("Syntax: TOPIC <#channel> <topic>"));
		return;
	}

	mc = mychan_find(chan);
	if (!mc)
	{
		command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), chan);
		return;
	}

	c = channel_find(chan);
	if (!c)
	{
                command_fail(si, fault_nosuch_target, _("\2%s\2 is currently empty."), chan);
                return;
        }

	if (metadata_find(mc, "private:close:closer"))
	{
		command_fail(si, fault_noprivs, _("\2%s\2 is closed."), chan);
		return;
	}

	if (!chanacs_source_has_flag(mc, si, CA_TOPIC))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to perform this operation."));
		return;
	}

	if (!validtopic(topic))
	{
		command_fail(si, fault_badparams, _("The new topic is invalid or too long."));
		return;
	}

	if (si->su != NULL)
		topicsetter = si->su->nick;
	else if (si->smu != NULL)
		topicsetter = entity(si->smu)->name;
	else
		topicsetter = "unknown";
	prevtopicts = c->topicts;
	handle_topic(c, topicsetter, CURRTIME, topic);
	topic_sts(c, si->service->me, topicsetter, CURRTIME, prevtopicts, topic);

	logcommand(si, CMDLOG_DO, "TOPIC: \2%s\2", mc->name);
	if (si->su == NULL || !chanuser_find(c, si->su))
		command_success_nodata(si, _("Topic set to \2%s\2 on \2%s\2."), topic, chan);
}

static void
cs_cmd_topicappend(struct sourceinfo *si, int parc, char *parv[])
{
        char *chan = parv[0];
        char *topic = parv[1];
        struct mychan *mc;
	char topicbuf[BUFSIZE];
	struct channel *c;
	const char *topicsetter;
	time_t prevtopicts;

        if (!chan || !topic)
        {
                command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "TOPICAPPEND");
                command_fail(si, fault_needmoreparams, _("Syntax: TOPICAPPEND <#channel> <topic>"));
                return;
        }

        mc = mychan_find(chan);
        if (!mc)
        {
                command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), chan);
                return;
        }

	c = channel_find(chan);
	if (!c)
	{
                command_fail(si, fault_nosuch_target, _("\2%s\2 is currently empty."), chan);
                return;
        }

        if (!chanacs_source_has_flag(mc, si, CA_TOPIC))
        {
                command_fail(si, fault_noprivs, _("You are not authorized to perform this operation."));
                return;
        }

        if (metadata_find(mc, "private:close:closer"))
	{
		command_fail(si, fault_noprivs, _("\2%s\2 is closed."), chan);
		return;
	}

	topicbuf[0] = '\0';

	if (c->topic)
	{
		mowgli_strlcpy(topicbuf, c->topic, BUFSIZE);
		mowgli_strlcat(topicbuf, " | ", BUFSIZE);
		mowgli_strlcat(topicbuf, topic, BUFSIZE);
	}
	else
		mowgli_strlcpy(topicbuf, topic, BUFSIZE);

	if (!validtopic(topicbuf))
	{
		command_fail(si, fault_badparams, _("The new topic is invalid or too long."));
		return;
	}

	if (si->su != NULL)
		topicsetter = si->su->nick;
	else if (si->smu != NULL)
		topicsetter = entity(si->smu)->name;
	else
		topicsetter = "unknown";
	prevtopicts = c->topicts;
	handle_topic(c, topicsetter, CURRTIME, topicbuf);
	topic_sts(c, si->service->me, topicsetter, CURRTIME, prevtopicts, topicbuf);

	logcommand(si, CMDLOG_DO, "TOPICAPPEND: \2%s\2", mc->name);
	if (si->su == NULL || !chanuser_find(c, si->su))
        	command_success_nodata(si, _("Topic set to \2%s\2 on \2%s\2."), c->topic, chan);
}

static void
cs_cmd_topicprepend(struct sourceinfo *si, int parc, char *parv[])
{
        char *chan = parv[0];
        char *topic = parv[1];
        struct mychan *mc;
	char topicbuf[BUFSIZE];
	struct channel *c;
	const char *topicsetter;
	time_t prevtopicts;

        if (!chan || !topic)
        {
                command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "TOPICPREPEND");
                command_fail(si, fault_needmoreparams, _("Syntax: TOPICPREPEND <#channel> <topic>"));
                return;
        }

        mc = mychan_find(chan);
        if (!mc)
        {
                command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), chan);
                return;
        }

	c = channel_find(chan);
	if (!c)
	{
                command_fail(si, fault_nosuch_target, _("\2%s\2 is currently empty."), chan);
                return;
        }

        if (!chanacs_source_has_flag(mc, si, CA_TOPIC))
        {
                command_fail(si, fault_noprivs, _("You are not authorized to perform this operation."));
                return;
        }

        if (metadata_find(mc, "private:close:closer"))
	{
		command_fail(si, fault_noprivs, _("\2%s\2 is closed."), chan);
		return;
	}

	topicbuf[0] = '\0';

	if (c->topic)
	{
		mowgli_strlcpy(topicbuf, topic, BUFSIZE);
		mowgli_strlcat(topicbuf, " | ", BUFSIZE);
		mowgli_strlcat(topicbuf, c->topic, BUFSIZE);
	}
	else
		mowgli_strlcpy(topicbuf, topic, BUFSIZE);

	if (!validtopic(topicbuf))
	{
		command_fail(si, fault_badparams, _("The new topic is invalid or too long."));
		return;
	}

	if (si->su != NULL)
		topicsetter = si->su->nick;
	else if (si->smu != NULL)
		topicsetter = entity(si->smu)->name;
	else
		topicsetter = "unknown";
	prevtopicts = c->topicts;
	handle_topic(c, topicsetter, CURRTIME, topicbuf);
	topic_sts(c, si->service->me, topicsetter, CURRTIME, prevtopicts, topicbuf);

	logcommand(si, CMDLOG_DO, "TOPICPREPEND: \2%s\2", mc->name);
	if (si->su == NULL || !chanuser_find(c, si->su))
        	command_success_nodata(si, _("Topic set to \2%s\2 on \2%s\2."), c->topic, chan);
}

static void
cs_cmd_topicswap(struct sourceinfo *si, int parc, char *parv[])
{
	char *chan = parv[0];
	char *topic = parv[1];
	struct mychan *mc;
	struct channel *c;
	const char *topicsetter;
	time_t prevtopicts;

	char topicbuf[BUFSIZE];
	char commbuf[BUFSIZE];
	char *pos = NULL;
	char *search = NULL;
	char *replace = NULL;
	size_t search_size = 0;
	size_t replace_size = 0;
	size_t copylen = 0;

	if (parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "TOPICSWAP");
		command_fail(si, fault_needmoreparams, _("Syntax: TOPICSWAP <#channel> <search>:[<replace>]"));
		return;
	}

	mowgli_strlcpy(commbuf, parv[1], BUFSIZE);
	search = commbuf;
	pos = strrchr(commbuf, ':');

	if (!pos || pos == commbuf)
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "TOPICSWAP");
		command_fail(si, fault_badparams, _("Syntax: TOPICSWAP <#channel> <search>:[<replace>]"));
		return;
	}

	mc = mychan_find(chan);
	if (!mc)
	{
		command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), chan);
		return;
	}

	c = channel_find(chan);
	if (!c)
	{
		command_fail(si, fault_nosuch_target, _("\2%s\2 is currently empty."), chan);
		return;
	}

	if (!chanacs_source_has_flag(mc, si, CA_TOPIC))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to perform this operation."));
		return;
	}

	if (metadata_find(mc, "private:close:closer"))
	{
		command_fail(si, fault_noprivs, _("\2%s\2 is closed."), chan);
		return;
	}

	if (!c->topic)
		topicbuf[0] = '\0';
	else
		mowgli_strlcpy(topicbuf, c->topic, BUFSIZE);

	*pos = '\0';
	replace = pos + 1;
	search_size = strlen(search);
	replace_size = strlen(replace);

	pos = strstr(topicbuf, search);
	if (!pos)
	{
		command_fail(si, fault_badparams, _("Channel \2%s\2 does not have \2%s\2 in its topic."), chan, search);
		return;
	}

	copylen = strlen(pos + search_size) + 1;
	if (pos - topicbuf + replace_size + copylen > BUFSIZE)
		goto invalid_error;

	memmove(pos + search_size + (replace_size - search_size), pos + search_size, copylen);
	memcpy(pos, replace, replace_size);

	if (!validtopic(topicbuf))
	{
invalid_error:
		command_fail(si, fault_badparams, _("The new topic is invalid or too long."));
		return;
	}

	if (si->su != NULL)
		topicsetter = si->su->nick;
	else if (si->smu != NULL)
		topicsetter = entity(si->smu)->name;
	else
		topicsetter = "unknown";
	prevtopicts = c->topicts;
	handle_topic(c, topicsetter, CURRTIME, topicbuf);
	topic_sts(c, si->service->me, topicsetter, CURRTIME, prevtopicts, topicbuf);

	logcommand(si, CMDLOG_DO, "TOPICSWAP: \2%s\2", mc->name);
	if (si->su == NULL || !chanuser_find(c, si->su))
		command_success_nodata(si, _("Topic set to \2%s\2 on \2%s\2."), c->topic, chan);
}

static void
mod_init(struct module ATHEME_VATTR_UNUSED *const restrict m)
{
        service_named_bind_command("chanserv", &cs_topic);
        service_named_bind_command("chanserv", &cs_topicappend);
        service_named_bind_command("chanserv", &cs_topicprepend);
        service_named_bind_command("chanserv", &cs_topicswap);
}

static void
mod_deinit(const enum module_unload_intent ATHEME_VATTR_UNUSED intent)
{
	service_named_unbind_command("chanserv", &cs_topic);
	service_named_unbind_command("chanserv", &cs_topicappend);
	service_named_unbind_command("chanserv", &cs_topicprepend);
	service_named_unbind_command("chanserv", &cs_topicswap);
}

SIMPLE_DECLARE_MODULE_V1("chanserv/topic", MODULE_UNLOAD_CAPABILITY_OK)
