/*
 * 03_SVar_TCP
 *
 * MotoPlus TCP Server Example
 * PC Client -> TCP -> MotoPlus -> S010
 *
 * Message format:
 *   Printable ASCII text + LF('\n')
 *
 * Example:
 *   HELLO ROBOT\n
 *
 * If STR_VAR_SIZE == 33:
 *   text area : index 0 ~ 31
 *   NULL      : index 32
 *   max text  : 32 bytes
 */

#include "motoPlus.h"

#define SERVER_PORT         11000
#define TARGET_S_VAR_NO     10
#define RECV_BUFFER_SIZE    128
#define ASCII_MIN           0x20
#define ASCII_MAX           0x7E

static int gTaskId;

static void TcpServerTask(void);
static void RunTcpServer(void);
static int CreateListenSocket(void);
static int WaitForClient(int listenFd);
static int ReceiveLine(int clientFd, char *lineBuffer);
static int AppendReceivedData(const char *recvBuffer, int bytesRecv, char *lineBuffer, int *lineLength, int *lineComplete);
static void ProcessLine(int clientFd, const char *lineBuffer);
static STATUS PutSVar(UINT16 varNo, const char *text);
static void SendText(int clientFd, const char *text);

/**************************************************************************
 * MotoPlus application entry point
 **************************************************************************/
void mpUsrRoot(
    int arg1, int arg2, int arg3, int arg4, int arg5,
    int arg6, int arg7, int arg8, int arg9, int arg10)
{
    /* Create the TCP server task. */
    gTaskId = mpCreateTask(
        MP_PRI_TIME_NORMAL,
        MP_STACK_SIZE,
        (FUNCPTR)TcpServerTask,
        arg1, arg2, arg3, arg4, arg5,
        arg6, arg7, arg8, arg9, arg10);

    /* End only the root task. */
    mpExitUsrRoot;
}

/**************************************************************************
 * TCP server task
 **************************************************************************/
static void TcpServerTask(void)
{
    RunTcpServer();
    mpSuspendSelf;
}

/**************************************************************************
 * Run the TCP server.
 *
 * Flow:
 *   1. Create listen socket
 *   2. Wait for PC connection
 *   3. Receive one line
 *   4. Write the line to S010
 *   5. Send result to PC
 **************************************************************************/
static void RunTcpServer(void)
{
    int listenFd;

    listenFd = CreateListenSocket();
    if(listenFd < 0)
        return;

    while(1)
    {
        int clientFd;
        char lineBuffer[STR_VAR_SIZE];

        /* Wait until the PC client connects. */
        clientFd = WaitForClient(listenFd);
        if(clientFd < 0)
            continue;

        /* One connection may send multiple lines. */
        while(ReceiveLine(clientFd, lineBuffer) == TRUE)
        {
            ProcessLine(clientFd, lineBuffer);
        }

        mpClose(clientFd);
    }
}

/**************************************************************************
 * Create, bind and listen on the TCP server socket.
 *
 * Return:
 *   socket descriptor : success
 *   -1                : failure
 **************************************************************************/
static int CreateListenSocket(void)
{
    int listenFd;
    int result;
    struct sockaddr_in serverAddr;

    /* Create an IPv4 TCP socket. */
    listenFd = mpSocket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0)
        return -1;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = mpHtons(SERVER_PORT);

    /* Bind the socket to port 11000. */
    result = mpBind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(result < 0)
    {
        mpClose(listenFd);
        return -1;
    }

    /* Change the socket to listen state. */
    result = mpListen(listenFd, SOMAXCONN);
    if(result < 0)
    {
        mpClose(listenFd);
        return -1;
    }

    return listenFd;
}

/**************************************************************************
 * Wait for one PC client.
 * mpAccept() blocks until a client connects.
 **************************************************************************/
static int WaitForClient(int listenFd)
{
    int clientFd;
    int clientAddrSize;
    struct sockaddr_in clientAddr;

    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddrSize = sizeof(clientAddr);

    clientFd = mpAccept(listenFd, (struct sockaddr *)&clientAddr, &clientAddrSize);
    return clientFd;
}

/**************************************************************************
 * Receive one complete line from the TCP stream.
 *
 * Delimiter:
 *   CR('\r') : ignored
 *   LF('\n') : end of one message
 *
 * Character rule:
 *   Printable ASCII only: 0x20 ~ 0x7E
 *
 * Length rule:
 *   Maximum text length is STR_VAR_SIZE - 1.
 *   Excess characters are discarded until LF is received.
 **************************************************************************/
static int ReceiveLine(int clientFd, char *lineBuffer)
{
    char recvBuffer[RECV_BUFFER_SIZE];
    int lineLength;
    int lineComplete;

    memset(lineBuffer, 0, STR_VAR_SIZE);
    lineLength = 0;
    lineComplete = FALSE;

    while(lineComplete == FALSE)
    {
        int bytesRecv;

        /*
         * TCP is a byte stream. One mpRecv() call may receive
         * part of one line, one full line, or multiple lines.
         */
        bytesRecv = mpRecv(clientFd, recvBuffer, sizeof(recvBuffer), 0);
        if(bytesRecv <= 0)
            return FALSE;

        AppendReceivedData(
            recvBuffer,
            bytesRecv,
            lineBuffer,
            &lineLength,
            &lineComplete);
    }

    /* Guarantee NULL termination. */
    lineBuffer[lineLength] = '\0';
    lineBuffer[STR_VAR_SIZE - 1] = '\0';

    return TRUE;
}

/**************************************************************************
 * Add received TCP bytes to the line buffer.
 *
 * Processing order:
 *   1. Ignore CR
 *   2. Stop at LF
 *   3. Reject non-printable ASCII
 *   4. Append valid character
 *   5. Discard characters after maximum length
 **************************************************************************/
static int AppendReceivedData(
    const char *recvBuffer,
    int bytesRecv,
    char *lineBuffer,
    int *lineLength,
    int *lineComplete)
{
    int i;

    for(i = 0; i < bytesRecv; i++)
    {
        unsigned char ch;

        ch = (unsigned char)recvBuffer[i];

        /* Windows commonly sends CR + LF. Ignore CR. */
        if(ch == '\r')
            continue;

        /* LF means one message is complete. */
        if(ch == '\n')
        {
            *lineComplete = TRUE;
            break;
        }

        /* Accept only printable 7-bit ASCII. */
        if((ch < ASCII_MIN) || (ch > ASCII_MAX))
            continue;

        /* Keep one byte for the final NULL character. */
        if(*lineLength < (STR_VAR_SIZE - 1))
        {
            lineBuffer[*lineLength] = (char)ch;
            (*lineLength)++;

            /* Keep the buffer as a valid C string. */
            lineBuffer[*lineLength] = '\0';
        }
        else
        {
            /* Buffer full: discard characters until LF. */
            lineBuffer[STR_VAR_SIZE - 1] = '\0';
        }
    }

    return TRUE;
}

/**************************************************************************
 * Process one completed line.
 *
 * Result:
 *   Empty string       -> ERR:EMPTY
 *   S010 write success -> OK:<text>
 *   S010 write failure -> ERR:S010_WRITE
 **************************************************************************/
static void ProcessLine(int clientFd, const char *lineBuffer)
{
    STATUS status;
    char response[STR_VAR_SIZE + 5];

    if(lineBuffer[0] == '\0')
    {
        SendText(clientFd, "ERR:EMPTY\n");
        return;
    }

    status = PutSVar(TARGET_S_VAR_NO, lineBuffer);
    if(status != OK)
    {
        SendText(clientFd, "ERR:S010_WRITE\n");
        return;
    }

    /* Echo the stored string to the PC. */
    memset(response, 0, sizeof(response));
    strncpy(response, "OK:", sizeof(response) - 1);
    strncat(response, lineBuffer, sizeof(response) - strlen(response) - 2);
    strcat(response, "\n");

    SendText(clientFd, response);
}

/**************************************************************************
 * Write a C string to a robot S variable.
 * varNo 10 means S010.
 **************************************************************************/
static STATUS PutSVar(UINT16 varNo, const char *text)
{
    MP_USR_VAR_INFO info;
    int copyLength;

    if(text == NULL)
        return ERROR;

    memset(&info, 0, sizeof(info));

    info.var_type = MP_VAR_S;
    info.var_no = varNo;

    copyLength = strlen(text);
    if(copyLength > (STR_VAR_SIZE - 1))
        copyLength = STR_VAR_SIZE - 1;

    memcpy(info.val.s, text, copyLength);

    /* NULL immediately after copied text. */
    info.val.s[copyLength] = '\0';

    /* Also guarantee final buffer position is NULL. */
    info.val.s[STR_VAR_SIZE - 1] = '\0';

    return mpPutUserVars(&info);
}

/**************************************************************************
 * Send a NULL-terminated ASCII string to the PC client.
 **************************************************************************/
static void SendText(int clientFd, const char *text)
{
    int textLength;

    if(text == NULL)
        return;

    textLength = strlen(text);
    if(textLength <= 0)
        return;

    mpSend(clientFd, (char *)text, textLength, 0);
}

