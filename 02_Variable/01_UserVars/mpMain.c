//mpMain.c
#include "motoPlus.h"

int  SetApplicationInfo();
void mpTask1();
void mpTask2();

BOOL PutUserBVar(int nAddr, MP_B_VAR_BUFF nData);
BOOL PutUserIVar(int nAddr, MP_I_VAR_BUFF nData);
BOOL PutUserDVar(int nAddr, MP_D_VAR_BUFF nData);
BOOL PutUserRVar(int nAddr, MP_R_VAR_BUFF nData);
BOOL PutUserPVar(int nAddr, MP_P_VAR_BUFF nData);
BOOL PutUserSVar(int nAddr, const char *str);
BITSTRING MakeFigureCtrl(
    BOOL back,
    BOOL lower,
    BOOL noFlip,
    BOOL r180,
    BOOL t180,
    BOOL s180);

BOOL GetUserBVar(int nAddr, MP_B_VAR_BUFF *pData);
BOOL GetUserIVar(int nAddr, MP_I_VAR_BUFF *pData);
BOOL GetUserDVar(int nAddr, MP_D_VAR_BUFF *pData);
BOOL GetUserRVar(int nAddr, MP_R_VAR_BUFF *pData);
BOOL GetUserPVar(int nAddr, MP_P_VAR_BUFF *pData);
BOOL GetUserSVar(int nAddr, char *str);

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
}

void mpTask2(int arg1, int arg2)
{
	//TODO: Add the code for this task
}

BOOL PutUserBVar(int nAddr, MP_B_VAR_BUFF nData)
{
	MP_USR_VAR_INFO info;
	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_B;
	info.var_no = nAddr;
	info.val.b = nData;

	return mpPutUserVars(&info) == 0 ? TRUE : FALSE;
}

BOOL PutUserIVar(int nAddr, MP_I_VAR_BUFF nData)
{
	MP_USR_VAR_INFO info;
	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_I;
	info.var_no = nAddr;
	info.val.i = nData;

	return mpPutUserVars(&info) == 0 ? TRUE : FALSE;
}
BOOL PutUserDVar(int nAddr, MP_D_VAR_BUFF nData)
{
	MP_USR_VAR_INFO info;
	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_D;
	info.var_no = nAddr;
	info.val.d = nData;

	return mpPutUserVars(&info) == 0 ? TRUE : FALSE;
}
BOOL PutUserRVar(int nAddr, MP_R_VAR_BUFF nData)
{
	MP_USR_VAR_INFO info;
	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_R;
	info.var_no = nAddr;
	info.val.r = nData;

	return mpPutUserVars(&info) == 0 ? TRUE : FALSE;
}

BOOL PutUserPVar(int nAddr, MP_P_VAR_BUFF nData)
{
	MP_USR_VAR_INFO info;
	memset(&info, 0, sizeof(info));
	info.var_type = MP_VAR_P;
	info.var_no = nAddr;
	info.val.p = nData;

	return mpPutUserVars(&info) == 0 ? TRUE : FALSE;
}


BOOL PutUserSVar(int nAddr, const char *str)
{
    MP_USR_VAR_INFO info;
    int len;

    if (str == NULL)
        return FALSE;

    memset(&info, 0, sizeof(info));

    info.var_type = MP_VAR_S;
    info.var_no   = nAddr;

    len = strlen(str);

    if (len >= STR_VAR_SIZE)
        len = STR_VAR_SIZE - 1;

    memcpy(info.val.s, str, len);
    info.val.s[len] = '\0';

    return (mpPutUserVars(&info) == 0) ? TRUE : FALSE;
}

/******************************************************************************
 * Function : MakeFigureCtrl
 * Purpose  : Creates the figure (configuration) information used by P variables.
 *
 * Each parameter corresponds to one bit of the fig_ctrl field.
 *
 * Parameter
 *   back    : FALSE = Front,      TRUE = Back
 *   lower   : FALSE = Upper Arm,  TRUE = Lower Arm
 *   noFlip  : FALSE = Flip,       TRUE = No Flip
 *   r180    : FALSE = R < 180°,   TRUE = R >= 180°
 *   t180    : FALSE = T < 180°,   TRUE = T >= 180°
 *   s180    : FALSE = S < 180°,   TRUE = S >= 180°
 *
 * Return
 *   BITSTRING : Figure information for MP_P_VAR_BUFF.fig_ctrl
 ******************************************************************************/
BITSTRING MakeFigureCtrl(
    BOOL back,
    BOOL lower,
    BOOL noFlip,
    BOOL r180,
    BOOL t180,
    BOOL s180)
{
    BITSTRING fig = 0;

    if (back)   fig |= MP_FIG_SIDE;
    if (lower)  fig |= MP_FIG_ELBOW;
    if (noFlip) fig |= MP_FIG_FLIP;
    if (r180)   fig |= MP_FIG_R180;
    if (t180)   fig |= MP_FIG_T180;
    if (s180)   fig |= MP_FIG_S180;

    return fig;
}



BOOL GetUserBVar(int nAddr, MP_B_VAR_BUFF *pData)
{
    /*
     * TODO
     * 1. Validate input parameter.
     * 2. Initialize MP_USR_VAR_INFO.
     * 3. Set MP_VAR_B and variable number.
     * 4. Call mpGetUserVars().
     * 5. Copy the returned B variable to pData.
     * 6. Return TRUE on success, FALSE on failure.
     */
}

BOOL GetUserIVar(int nAddr, MP_I_VAR_BUFF *pData)
{
    /*
     * TODO
     * Implement the same logic as GetUserBVar() for I variables.
     */
}

BOOL GetUserDVar(int nAddr, MP_D_VAR_BUFF *pData)
{
    /*
     * TODO
     * Implement the same logic as GetUserBVar() for D variables.
     */
}

BOOL GetUserRVar(int nAddr, MP_R_VAR_BUFF *pData)
{
    /*
     * TODO
     * Implement the same logic as GetUserBVar() for R variables.
     */
}

BOOL GetUserPVar(int nAddr, MP_P_VAR_BUFF *pData)
{
    /*
     * TODO
     * Implement the same logic as GetUserBVar() for P variables.
     * (Copy the entire structure.)
     */
}

BOOL GetUserSVar(int nAddr, char *str)
{
    /*
     * TODO
     * Implement the same logic as GetUserBVar() for S variables.
     * (Copy the string using memcpy().)
     */
}