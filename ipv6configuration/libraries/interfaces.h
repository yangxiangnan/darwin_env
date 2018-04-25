#ifndef _S_INTERFACES_H
#define _S_INTERFACES_H
/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * interfaces.h
 * - get the list of inet interfaces in the system
 */

#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <net/if_types.h>

#include "dynarray.h"

#define INDEX_BAD	((int)(-1))

/*
 * Type: interface_t
 * Purpose:
 *   Enclose IP6 and link-level information for a particular
 *   interface.
 */
typedef struct {
    struct in6_addr	addr;
    int			prefixlen;
    int			addr_flags;
    struct in6_addr	prefixmask;
    struct in6_addr	netaddr;
    struct in6_addr	router;
} inet6_addrinfo_t;

#define MAX_LINK_ADDR_LEN	16
typedef struct {
    u_char		addr[MAX_LINK_ADDR_LEN];
    u_short		index;
    u_char		alen;
    u_char		type;
} link_addr_t;

typedef struct {
    char 		name[IFNAMSIZ + 1]; /* eg. en0 */
    short		flags;
    u_char		type;	/* e.g. IFT_ETHER */
    dynarray_t		inet;
    link_addr_t		link_address;
    u_int32_t		user_defined;
} interface_t;

typedef struct {
    interface_t *	list;
    int				count;
    int				size;
} interface_list_t;

/*
 * Functions: ifl_*
 * Purpose:
 *   Interface list routines.
 */
int					ifl_count(interface_list_t * list_p);
interface_t *		ifl_at_index(interface_list_t * list_p, int i);
interface_t *		ifl_find_name(interface_list_t * intface, const char * name);
interface_list_t * 	ifl_init();
void				ifl_free(interface_list_t * * list_p);

/*
 * Functions: if_*
 * Purpose:
 *   Interface-specific routines.
 */
interface_t *		if_dup(interface_t * intface); /* dup an entry */
void				if_free(interface_t * * if_p_p); /* free dup'd entry */
char *				if_name(interface_t * if_p);
short				if_flags(interface_t * if_p);
void				if_setflags(interface_t * if_p, short flags);
void			if_link_update(interface_t * if_p);
void			if_link_copy(interface_t * dest, 
				     const interface_t * source);
int			if_ift_type(interface_t * if_p);


#endif _S_INTERFACES_H
