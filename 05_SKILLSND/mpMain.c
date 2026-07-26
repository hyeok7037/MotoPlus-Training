//mpMain.c
#include "motoPlus.h"

int SetApplicationInfo();
void SkillCommandTask();

BOOL PutUserDVar(int nAddr, MP_D_VAR_BUFF nData);
BOOL GetUserDVar(int nAddr, MP_D_VAR_BUFF* pData);
BOOL IncrementDVar(int nAddr);

//GLOBAL DATA DEFINITIONS
int nTaskID1;

void mpUsrRoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
	nTaskID1 = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)SkillCommandTask,
		arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);

	SetApplicationInfo();
	mpExitUsrRoot;
}

int SetApplicationInfo(void)
{
	MP_APPINFO_SEND_DATA sData;
	MP_STD_RSP_DATA rData;

	memset(&sData, 0, sizeof(sData));
	memset(&rData, 0, sizeof(rData));

	strncpy(sData.AppName, "05_SKILLSND", MP_MAX_APP_NAME);
	strncpy(sData.Version, "1.00", MP_MAX_APP_VERSION);
	strncpy(sData.Comment, "SKILLSND Sample", MP_MAX_APP_COMMENT);

	return mpApplicationInfoNotify(&sData, &rData);
}

BOOL PutUserDVar(int nAddr, MP_D_VAR_BUFF nData)
{
	MP_USR_VAR_INFO info;

	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_D;
	info.var_no = nAddr;
	info.val.d = nData;

	return (mpPutUserVars(&info) == 0) ? TRUE : FALSE;
}

BOOL GetUserDVar(int nAddr, MP_D_VAR_BUFF* pData)
{
	MP_USR_VAR_INFO info;

	if (pData == NULL)
		return FALSE;

	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_D;
	info.var_no = nAddr;

	if (mpGetUserVars(&info) != 0)
		return FALSE;

	*pData = info.val.d;
	return TRUE;
}

BOOL IncrementDVar(int nAddr)
{
	MP_D_VAR_BUFF data;

	if (GetUserDVar(nAddr, &data) == FALSE)
		return FALSE;

	data++;
	return PutUserDVar(nAddr, data);
}

void SkillCommandTask(void)
{
	SYS2MP_SENS_MSG msg;
	int status;

	memset(&msg, CLEAR, sizeof(msg));

	while (1)
	{
		status = mpReceiveSkillCommand(MP_SL_ID1, &msg);

		if (status != OK)
		{
			mpTaskDelay(100);
			continue;
		}

		if (msg.main_comm == MP_SKILL_COMM)
		{
			switch (msg.sub_comm)
			{
			case MP_SKILL_SEND:

				if (strcmp(msg.cmd, "D01") == 0)
					IncrementDVar(1);
				else if (strcmp(msg.cmd, "D02") == 0)
					IncrementDVar(2);
				else
					IncrementDVar(3);

				break;

			case MP_SKILL_END:
				break;
			}
		}

		mpEndSkillCommandProcess(MP_SL_ID1, &msg);
	}
}

