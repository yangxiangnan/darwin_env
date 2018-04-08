/*
 * Copyright (c) 2001-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
 
#ifndef _IOPMPOWERSTATEQUEUE_H_
#define _IOPMPOWERSTATEQUEUE_H_
 
#include <IOKit/IOEventSource.h>
#include <IOKit/IOLocks.h>
#include <kern/queue.h>

typedef void (*IOPMPowerStateQueueAction)(OSObject *, uint32_t event, void *, void *);

class IOPMPowerStateQueue : public IOEventSource
{
    OSDeclareDefaultStructors(IOPMPowerStateQueue)

private:
    struct PowerEventEntry {
        queue_chain_t   chain;
        uint32_t        eventType;
        void *          args[2];
    };

    queue_head_t    queueHead;
    IOLock *        queueLock;

protected:
    virtual bool checkForWork( void );
    virtual bool init( OSObject * owner, Action action );

public:
    static IOPMPowerStateQueue * PMPowerStateQueue( OSObject * owner, Action action );

    bool submitPowerEvent( uint32_t eventType, void * arg0 = 0, void * arg1 = 0 );
};

#endif /* _IOPMPOWERSTATEQUEUE_H_ */
