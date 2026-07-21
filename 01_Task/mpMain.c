//mpMain.c
#include "motoPlus.h"

int  SetApplicationInfo();
void mpTask1();
void mpTask2();
void DelayMS(int nMsec);

//GLOBAL DATA DEFINITIONS
int nTaskID1;
int nTaskID2;

void mpUsrRoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
	int rc;

	//TODO: Add additional intialization routines.

	//Creates and starts a new task in a seperate thread of execution.
	//All arguments will be passed to the new task if the function
	//prototype will accept them.
	nTaskID1 = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)mpTask1,
						arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
	nTaskID2 = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)mpTask2,
						arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);

	//Set application information.
	rc = SetApplicationInfo();

	//Ends the initialization task.
	mpExitUsrRoot;
}

//Set application information.
int SetApplicationInfo(void)
{
	MP_APPINFO_SEND_DATA    sData;
	MP_STD_RSP_DATA         rData;
	int                     rc;

	memset(&sData, 0x00, sizeof(sData));
	memset(&rData, 0x00, sizeof(rData));

	strncpy(sData.AppName,  "Default Application",  MP_MAX_APP_NAME);
	strncpy(sData.Version,  "0.00",                 MP_MAX_APP_VERSION);
	strncpy(sData.Comment,  "MotoPlus Application", MP_MAX_APP_COMMENT);

	rc = mpApplicationInfoNotify(&sData, &rData);
	return rc;
}

void mpTask1(void)
{
	//TODO: Add the code for this task
	while(true)
	{
		DelayMS(100);		
	}
}

void mpTask2(int arg1, int arg2)
{
	//TODO: Add the code for this task
	while(true)
	{
		DelayMS(1000);		
	}
}

/*---------------------------------------------------------------------------
    DelayMS()

    MotoPlus delay function.

    mpTaskDelay() uses Tick instead of milliseconds.

    mpGetRtc() returns the controller Tick period.
---------------------------------------------------------------------------*/
void DelayMS(int nMsec)
{
    mpTaskDelay(nMsec / mpGetRtc());
}
