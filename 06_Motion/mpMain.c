//mpMain.c
#include "motoPlus.h"

#define P_VAR_START	10
#define P_VAR_END	15
#define MOVL_SPEED	100		//0.1mm/s unit = 10.0mm/s

int SetApplicationInfo(void);
void mpTask1(void);

BOOL IsRemoteMode(void);
BOOL IsPlayMode(void);
BOOL IsServoOn(void);
BOOL ServoOn(void);
BOOL GetUserPVar(int nAddr, MP_P_VAR_BUFF* pData);
int SetCoordFromPVar(int grpNo, MP_P_VAR_BUFF* pData);
BOOL SendPVarTarget(int grpNo, CTRLG_T grpMask, int pNo);

int nTaskID1;

void mpUsrRoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
	nTaskID1 = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)mpTask1,
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

	strncpy(sData.AppName, "06_MotionAPI", MP_MAX_APP_NAME);
	strncpy(sData.Version, "1.00", MP_MAX_APP_VERSION);
	strncpy(sData.Comment, "P010-P015 MOVL Sample", MP_MAX_APP_COMMENT);

	return mpApplicationInfoNotify(&sData, &rData);
}

BOOL IsRemoteMode(void)
{
	MP_MODE_RSP_DATA data;

	memset(&data, 0, sizeof(data));

	if (mpGetMode(&data) != OK)
		return FALSE;

	return (data.sRemote == 1) ? TRUE : FALSE;
}

BOOL IsPlayMode(void)
{
	MP_MODE_RSP_DATA data;

	memset(&data, 0, sizeof(data));

	if (mpGetMode(&data) != OK)
		return FALSE;

	return (data.sMode == 2) ? TRUE : FALSE;
}

BOOL IsServoOn(void)
{
	MP_SERVO_POWER_RSP_DATA data;

	memset(&data, 0, sizeof(data));

	if (mpGetServoPower(&data) != OK)
		return FALSE;

	return data.sServoPower ? TRUE : FALSE;
}

BOOL ServoOn(void)
{
	MP_SERVO_POWER_SEND_DATA sData;
	MP_STD_RSP_DATA rData;

	memset(&sData, 0, sizeof(sData));
	memset(&rData, 0, sizeof(rData));

	sData.sServoPower = 1;

	if (mpSetServoPower(&sData, &rData) != OK)
		return FALSE;

	return (rData.err_no == 0) ? TRUE : FALSE;
}

BOOL GetUserPVar(int nAddr, MP_P_VAR_BUFF* pData)
{
	MP_USR_VAR_INFO info;

	if (pData == NULL)
		return FALSE;

	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_P;
	info.var_no = nAddr;

	if (mpGetUserVars(&info) != OK)
		return FALSE;

	*pData = info.val.p;
	return TRUE;
}

int SetCoordFromPVar(int grpNo, MP_P_VAR_BUFF* pData)
{
	switch (pData->dtype)
	{
	case MP_BASE_DTYPE:
		return mpMotSetCoord(grpNo, MP_BASE_TYPE, 0);

	case MP_ROBO_DTYPE:
		return mpMotSetCoord(grpNo, MP_ROBOT_TYPE, 0);

	case MP_USER_DTYPE:
		return mpMotSetCoord(grpNo, MP_USER_TYPE, pData->uf_no);

	case MP_PULSE_DTYPE:
		return mpMotSetCoord(grpNo, MP_PULSE_TYPE, 0);

	default:
		return -1;
	}
}

BOOL SendPVarTarget(int grpNo, CTRLG_T grpMask, int pNo)
{
	MP_P_VAR_BUFF pData;
	MP_SPEED speed;
	MP_TARGET target;

	if (GetUserPVar(pNo, &pData) == FALSE)
		return FALSE;

	if (SetCoordFromPVar(grpNo, &pData) < 0)
		return FALSE;

	if (mpMotSetTool(grpNo, pData.tool_no) < 0)
		return FALSE;

	memset(&speed, 0, sizeof(speed));
	speed.v = MOVL_SPEED;

	if (mpMotSetSpeed(grpNo, &speed) < 0)
		return FALSE;

	memset(&target, 0, sizeof(target));
	target.id = pNo;
	target.intp = MP_MOVL_TYPE;
	target.dst.coord.x = pData.data[0];
	target.dst.coord.y = pData.data[1];
	target.dst.coord.z = pData.data[2];
	target.dst.coord.rx = pData.data[3];
	target.dst.coord.ry = pData.data[4];
	target.dst.coord.rz = pData.data[5];
	target.dst.coord.ex1 = pData.data[6];
	target.dst.coord.ex2 = pData.data[7];

	return (mpMotTargetSend(grpMask, &target, WAIT_FOREVER) >= 0) ? TRUE : FALSE;
}

void mpTask1(void)
{
	int grpNo;
	CTRLG_T grpMask;
	int pNo;
	int vacant;
	int recvId;
	int executed;

	grpNo = mpCtrlGrpId2GrpNo(MP_R1_GID);
	grpMask = (1 << grpNo);
	executed = FALSE;

	while (1)
	{
		if (IsRemoteMode() == FALSE)
		{
			executed = FALSE;
			mpTaskDelay(100);
			continue;
		}

		if (executed == TRUE)
		{
			mpTaskDelay(100);
			continue;
		}

		if (IsServoOn() == FALSE)
		{
			if (ServoOn() == FALSE)
			{
				mpTaskDelay(100);
				continue;
			}

			while (IsRemoteMode() == TRUE && IsServoOn() == FALSE)
				mpTaskDelay(50);
		}

		if (IsRemoteMode() == FALSE || IsPlayMode() == FALSE || IsServoOn() == FALSE)
		{
			mpTaskDelay(100);
			continue;
		}

		mpMotStop(0);
		mpMotTargetClear(grpMask, 0);
		mpTaskDelay(10);

		if (mpMotTargetGetVacantBufNum(grpNo, &vacant) < 0 || vacant < (P_VAR_END - P_VAR_START + 1))
		{
			mpTaskDelay(100);
			continue;
		}

		for (pNo = P_VAR_START; pNo <= P_VAR_END; pNo++)
		{
			if (SendPVarTarget(grpNo, grpMask, pNo) == FALSE)
				break;
		}

		if (pNo <= P_VAR_END)
		{
			mpMotTargetClear(grpMask, 0);
			mpTaskDelay(100);
			continue;
		}

		if (mpMotStart(0) < 0)
		{
			mpMotTargetClear(grpMask, 0);
			mpTaskDelay(100);
			continue;
		}

		recvId = 0;

		if (mpMotTargetReceive(grpNo, P_VAR_END, &recvId, WAIT_FOREVER, 0) >= 0)
			executed = TRUE;

		mpTaskDelay(100);
	}
}

