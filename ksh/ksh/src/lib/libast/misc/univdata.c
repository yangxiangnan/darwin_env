/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2007 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * universe common data
 */

#include "univlib.h"

#ifndef UNIV_MAX

char		univ_env[] = "__UNIVERSE__";

#else

#ifndef NUMUNIV

#if !_lib_universe
#undef	U_GET
#endif

#ifdef	U_GET
char*		univ_name[] = { "ucb", "att" };
#else
char*		univ_name[] = { "att", "ucb" };
#endif

int		univ_max = sizeof(univ_name) / sizeof(univ_name[0]);

#endif

char		univ_cond[] = "$(UNIVERSE)";

int		univ_size = sizeof(univ_cond) - 1;

#endif
