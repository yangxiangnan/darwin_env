/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * NI.c
 * NetInfo agent for lookupd
 * Written by Marc Majka
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <NetInfo/syslock.h>
#include <NetInfo/system_log.h>
#include <NetInfo/dsutil.h>
#include <NetInfo/nilib2.h>
#include <NetInfo/ni_shared.h>
#include <netinfo/ni.h>
#include <NetInfo/DynaAPI.h>

#define forever for(;;)

static syslock *rpcLock = NULL;

#define DEFAULT_TIMEOUT 30
#define DEFAULT_CONNECT_TIMEOUT 30
#define DEFAULT_LATENCY 30

#define CLIMB_TO_ROOT 0x00000001

#define HOST_LUCATEGORY_STRING "2"
#define USER_LUCATEGORY_STRING "0"
#define NETDOMAIN_LUCATEGORY_STRING "12"

u_int32_t NI_query(void *c, dsrecord *pattern, dsrecord **list);

typedef struct
{
	ni_shared_handle_t *handle;
	u_int32_t checksum;
	u_int32_t checksum_time;
	u_int32_t flags;
} ni_data;

typedef struct
{
	u_int32_t domain_count;
	u_int32_t timeout;
	u_int32_t connect_timeout;
	u_int32_t latency;
	ni_data *domain;
	dynainfo *dyna;
} agent_private;

static char *pathForCategory[] =
{
	"/users",
	"/groups",
	"/machines",
	"/networks",
	"/services",
	"/protocols",
	"/rpcs",
	"/mounts",
	"/printers",
	"/machines",
	"/machines",
	"/aliases",
	"/netap->domains",
	NULL,
	NULL,
	NULL,
	NULL
};

static u_int32_t
unsigned_long_from_record(dsrecord *r, char *key, u_int32_t def)
{
	dsattribute *a;
	dsdata *d;
	u_int32_t x;

	if (r == NULL) return def;
	if (key == NULL) return def;

	d = cstring_to_dsdata(key);
	a = dsrecord_attribute(r, d, SELECT_ATTRIBUTE);
	dsdata_release(d);

	if (a == NULL) return def;

	d = dsattribute_value(a, 0);
	dsattribute_release(a);
	if (d == NULL) return def;
	
	x = atoi(dsdata_to_cstring(d));
	dsdata_release(d);

	return x;
}

static void
NI_configure(agent_private *ap)
{
	int status;
	dsrecord *r;

	if (ap == NULL) return;

	ap->latency = DEFAULT_LATENCY;
	ap->timeout = DEFAULT_TIMEOUT;
	ap->connect_timeout = DEFAULT_CONNECT_TIMEOUT;

	if (ap->dyna == NULL) return;

	if (ap->dyna->dyna_config_global != NULL)
	{
		status = (ap->dyna->dyna_config_global)(ap->dyna, -1, &r);
		if (status == 0)
		{
			ap->latency = unsigned_long_from_record(r, "ValidationLatency", ap->latency);
			ap->timeout = unsigned_long_from_record(r, "Timeout", ap->timeout);
			ap->connect_timeout = unsigned_long_from_record(r, "ConnectTimeout", ap->connect_timeout);
			dsrecord_release(r);
		}
	}

	if (ap->dyna->dyna_config_agent != NULL)
	{
		status = (ap->dyna->dyna_config_agent)(ap->dyna, -1, &r);
		if (status == 0)
		{
			ap->latency = unsigned_long_from_record(r, "ValidationLatency", ap->latency);
			ap->timeout = unsigned_long_from_record(r, "Timeout", ap->timeout);
			ap->connect_timeout = unsigned_long_from_record(r, "ConnectTimeout", ap->connect_timeout);
			dsrecord_release(r);
		}
	}
}

static void
NI_add_domain(agent_private *ap, ni_shared_handle_t *h, u_int32_t f)
{
	sa_setreadtimeout(h, ap->timeout);
	sa_setabort(h, 1);

	syslock_lock(rpcLock);

	sa_setpassword(h, "checksum");
	syslock_unlock(rpcLock);

	if (ap->domain_count == 0)
	{
		ap->domain = (ni_data *)calloc(1, sizeof(ni_data));
	}
	else
	{
		ap->domain = (ni_data *)realloc(ap->domain, (ap->domain_count + 1) * sizeof(ni_data));
	}

	ap->domain[ap->domain_count].handle = h;
	ap->domain[ap->domain_count].checksum = 0;
	ap->domain[ap->domain_count].checksum_time = 0;
	ap->domain[ap->domain_count].flags = f;
	ap->domain_count++;
}

static void
NI_climb_to_root(agent_private *ap)
{
	ni_shared_handle_t *h0, *h1;

	h0 = NULL;

	if (ap->domain_count == 0) 
	{
		syslock_lock(rpcLock);
		h0 = ni_shared_local();
		syslock_unlock(rpcLock);
		if (h0 == NULL) return;
		NI_add_domain(ap, h0, CLIMB_TO_ROOT);
	}

	if (ap->domain_count == 0) return;

	h0 = ap->domain[ap->domain_count - 1].handle;

	if (ap->domain_count > 1)
	{
		sa_setreadtimeout(h0, ap->connect_timeout);
		sa_setabort(h0, 1);
	}

	forever
	{
		syslock_lock(rpcLock);
		h1 = ni_shared_parent(h0);
		syslock_unlock(rpcLock);

		if (h1 == NULL) return;

		h0 = h1;
		NI_add_domain(ap, h0, CLIMB_TO_ROOT);
	}
}

static void
NI_set_source(agent_private *ap, char *arg)
{
	ni_shared_handle_t *h;
	char **list;
	u_int32_t i, len;

	if (arg == NULL)
	{
		NI_climb_to_root(ap);
		return;
	}

	list = explode(arg, ",");
	len = listLength(list);
	for (i = 0; i < len; i++)
	{
		if (streq(list[i], "..."))
		{
			NI_climb_to_root(ap);
		}
		else
		{
			syslock_lock(rpcLock);
			h = ni_shared_open(NULL, list[i]);
			syslock_unlock(rpcLock);

			if (h == NULL) continue;
			NI_add_domain(ap, h, 0);
		}
	}
	freeList(list);
}

u_int32_t
NI_new(void **c, char *args, dynainfo *d)
{
	agent_private *ap;

	if (c == NULL) return 1;
	ap = (agent_private *)calloc(1, sizeof(agent_private));

	system_log(LOG_DEBUG, "Allocated NI 0x%08x", (int)ap);

	*c = ap;
	ap->dyna = d;

	rpcLock = syslock_get(RPCLockName);

	NI_configure(ap);
	NI_set_source(ap, args);

	return 0;
}

u_int32_t
NI_free(void *c)
{
	agent_private *ap;

	if (c == NULL) return 0;

	ap = (agent_private *)c;

	free(ap->domain);
	ap->domain = NULL;
	ap->domain_count = 0;

	system_log(LOG_DEBUG, "Deallocated NI 0x%08x", (int)ap);

	free(ap);
	c = NULL;

	return 0;
}

static u_int32_t
NI_checksum_for_index(agent_private *ap, u_int32_t i)
{
	struct timeval now;
	u_int32_t age;
	ni_status status;
	ni_proplist pl;
	u_int32_t sum;
	ni_index where;

	if (ap == NULL) return 0;

	if (i >= ap->domain_count) return 0;

	gettimeofday(&now, (struct timezone *)NULL);
	age = now.tv_sec - ap->domain[i].checksum_time;
	if (age <= ap->latency) return ap->domain[i].checksum;

	NI_INIT(&pl);

	syslock_lock(rpcLock);
	status = sa_statistics(ap->domain[i].handle, &pl);
	syslock_unlock(rpcLock);

	if (status != NI_OK)
	{
		system_log(LOG_ERR, "ni_statistics: %s", ni_error(status));
		ni_proplist_free(&pl);
		return 0;
	}

	/* checksum should be first (and only!) property */
	where = NI_INDEX_NULL;
	if (pl.ni_proplist_len > 0)
	{
		if (strcmp(pl.ni_proplist_val[0].nip_name, "checksum"))
			where = 0;
		else
			where = ni_proplist_match(pl, "checksum", NULL);
	}

	if (where == NI_INDEX_NULL)
	{
		system_log(LOG_ERR, "ap->domain %lu: can't get checksum", i);
		ni_proplist_free(&pl);
		return 0;
	}

	sscanf(pl.ni_proplist_val[where].nip_val.ni_namelist_val[0], "%lu", (unsigned long *)&sum);
	ni_proplist_free(&pl);

	ap->domain[i].checksum_time = now.tv_sec;
	ap->domain[i].checksum = sum;

	return sum;
}

static void
NI_add_validation(agent_private *ap, dsrecord *r, u_int32_t i)
{
	char str[64];
	dsdata *d;
	dsattribute *a;

	if (r == NULL) return;

	d = cstring_to_dsdata("lookup_validation");
	dsrecord_remove_key(r, d, SELECT_META_ATTRIBUTE);

	a = dsattribute_new(d);
	dsrecord_append_attribute(r, a, SELECT_META_ATTRIBUTE);

	dsdata_release(d);

	sprintf(str, "%lu %lu", (unsigned long)i, (unsigned long)NI_checksum_for_index(ap, i));
	d = cstring_to_dsdata(str);
	dsattribute_append(a, d);
	dsdata_release(d);

	dsattribute_release(a);
}

u_int32_t
NI_validate(void *c, char *v)
{
	agent_private *ap;
	int n;
	u_int32_t i, ci;

	if (c == NULL) return 0;
	if (v == NULL) return 0;

	ap = (agent_private *)c;

	n = sscanf(v, "%lu %lu", (unsigned long *)&i, (unsigned long *)&ci);
	if (n != 2) return 0;

	if (	ci == 0) return 0;
	if (ci == NI_checksum_for_index(ap, i)) return 1;

	return 0;
}

static dsrecord *
nitods(ni_proplist *p)
{
	int pn, vn, len;
	dsrecord *r;
	dsattribute *a;

	if (p == NULL) return NULL;

	r = dsrecord_new();

	r->count = p->ni_proplist_len;

	if (r->count > 0)
	{
		r->attribute = (dsattribute **)malloc(r->count * sizeof(dsattribute *));
	}

	/* for each property */
	for (pn = 0; pn < p->ni_proplist_len; pn++)
	{
		r->attribute[pn] = (dsattribute *)malloc(sizeof(dsattribute));
		a = r->attribute[pn];

		a->retain = 1;

		a->key = cstring_to_dsdata(p->ni_proplist_val[pn].nip_name);

		len = p->ni_proplist_val[pn].nip_val.ni_namelist_len;
		a->count = len;
		a->value = NULL;
		if (len > 0) a->value = (dsdata **)malloc(len * sizeof(dsdata *));

		/* for each value in the namelist for this property */
		for (vn = 0; vn < len; vn++)
		{
			a->value[vn] = cstring_to_dsdata(p->ni_proplist_val[pn].nip_val.ni_namelist_val[vn]);
		}
	}

	return r;
}

/*
 * Fetch all subdirectories and append those that match the pattern to the list.
 */
static u_int32_t
NI_query_all(agent_private *ap, char *path, int single_item, int stamp, dsrecord *pattern, dsrecord **list)
{
	ni_idlist idl;
	ni_proplist pl;
	ni_id dir;
	int dx, i;
	ni_status status;
	dsrecord *r, *lastrec;

	lastrec = NULL;

	for (dx = 0; dx < ap->domain_count; dx++)
	{
		if (stamp == 1)
		{
			r = dsrecord_new();
			NI_add_validation(ap, r, dx);
			if (*list == NULL) *list = r;
			else lastrec->next = r;
			lastrec = r;
			continue;
		}
	
		NI_INIT(&dir);
		syslock_lock(rpcLock);
		status = sa_pathsearch(ap->domain[dx].handle, &dir, path);
		syslock_unlock(rpcLock);
		if (status != NI_OK) continue;

		NI_INIT(&idl);
		syslock_lock(rpcLock);
		status = sa_children(ap->domain[dx].handle, &dir, &idl);
		syslock_unlock(rpcLock);
		if (status != NI_OK) continue;
		
		for (i = 0; i < idl.ni_idlist_len; i++)
		{
			dir.nii_object = idl.ni_idlist_val[i];
			dir.nii_instance = 0;

			NI_INIT(&pl);
			syslock_lock(rpcLock);
			status = sa_read(ap->domain[dx].handle, &dir, &pl);
			syslock_unlock(rpcLock);
			if (status != NI_OK) continue;

			r = nitods(&pl);
			NI_add_validation(ap, r, dx);
			ni_proplist_free(&pl);
			if (r == NULL) continue;

			if (dsrecord_match(r, pattern))
			{
				if (*list == NULL) *list = r;
				else lastrec->next = r;
				lastrec = r;

				if (single_item == 1)
				{
					ni_idlist_free(&idl);
					return 0;
				}

			}
			else
			{
				dsrecord_release(r);
			}
		}

		ni_idlist_free(&idl);
	}
	
	return 0;
}

/*
 * Use ni_lookup to match key=value, 
 * Fetch matching subdirectories and append those that match the pattern to the list.
 */
static u_int32_t
NI_query_lookup(agent_private *ap, char *path, int single_item, u_int32_t where, dsrecord *pattern, dsrecord **list)
{
	ni_idlist idl;
	ni_proplist pl;
	ni_id dir;
	int dx, i, try_realname;
	ni_status status;
	dsrecord *r, *lastrec;
	char *key, *val;
	dsattribute *a;

	lastrec = NULL;

	a = dsattribute_retain(pattern->attribute[where]);
	key = dsdata_to_cstring(a->key);
	val = dsdata_to_cstring(a->value[0]);
	dsrecord_remove_attribute(pattern, a, SELECT_ATTRIBUTE);

	/*
	 * SPECIAL CASE For category user, key "name":
	 * if no matches for "name", try "realname".
	 */
	try_realname = 0;
	if ((!strcmp(path, "/users")) && (!strcmp(key, "name"))) try_realname = 1;

	for (dx = 0; dx < ap->domain_count; dx++)
	{
		NI_INIT(&dir);
		syslock_lock(rpcLock);
		status = sa_pathsearch(ap->domain[dx].handle, &dir, path);
		syslock_unlock(rpcLock);
		if (status != NI_OK) continue;

		NI_INIT(&idl);
		syslock_lock(rpcLock);

		status = sa_lookup(ap->domain[dx].handle, &dir, key, val, &idl);

		if ((idl.ni_idlist_len == 0) && (try_realname == 1))
		{
			status = sa_lookup(ap->domain[dx].handle, &dir, "realname", val, &idl);
		}

		syslock_unlock(rpcLock);
		if (status != NI_OK) continue;
		
		for (i = 0; i < idl.ni_idlist_len; i++)
		{
			dir.nii_object = idl.ni_idlist_val[i];
			dir.nii_instance = 0;

			NI_INIT(&pl);
			syslock_lock(rpcLock);
			status = sa_read(ap->domain[dx].handle, &dir, &pl);
			syslock_unlock(rpcLock);
			if (status != NI_OK) continue;

			r = nitods(&pl);
			NI_add_validation(ap, r, dx);
			ni_proplist_free(&pl);
			if (r == NULL) continue;

			if (dsrecord_match(r, pattern))
			{
				if (*list == NULL) *list = r;
				else lastrec->next = r;
				lastrec = r;

				if (single_item == 1)
				{
					ni_idlist_free(&idl);
					dsattribute_release(a);
					return 0;
				}

			}
			else
			{
				dsrecord_release(r);
			}
		}

		ni_idlist_free(&idl);
	}
	
	dsattribute_release(a);
	return 0;
}

static u_int32_t
NI_query_netgroup(void *c, dsrecord *pattern, dsrecord **list)
{
	u_int32_t status;
	dsattribute *a, *res_ha, *res_ua, *res_da;
	dsdata *k;
	int stamp, i, count;
	dsrecord *ngquery, *res, *hostres, *userres, *domainres, *r;

	if (c == NULL) return 1;
	if (pattern == NULL) return 1;
	if (list == NULL) return 1;

	*list = NULL;
	stamp = 0;

	k = cstring_to_dsdata(SINGLE_KEY);
	a = dsrecord_attribute(pattern, k, SELECT_META_ATTRIBUTE);
	dsdata_release(k);
	if (a != NULL)
	{
		dsrecord_remove_attribute(pattern, a, SELECT_META_ATTRIBUTE);
	}
	dsattribute_release(a);

	/* Check if the caller desires a validation stamp */
	k = cstring_to_dsdata(STAMP_KEY);
	a = dsrecord_attribute(pattern, k, SELECT_META_ATTRIBUTE);
	dsdata_release(k);
	if (a != NULL)
	{
		dsrecord_remove_attribute(pattern, a, SELECT_META_ATTRIBUTE);
		stamp = 1;
	}
	dsattribute_release(a);

	if (stamp == 1)
	{
		ngquery = dsrecord_new();

		a = dsattribute_from_cstrings(CATEGORY_KEY, HOST_LUCATEGORY_STRING, NULL);
		dsrecord_append_attribute(ngquery, a, SELECT_META_ATTRIBUTE);
		dsattribute_release(a);

		a = dsattribute_from_cstrings(STAMP_KEY, "1", NULL);
		dsrecord_append_attribute(ngquery, a, SELECT_META_ATTRIBUTE);
		dsattribute_release(a);

		status = NI_query(c, ngquery, list);
		dsrecord_release(ngquery);
		return status;
	}

	count = 0;
	res = dsrecord_new();
	res_ha = dsattribute_from_cstrings("hosts", NULL);
	dsrecord_append_attribute(res, res_ha, SELECT_ATTRIBUTE);
	res_ua = dsattribute_from_cstrings("users", NULL);
	dsrecord_append_attribute(res, res_ua, SELECT_ATTRIBUTE);
	res_da = dsattribute_from_cstrings("domains", NULL);
	dsrecord_append_attribute(res, res_da, SELECT_ATTRIBUTE);

	/* Find hosts with netgroups=<val> */
	ngquery = dsrecord_copy(pattern);

	a = dsattribute_from_cstrings(CATEGORY_KEY, HOST_LUCATEGORY_STRING, NULL);
	dsrecord_append_attribute(ngquery, a, SELECT_META_ATTRIBUTE);

	k = cstring_to_dsdata("name");
	a = dsrecord_attribute(ngquery, k, SELECT_ATTRIBUTE);
	dsdata_release(k);
	k = cstring_to_dsdata("netgroups");
	dsattribute_setkey(a, k);
	dsdata_release(k);
	dsattribute_release(a);

	hostres = NULL;
	status = NI_query(c, ngquery, &hostres);
	dsrecord_release(ngquery);

	/* Find users with netgroups=<val> */
	ngquery = dsrecord_copy(pattern);
	
	a = dsattribute_from_cstrings(CATEGORY_KEY, USER_LUCATEGORY_STRING, NULL);
	dsrecord_append_attribute(ngquery, a, SELECT_META_ATTRIBUTE);
	
	k = cstring_to_dsdata("name");
	a = dsrecord_attribute(ngquery, k, SELECT_ATTRIBUTE);
	dsdata_release(k);
	k = cstring_to_dsdata("netgroups");
	dsattribute_setkey(a, k);
	dsdata_release(k);
	dsattribute_release(a);

	userres = NULL;
	status = NI_query(c, ngquery, &userres);
	dsrecord_release(ngquery);
	
	/* Find netdomains with netgroups=<val> */
	ngquery = dsrecord_copy(pattern);
	
	a = dsattribute_from_cstrings(CATEGORY_KEY, NETDOMAIN_LUCATEGORY_STRING, NULL);
	dsrecord_append_attribute(ngquery, a, SELECT_META_ATTRIBUTE);
	
	k = cstring_to_dsdata("name");
	a = dsrecord_attribute(ngquery, k, SELECT_ATTRIBUTE);
	dsdata_release(k);
	k = cstring_to_dsdata("netgroups");
	dsattribute_setkey(a, k);
	dsdata_release(k);
	dsattribute_release(a);
	domainres = NULL;
	status = NI_query(c, ngquery, &domainres);
	dsrecord_release(ngquery);

	k = cstring_to_dsdata("name");

	/* Merge host names */
	for (r = hostres; r != NULL; r = r->next)
	{
		a = dsrecord_attribute(r, k, SELECT_ATTRIBUTE);
		if (a == NULL) continue;
		for (i = 0; i < a->count; i++) dsattribute_merge(res_ha, a->value[i]);
		count += a->count;
		dsattribute_release(a);
	}	
	
	dsrecord_release(hostres);
	dsattribute_release(res_ha);
	
	/* Merge user names */
	for (r = userres; r != NULL; r = r->next)
	{
		a = dsrecord_attribute(r, k, SELECT_ATTRIBUTE);
		if (a == NULL) continue;
		for (i = 0; i < a->count; i++) dsattribute_merge(res_ua, a->value[i]);
		count += a->count;
		dsattribute_release(a);
	}	
	
	dsrecord_release(userres);
	dsattribute_release(res_ua);
	
	/* Merge netdomain names */
	for (r = domainres; r != NULL; r = r->next)
	{
		a = dsrecord_attribute(r, k, SELECT_ATTRIBUTE);
		if (a == NULL) continue;
		for (i = 0; i < a->count; i++) dsattribute_merge(res_da, a->value[i]);
		count += a->count;
		dsattribute_release(a);
	}	
	
	dsrecord_release(domainres);
	dsattribute_release(res_da);

	dsdata_release(k);

	if (count > 0)
	{
		*list = res;
		status = 0;
	}
	else
	{
		dsrecord_release(res);
		status = 1;
	}

	return status;
}

u_int32_t
NI_query(void *c, dsrecord *pattern, dsrecord **list)
{
	agent_private *ap;
	u_int32_t cat, i, wname, wkey, status;
	dsattribute *a;
	dsdata *k;
	int single_item, stamp;
	char *path, *str, *catname;

	if (c == NULL) return 1;
	if (pattern == NULL) return 1;
	if (list == NULL) return 1;

	*list = NULL;
	single_item = 0;
	stamp = 0;

	ap = (agent_private *)c;

	/* Determine the category */
	k = cstring_to_dsdata(CATEGORY_KEY);
	a = dsrecord_attribute(pattern, k, SELECT_META_ATTRIBUTE);
	dsdata_release(k);

	if (a == NULL) return 1;
	if (a->count == 0) return 1;
	dsrecord_remove_attribute(pattern, a, SELECT_META_ATTRIBUTE);

	catname = dsdata_to_cstring(a->value[0]);
	if (catname == NULL) return 1;

	str = NULL;
	if (catname[0] == '/')
	{
		cat = -1;
		str = catname;
	}
	else
	{
		cat = atoi(catname);
		if (cat == LUCategoryNetgroup)
		{
			dsattribute_release(a);
			return NI_query_netgroup(c, pattern, list);
		}

		str = pathForCategory[cat];
		if (str == NULL)
		{
			dsattribute_release(a);
			return 1;
		}
	}

	path = strdup(str);
	dsattribute_release(a);

	if ((ap->domain_count == 0) || ((ap->domain[ap->domain_count - 1].flags & CLIMB_TO_ROOT) != 0))
	{
		/* Re-check the (current) top-level domain for a parent */
		NI_climb_to_root(ap);
	}

	if (ap->domain_count == 0)
	{
		free(path);
		return 1;
	}

	/* Check if the caller desires a validation stamp */
	k = cstring_to_dsdata(STAMP_KEY);
	a = dsrecord_attribute(pattern, k, SELECT_META_ATTRIBUTE);
	dsdata_release(k);
	if (a != NULL)
	{
		dsrecord_remove_attribute(pattern, a, SELECT_META_ATTRIBUTE);
		stamp = 1;
	}
	dsattribute_release(a);

	/* Check if the caller desires a single record */
	k = cstring_to_dsdata(SINGLE_KEY);
	a = dsrecord_attribute(pattern, k, SELECT_META_ATTRIBUTE);
	dsdata_release(k);
	if (a != NULL)
	{
		dsrecord_remove_attribute(pattern, a, SELECT_META_ATTRIBUTE);
		single_item = 1;
	}
	dsattribute_release(a);

	/* Check the pattern */
	if ((pattern->count == 0) || (stamp == 1))
	{
		status = NI_query_all(ap, path, single_item, stamp, pattern, list);
		free(path);
		return status;
	}

	wkey = IndexNull;

	/* Prefer to search for "name"=<val> */
	k = cstring_to_dsdata("name");
	wname = dsrecord_attribute_index(pattern, k, SELECT_ATTRIBUTE);
	dsdata_release(k);
	if (wname != IndexNull)
	{
		if (pattern->attribute[wname]->count != 0) wkey = wname;
	}

	/* Look for a <key>=<val> attribute */
	for (i = 0; (i < pattern->count) && (wkey == IndexNull); i++)
	{
		if (pattern->attribute[i]->count != 0) wkey = i;
	}

	if (wkey == IndexNull)
	{
		status = NI_query_all(ap, path, single_item, stamp, pattern, list);
		free(path);
		return status;
	}

	status = NI_query_lookup(ap, path, single_item, wkey, pattern, list);
	free(path);
	return status;
}
