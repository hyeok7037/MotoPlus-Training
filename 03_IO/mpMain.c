// mpMain.c
#include "motoPlus.h"

#define MAX_READ_IO_COUNT 253
#define MAX_WRITE_IO_COUNT 126
#define SAMPLE_IO_COUNT 3

int SetApplicationInfo(void);
void mpTask1(void);

BOOL ReadIO(ULONG ioAddr, USHORT *value);
BOOL ReadIOs(const ULONG *ioAddr, int count, USHORT *value);
BOOL WriteIO(ULONG ioAddr, ULONG value);
BOOL WriteIOs(const ULONG *ioAddr, const ULONG *value, int count);

int g_taskId1;

/******************************************************************************
 * Function : mpUsrRoot
 * Purpose  : Initializes the MotoPlus application and creates the user task.
 ******************************************************************************/
void mpUsrRoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
    g_taskId1 = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)mpTask1, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    SetApplicationInfo();
    mpExitUsrRoot;
}

/******************************************************************************
 * Function : SetApplicationInfo
 * Purpose  : Sets the application name, version, and comment.
 ******************************************************************************/
int SetApplicationInfo(void)
{
    MP_APPINFO_SEND_DATA sendData;
    MP_STD_RSP_DATA responseData;
    int rc;

    memset(&sendData, 0, sizeof(sendData));
    memset(&responseData, 0, sizeof(responseData));

    strncpy(sendData.AppName, "IO Sample", MP_MAX_APP_NAME);
    strncpy(sendData.Version, "1.00", MP_MAX_APP_VERSION);
    strncpy(sendData.Comment, "mpReadIO and mpWriteIO example", MP_MAX_APP_COMMENT);

    rc = mpApplicationInfoNotify(&sendData, &responseData);

    return rc;
}

/******************************************************************************
 * Function : mpTask1
 * Purpose  : Demonstrates single and multiple CIO read/write operations.
 *
 * Important:
 *   Review and change all CIO addresses before running this sample.
 *   The addresses below are examples only.
 ******************************************************************************/
void mpTask1(void)
{
    ULONG readAddr[SAMPLE_IO_COUNT] = {37010, 37011, 37012};
    ULONG writeAddr[SAMPLE_IO_COUNT] = {27010, 27011, 27012};
    USHORT readValue[SAMPLE_IO_COUNT];
    ULONG writeValue[SAMPLE_IO_COUNT];
    USHORT singleReadValue;
    int i;

    memset(readValue, 0, sizeof(readValue));
    memset(writeValue, 0, sizeof(writeValue));
    singleReadValue = 0;

    /**************************************************************************
     * Example 1
     * Read one CIO address.
     **************************************************************************/
    if(ReadIO(37010, &singleReadValue) == FALSE)
    {
        printf("ReadIO failed.\r\n");
        return;
    }

    printf("\r\n[Single IO Read]\r\n");
    printf("CIO 37010 = %u\r\n", singleReadValue);

    /**************************************************************************
     * Example 2
     * Read multiple CIO addresses in one API call.
     **************************************************************************/
    if(ReadIOs(readAddr, SAMPLE_IO_COUNT, readValue) == FALSE)
    {
        printf("ReadIOs failed.\r\n");
        return;
    }

    printf("\r\n[Multiple IO Read]\r\n");

    for(i=0; i<SAMPLE_IO_COUNT; i++)
    {
        printf("CIO %lu = %u\r\n", readAddr[i], readValue[i]);
    }

    /**************************************************************************
     * Example 3
     * Write one CIO address.
     *
     * mpWriteIO() permits only the writable CIO areas listed in the manual.
     * Network Input 27010 is used here as a sample writable address.
     **************************************************************************/
    if(WriteIO(27010, 1) == FALSE)
    {
        printf("WriteIO failed.\r\n");
        return;
    }

    printf("\r\n[Single IO Write]\r\n");
    printf("CIO 27010 <- 1\r\n");

    /**************************************************************************
     * Example 4
     * Write multiple CIO addresses in one API call.
     **************************************************************************/
    writeValue[0] = 1;
    writeValue[1] = 0;
    writeValue[2] = 1;

    if(WriteIOs(writeAddr, writeValue, SAMPLE_IO_COUNT) == FALSE)
    {
        printf("WriteIOs failed.\r\n");
        return;
    }

    printf("\r\n[Multiple IO Write]\r\n");

    for(i=0; i<SAMPLE_IO_COUNT; i++)
    {
        printf("CIO %lu <- %lu\r\n", writeAddr[i], writeValue[i]);
    }

    printf("\r\nIO sample completed.\r\n");

    while(1)
    {
        mpTaskDelay(1000);
    }
}

/******************************************************************************
 * Function : ReadIO
 * Purpose  : Reads one CIO address.
 ******************************************************************************/
BOOL ReadIO(ULONG ioAddr, USHORT *value)
{
    return ReadIOs(&ioAddr, 1, value);
}

/******************************************************************************
 * Function : ReadIOs
 * Purpose  : Reads multiple CIO addresses.
 *
 * Parameters:
 *   ioAddr : Array of CIO addresses.
 *   count  : Number of CIO addresses. Maximum 253.
 *   value  : Buffer receiving the CIO values.
 ******************************************************************************/
BOOL ReadIOs(const ULONG *ioAddr, int count, USHORT *value)
{
    MP_IO_INFO ioInfo[MAX_READ_IO_COUNT];
    int i;
    LONG rc;

    if(ioAddr == NULL || value == NULL)
        return FALSE;

    if(count <= 0 || count > MAX_READ_IO_COUNT)
        return FALSE;

    memset(ioInfo, 0, sizeof(ioInfo));

    for(i=0; i<count; i++)
    {
        ioInfo[i].ulAddr = ioAddr[i];
    }

    rc = mpReadIO(ioInfo, value, count);

    return (rc == OK) ? TRUE : FALSE;
}

/******************************************************************************
 * Function : WriteIO
 * Purpose  : Writes one CIO address.
 ******************************************************************************/
BOOL WriteIO(ULONG ioAddr, ULONG value)
{
    return WriteIOs(&ioAddr, &value, 1);
}

/******************************************************************************
 * Function : WriteIOs
 * Purpose  : Writes multiple CIO addresses.
 *
 * Parameters:
 *   ioAddr : Array of writable CIO addresses.
 *   value  : Array of values to write.
 *   count  : Number of CIO data items. Maximum 126.
 ******************************************************************************/
BOOL WriteIOs(const ULONG *ioAddr, const ULONG *value, int count)
{
    MP_IO_DATA ioData[MAX_WRITE_IO_COUNT];
    int i;
    LONG rc;

    if(ioAddr == NULL || value == NULL)
        return FALSE;

    if(count <= 0 || count > MAX_WRITE_IO_COUNT)
        return FALSE;

    memset(ioData, 0, sizeof(ioData));

    for(i=0; i<count; i++)
    {
        ioData[i].ulAddr = ioAddr[i];
        ioData[i].ulValue = value[i];
    }

    rc = mpWriteIO(ioData, count);

    return (rc == OK) ? TRUE : FALSE;
}

