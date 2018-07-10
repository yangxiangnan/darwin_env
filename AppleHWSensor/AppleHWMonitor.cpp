/*
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 2002-2003 Apple Computer, Inc.  All rights reserved.
 *
 */
//		$Log: AppleHWMonitor.cpp,v $
//		Revision 1.4  2004/02/12 01:17:01  eem
//		Merge Rohan changes from tag MERGED-FROM-rohan-branch-TO-TOT-1
//		
//		Revision 1.3  2004/01/30 23:52:00  eem
//		[3542678] IOHWSensor/IOHWControl should use "reg" with version 2 thermal parameters
//		Remove AppleSMUSensor/AppleSMUFan since that code will be in AppleSMUDevice class.
//		Fix IOHWMonitor, AppleMaxim6690, AppleAD741x to use setPowerState() API instead of
//		unsynchronized powerStateWIllChangeTo() API.
//		
//		Revision 1.2.4.1  2004/02/10 09:58:01  eem
//		3548562, 3554178 - prevent extra OSNumber allocations
//		
//		Revision 1.2  2003/12/02 02:02:28  tsherman
//		3497295 - Q42 Task - AppleHWSensor's AppleLM8x (NEW) driver needs to be revised to comply with Thermal API
//		
//		Revision 1.1  2003/10/23 20:08:18  wgulland
//		Adding IOHWControl and a base class for IOHWSensor and IOHWControl
//		
//		

#include <sys/cdefs.h>

#include "AppleHWMonitor.h"


OSDefineMetaClass( IOHWMonitor, IOService )
OSDefineAbstractStructors(IOHWMonitor, IOService)

bool IOHWMonitor::systemIsRestarting = FALSE; // instantiate static member systemIsRestarting and reflect restart state as false

static const IOPMPowerState ourPowerStates[kIOHWMonitorNumPowerStates] = 
{
	{kIOPMPowerStateVersion1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{kIOPMPowerStateVersion1, kIOPMDeviceUsable, IOPMPowerOn, IOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

#ifdef APPLEHWMONITOR_DEBUG
void IOHWMonitor::initDebugID( IOService *provider )
{
	IOService *provider2, *provider3;
	OSData *regData;
	const char *name;
	UInt32 reg;

	// failure case
	fDebugID[0] = '\0';

	// get pointer to device node
	if ((provider2 = provider->getProvider()) == NULL ||
	    (provider3 = provider2->getProvider()) == NULL)
		return;

	if ((name = provider3->getName()) == NULL)
		return;

	if ((regData = OSDynamicCast(OSData, provider3->getProperty("reg"))) == NULL)
		return;

	reg = *((UInt32 *)regData->getBytesNoCopy());

	// build id string
	sprintf(fDebugID, "%s@%lx", name, reg);
}
#endif

bool IOHWMonitor::start(IOService *provider)
{
	mach_timespec_t 	WaitTimeOut;
	IOService *service;

    if ( !(IOService::start(provider)) )
        return false;

#ifdef APPLEHWMONITOR_DEBUG
	initDebugID(provider);
#endif

	DLOG("IOHWMonitor::start(%s) - entered\n", fDebugID);

	OSObject *obj;
	OSNumber *num;
	OSData *data;
	char *ptr, type[32];
 
	// IOHWSensor/IOHWControl property: "version"
	// Description: Version of sensor/control node
	// Input Format: OSData
	// Output Format: OSNumber
    // Required: Yes
    data = OSDynamicCast(OSData, provider->getProperty("version"));
    if (!data)
    {
        IOLog("IOHWMonitor - no Params Version !!\n");
        return false;
    }

    UInt32 version;
    version = *(UInt32 *)data->getBytesNoCopy();
    num = OSNumber::withNumber(version, 32);
    if (!num)
    {
        IOLog("IOHWMonitor - can't set Params Version !!\n");
        return false;
    }

    setProperty("version", num);
    num->release();


	// IOHWSensor/IOHWControl property: "zone"
	// Description: Zone ID of the sensor/control
	// Input Format: OSData
	// Output Format: OSData
    // Required: Yes
    obj = provider->getProperty("zone");
    if (!obj)
    {
        IOLog("IOHWMonitor - no Zone !!\n");
        return false;
    }
	setProperty("zone", obj);

	// IOHWSensor/IOHWControl property: "location"
	// Description: Descriptive location of the sensor/control
	// Input Format: OSData/OSString
	// Output Format: OSString
    // Required: Yes
    obj = provider->getProperty("location");
    if (!obj)
    {
        IOLog("IOHWMonitor - no Location !!\n");
        return false;
    }

	data = OSDynamicCast(OSData, obj); // object type OSData?
	if (data)
	{
		ptr = (char *)data->getBytesNoCopy();
		DLOG("IOHWMonitor::start(%s) - found location '%s'\n", fDebugID, ptr);
		setProperty("location", ptr); // set as OSString
    }
	else
		setProperty("location", obj);

	// IOHWSensor/IOHWControl property: "type"
	// Description: Type of the sensor/control (ie: temp, adc ...)
	// Input Format: OSData
	// Output Format: OSString
    // Required: Yes
    obj = provider->getProperty("type");
    if (!obj)
    {
		obj = provider->getProperty("device_type"); // if no type, parse device_type
		data = OSDynamicCast(OSData, obj);
		if (!data)
		{
			IOLog("IOHWMonitor - no Type !!\n");
			return false;
		}
		
		ptr = (char *)data->getBytesNoCopy();
		strcpy(type, ptr);
		DLOG("IOHWMonitor::start(%s) - found device_type '%s'\n", fDebugID, type);

		for(unsigned int i = strlen(type); i >= 0; i--)
		{
			if (type[i] == '-')
			{
				type[i] = '\0';
				break;
			}
		}

		setProperty("type", type);

	}
	else
		setProperty("type", obj);

	DLOG("IOHWMonitor::start(%s) - waiting for pmon\n", fDebugID);

	// find the platform monitor and tell it that we exist
	// Use a 10 second timeout when calling waitForService()
    WaitTimeOut.tv_sec = 10;
    WaitTimeOut.tv_nsec = 0;

	service = waitForService(resourceMatching("IOPlatformMonitor"), &WaitTimeOut);

	// if there's no IOPlatformMonitor, try IOPlatformPlugin
    if(!service)
	{
		service = waitForService(resourceMatching("IOPlatformPlugin"), &WaitTimeOut);
	}

	if (service)
        fIOPMon = OSDynamicCast (IOService, service->getProperty("IOPlatformMonitor"));
    
	// Initialize Power Management superclass variables from IOService.h
    PMinit();

	// Register as controlling driver from IOService.h
    IOReturn status = registerPowerDriver( this, (IOPMPowerState *) ourPowerStates, kIOHWMonitorNumPowerStates );
	if (status != kIOReturnSuccess)
	{
		IOLog("%s: Failed to registerPowerDriver.\n", getName());
	}

	// Join the Power Management tree from IOService.h
	provider->joinPMtree( this);

    // Install power change handler (for restart notification)
    DLOG("IOHWMonitor::start register insterest in power changes\n");
    registerPrioritySleepWakeInterest(&sysPowerDownHandler, this, 0);
    
	DLOG("IOHWMonitor::start(%s) - done\n", fDebugID);
    return true;
}

IOReturn IOHWMonitor::setPowerState(unsigned long whatState, IOService *policyMaker)
{
	powerPolicyMaker = policyMaker;			// Remember who called us
	
	sleeping = (whatState == kIOHWMonitorOffState);
	
	if ( sleeping )
		// If we're busy, promise to ack within kHWSensorPowerAckLimit microseconds, otherwise we're cool.
		return busy ? kHWMonitorPowerAckLimit : IOPMAckImplied;
	else
		// If we're waking we don't care, so just ack.
		return IOPMAckImplied;
}

IOReturn IOHWMonitor::sysPowerDownHandler(void *target, void *refCon, UInt32 messageType, IOService *service, void *messageArgument, vm_size_t argSize )
{
    IOReturn ret;

    switch (messageType)
    {
#if 0
        case kIOMessageSystemWillSleep:
            break;
#endif            
        case kIOMessageSystemWillPowerOff:
        case kIOMessageSystemWillRestart:
            systemIsRestarting = TRUE; // set flag to reflect shutting down state.
            ret = kIOReturnSuccess;
            break;

        default:
            ret = kIOReturnUnsupported;
            break;
    }
    
    return ret;
}

IOReturn IOHWMonitor::updateValue(const OSSymbol *func, const OSSymbol *key)
{
    SInt32 val;
    IOReturn ret;

	/*
	 * If the system is going to sleep or waking from sleep, do nothing - leave the value unchanged
	 *
	 * This prevents untimely I/O during sleep/wake
	 */
         
	if (sleeping || systemIsRestarting)
        return kIOReturnOffline;
            
	busy = true;
    ret = callPlatformFunction(func, FALSE, (void *)fChannel, &val, NULL, NULL);
	busy = false;
	
	/*
	 * If PM called us while we were reading the sensor, then sleeping may now be true
	 * in which case we have to ack the PM so it knows we can sleep.
	 */
	if (sleeping && powerPolicyMaker)
		powerPolicyMaker->acknowledgePowerChange (this);
		
    if(ret != kIOReturnSuccess)
        return ret;
	
	setNumber( key, val );
    
    return ret;
}

void IOHWMonitor::setNumber( const OSSymbol * key, UInt32 val )
{
	OSObject * obj;
	OSNumber * num;

	obj = getProperty( key );
	num = OSDynamicCast(OSNumber, obj);

	if (num)
	{
		num->setValue( val );
	}
	else if (!obj)
	{
		num = OSNumber::withNumber( val, 32 );
		setProperty( key, num );
		num->release();
	}
}