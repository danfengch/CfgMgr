/**
  ******************************************************************************
  * @par Copyright (c): ChengDu BinHong Science & Technology Co.,ltd
  * @file     cfgMgr.c
  * @author   Chenxu
  * @version  V1.0.0
  * @date     2019/09/25
  * @defgroup cgi
  * @ingroup  cgi  
  * @brief    Configure management unit function cgi implement
  * @par History
  * Date          Owner         BugID/CRID        Contents
  * 2019/09/25    Chenxu        None              File Create
  ****************************************************************************** 
  */

#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <CgicDebug.h>
#include <parameters.h>
#include <string.h>
#include <assert.h>
#include <message.h>
#include <time.h>
#include <cfgMgrCgi.h>
#include <../message.c>
#include <../share.c>
#include <unistd.h>
#include <netdb.h>
#include <config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>  
#include <linux/ethtool.h> 
#include <linux/sockios.h>
#include <net/route.h>
#include <net/if_arp.h>


#define stringLenZeroChkReturn(str)\
    if (strlen(str) == 0)\
    {\
        if (functionName)\
            CGIDEBUG("In Function %s : ", functionName);\
        CGIDEBUG("cann't find a element at %s line %d !!!\n", __FILE__, __LINE__);\
        CGICASSERT(0);\
        return -1;\
    }
#define fileNameLenChkReturn(fileName)\
    if (strlen(fileName) > FILE_NAME_LEN_MAX)\
    {\
        CGIDEBUG("%s FileName[%s] len > %d !!!\n", __FUNCTION__, fileName, FILE_NAME_LEN_MAX);\
        return -1;\
    }\

typedef struct
{
    char *formName;
    int  (*func)(msg *m);
}formMethod;

static char *functionName = NULL;
static int isFirstDebugInfo = 1;

static void printCgiEnv(void)
{
    CGIDEBUG("cgiServerSoftware : %s\n", cgiServerSoftware);
    CGIDEBUG("cgiServerName : %s\n", cgiServerName);
    CGIDEBUG("cgiGatewayInterface : %s\n", cgiGatewayInterface);
    CGIDEBUG("cgiServerProtocol : %s\n", cgiServerProtocol);
    CGIDEBUG("cgiServerPort : %s\n", cgiServerPort);
    CGIDEBUG("cgiRequestMethod : %s\n", cgiRequestMethod);
    CGIDEBUG("cgiPathInfo : %s\n", cgiPathInfo);
    CGIDEBUG("cgiPathTranslated : %s\n", cgiPathTranslated);
    CGIDEBUG("cgiScriptName : %s\n", cgiScriptName);
    CGIDEBUG("cgiQueryString : %s\n", cgiQueryString);
    CGIDEBUG("cgiRemoteHost : %s\n",cgiRemoteHost );
    CGIDEBUG("cgiRemoteAddr : %s\n", cgiRemoteAddr);
    CGIDEBUG("cgiAuthType : %s\n", cgiAuthType);
    CGIDEBUG("cgiRemoteUser : %s\n", cgiRemoteUser);
    CGIDEBUG("cgiRemoteIdent : %s\n", cgiRemoteIdent);    
}

static int login (msg *m)
{
    char UserName[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Passwd[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    loginRequest *lg = (loginRequest *)m->data;

    m->type = MSGTYPE_LOGIN_REQUEST;

    cgiFormString("UserName", UserName, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("Passwd", Passwd, FORM_ELEMENT_STRING_LEN_MAX);

    stringLenZeroChkReturn(UserName);
    stringLenZeroChkReturn(Passwd);

    if (strlen(UserName) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("login UserName[%s] len > %d !!!\n", UserName, USR_KEY_LNE_MAX);
        return -1;
    }
    strncpy(lg->userName, UserName, sizeof(lg->userName));

    if (strlen(Passwd) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("login Passwd[%s] len > %d !!!\n", Passwd, USR_KEY_LNE_MAX);
        return -1;
    }
    strncpy(lg->passwd, Passwd, sizeof(lg->passwd));    
    
    return 0;
}


static int netParamGet(netParam *net, int netNumber)
{
    char Lan_Auto[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Lan_IP[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Lan_Mask[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Lan_GateWay[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Lan_Mac[FORM_ELEMENT_STRING_LEN_MAX] = {0};

    memset(net, 0, sizeof(netParam));

    if (netNumber == 1)
    {
        cgiFormString("Lan1_Auto", Lan_Auto, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan1_IP", Lan_IP, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan1_Mask", Lan_Mask, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan1_GateWay", Lan_GateWay, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan1_Mac", Lan_Mac, FORM_ELEMENT_STRING_LEN_MAX);

    }
    else
    {
        cgiFormString("Lan2_Auto", Lan_Auto, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan2_IP", Lan_IP, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan2_Mask", Lan_Mask, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan2_GateWay", Lan_GateWay, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan2_Mac", Lan_Mac, FORM_ELEMENT_STRING_LEN_MAX);
    }

    stringLenZeroChkReturn(Lan_Auto);
    stringLenZeroChkReturn(Lan_Mac);

    if (strstr(Lan_Auto, "true"))
        net->isDhcp = TRUE;
    else if (strstr(Lan_Auto, "false"))
        net->isDhcp = FALSE;
    else
    {
        CGIDEBUG ("Lan%d_Auto invalid\n", netNumber);
//        CGICASSERT(0);
        return -1;
    }
    
    if (!net->isDhcp)
    {
        stringLenZeroChkReturn(Lan_IP);
        stringLenZeroChkReturn(Lan_Mask);
        stringLenZeroChkReturn(Lan_GateWay);        
        inet_pton(AF_INET, Lan_IP, (void*)&net->ip);
        inet_pton(AF_INET, Lan_Mask, (void*)&net->mask);
        inet_pton(AF_INET, Lan_GateWay, (void*)&net->gateway);
    }    

    if(0 != macString2Hex(Lan_Mac, net->mac))
    {
        CGIDEBUG ("Lan%d_Mac %s invalid !!!\n", netNumber, Lan_Mac);
        return -1;
    }

    return 0;
}

static int lan1Test (msg *m)
{
    lanTestRequest *req = (lanTestRequest *)m->data;

    m->type = MSGTYPE_LAN1TEST;

//    CGIDEBUG ("cgiRemoteAddr :%s \n", cgiRemoteAddr);

//    inet_pton(AF_INET, attr_value, (void*)&p->lan1.net.ip);

    inet_pton(AF_INET, cgiRemoteAddr, (void*)&req->destIp);

    return netParamGet(&req->net, 1);
}

static int lan2Test (msg *m)
{
    netParam *net = (netParam *)m->data;

    m->type = MSGTYPE_LAN2TEST;

    return netParamGet(net, 2);
}

static int netConfigSave (msg *m)
{
    netParam *net = (netParam *)m->data;

    m->type = MSGTYPE_NETCONFIGSAVE;

    if (0 != netParamGet(net, 1))
    {
        CGIDEBUG("netParamGet 1 error !!!\n");
        return -1;
    }

    net++;
    
    if (0 != netParamGet(net, 2))
    {
        CGIDEBUG("netParamGet 2 error !!!\n");
        return -1;
    }

    return 0;
}

static int captureParamGet(captureParam *capture, int netNumber)
{
    char Lan_CaptureServiceStatus[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Lan_AutoUpLoadEnable[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Lan_AutoUpLoadPath[FORM_ELEMENT_STRING_LEN_MAX] = {0};

    if (netNumber == 1)
    {
        cgiFormString("Lan1_CaptureServiceStatus", Lan_CaptureServiceStatus, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan1_AutoUpLoadEnable", Lan_AutoUpLoadEnable, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan1_AutoUpLoadPath", Lan_AutoUpLoadPath, FORM_ELEMENT_STRING_LEN_MAX);
    }
    else
    {
        cgiFormString("Lan2_CaptureServiceStatus", Lan_CaptureServiceStatus, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan2_AutoUpLoadEnable", Lan_AutoUpLoadEnable, FORM_ELEMENT_STRING_LEN_MAX);
        cgiFormString("Lan2_AutoUpLoadPath", Lan_AutoUpLoadPath, FORM_ELEMENT_STRING_LEN_MAX);
    }

    stringLenZeroChkReturn(Lan_CaptureServiceStatus);
    stringLenZeroChkReturn(Lan_AutoUpLoadEnable);
    stringLenZeroChkReturn(Lan_AutoUpLoadPath);

    if (strstr(Lan_CaptureServiceStatus, "true"))
        capture->isCapture = TRUE;
    else if (strstr(Lan_CaptureServiceStatus, "false"))
        capture->isCapture = FALSE;
    else
    {
        CGIDEBUG ("Lan%d_CaptureServiceStatus invalid\n", netNumber);
        CGICASSERT(0);
        return -1;
    }
    
    if (strstr(Lan_AutoUpLoadEnable, "true"))
        capture->isAutoUpLoad = TRUE;
    else if (strstr(Lan_AutoUpLoadEnable, "false"))
        capture->isAutoUpLoad = FALSE;
    else
    {
        CGIDEBUG ("Lan%d_AutoUpLoadEnable invalid\n", netNumber);
        CGICASSERT(0);
        return -1;
    }

    strncpy(capture->autoUpLoadPath, Lan_AutoUpLoadPath, sizeof(capture->autoUpLoadPath));

    return 0;
}


static int netCapture(msg *m)
{
    captureParam *capture = (captureParam *)m->data;

    if (0 != captureParamGet(capture, 1))
    {
        CGIDEBUG("captureParamGet 1 error !!!\n");
        return -1;
    }

    capture++;

    if (0 != captureParamGet(capture, 2))
    {
        CGIDEBUG("captureParamGet 2 error !!!\n");
        return -1;
    }

    m->type = MSGTYPE_NETCAPTURE;

    return 0;
}

static int filterParamGet(filterParam *filter, int netNumber)
{
    char Lan_NetFilterServiceStatus[FORM_ELEMENT_STRING_LEN_MAX] = {0};

    if (netNumber == 1)
    {
        cgiFormString("Lan1_NetFilterServiceStatus", Lan_NetFilterServiceStatus, FORM_ELEMENT_STRING_LEN_MAX);
    }
    else
    {
        cgiFormString("Lan2_NetFilterServiceStatus", Lan_NetFilterServiceStatus, FORM_ELEMENT_STRING_LEN_MAX);
    }

    stringLenZeroChkReturn(Lan_NetFilterServiceStatus);

    if (strstr(Lan_NetFilterServiceStatus, "true"))
        filter->isFilter = TRUE;
    else if (strstr(Lan_NetFilterServiceStatus, "false"))
        filter->isFilter = FALSE;
    else
    {
        CGIDEBUG ("Lan%d_NetFilterServiceStatus invalid\n", netNumber);
        CGICASSERT(0);
        return -1;
    }

    return 0;
}


static int netFilter(msg *m)
{
    filterParam *filter = (filterParam *)m->data;

    if (0 != filterParamGet(filter, 1))
    {
        CGIDEBUG("filterParamGet 1 error !!!\n");
        return -1;
    }

    filter++;

    if (0 != filterParamGet(filter, 2))
    {
        CGIDEBUG("filtereParamGet 2 error !!!\n");
        return -1;
    }

    m->type = MSGTYPE_NETCAPTURE;

    return 0;
}

static int fileLookUp(msg *m)
{
    fileLookUpRequest *fileLookUpReq = (fileLookUpRequest *)m->data;
    char NetNumber[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char StartTime[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char EndTime[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char start[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char length[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char draw[FORM_ELEMENT_STRING_LEN_MAX] = {0};    

    cgiFormString("NetNumber", NetNumber, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("StartTime", StartTime, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("EndTime", EndTime, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("start", start, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("length", length, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("draw", draw, FORM_ELEMENT_STRING_LEN_MAX);

    stringLenZeroChkReturn(NetNumber);
    stringLenZeroChkReturn(StartTime);
    stringLenZeroChkReturn(EndTime);
    stringLenZeroChkReturn(start);
    stringLenZeroChkReturn(length);
    stringLenZeroChkReturn(draw);    

    fileLookUpReq->netNumber = atoi(NetNumber);
    fileLookUpReq->startTime = format2time(StartTime);
    fileLookUpReq->endTime = format2time(EndTime);
    fileLookUpReq->start = atoi(start);
    fileLookUpReq->length = atoi(length);
    fileLookUpReq->draw = atoi(draw);

    m->type = MSGTYPE_FILELOOKUP_REQUEST;
    
    return 0;
}

static int fileUpLoad(msg *m)
{
    fileUpLoadRequest *req = (fileUpLoadRequest *)m->data;
    char FileName[FORM_ELEMENT_STRING_LEN_MAX] = {0};  

    cgiFormString("FileName", FileName, FORM_ELEMENT_STRING_LEN_MAX);

    stringLenZeroChkReturn(FileName);
    fileNameLenChkReturn(FileName);

    strncpy(req->fileName, FileName, sizeof(req->fileName));

    m->type = MSGTYPE_FILEUPLOAD_REQUEST;
    
    return 0;
}

static int diskInfo(msg *m)
{
    m->type = MSGTYPE_DISKINFO;
    
    return 0;
}

static int normalUserMgr(msg *m)
{
    normalUserMgrRequest *req = (normalUserMgrRequest *)m->data;
    char NewKey[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char PrimaryKey[FORM_ELEMENT_STRING_LEN_MAX] = {0};

    cgiFormString("NewKey", NewKey, FORM_ELEMENT_STRING_LEN_MAX);    
    cgiFormString("PrimaryKey", PrimaryKey, FORM_ELEMENT_STRING_LEN_MAX);    

    stringLenZeroChkReturn(NewKey);
    stringLenZeroChkReturn(PrimaryKey);

    if (strlen(NewKey) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("normalUserMgr NewKey[%s] len > %d !!!\n", NewKey, USR_KEY_LNE_MAX);
        return -1;
    }
    if (strlen(PrimaryKey) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("normalUserMgr PrimaryKey[%s] len > %d !!!\n", PrimaryKey, USR_KEY_LNE_MAX);
        return -1;
    }

    strncpy(req->newKey, NewKey, sizeof(req->newKey));
    strncpy(req->primaryKey, PrimaryKey, sizeof(req->primaryKey));

    m->type = MSGTYPE_NORMALUSERMGR_REQUEST;
    
    return 0;
}

static int superUserMgr(msg *m)
{
    superUserMgrRequest *req = (superUserMgrRequest *)m->data;
    char UserName[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char AdminKey[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char NewKey[FORM_ELEMENT_STRING_LEN_MAX] = {0};

    cgiFormString("UserName", UserName, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("AdminKey", AdminKey, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("NewKey", NewKey, FORM_ELEMENT_STRING_LEN_MAX);

    stringLenZeroChkReturn(UserName);
    stringLenZeroChkReturn(AdminKey);
    stringLenZeroChkReturn(NewKey);    

    if (strlen(UserName) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("superUserMgr UserName[%s] len > %d !!!\n", UserName, USR_KEY_LNE_MAX);
        return -1;
    }
    strncpy(req->userName, UserName, sizeof(req->userName));

    if (strlen(AdminKey) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("superUserMgr AdminKey[%s] len > %d !!!\n", AdminKey, USR_KEY_LNE_MAX);
        return -1;
    }
    strncpy(req->adminKey, AdminKey, sizeof(req->adminKey));
    
    if (strlen(NewKey) > USR_KEY_LNE_MAX)
    {
        CGIDEBUG("superUserMgr NewKey[%s] len > %d !!!\n", NewKey, USR_KEY_LNE_MAX);
        return -1;
    }
    strncpy(req->newKey, NewKey, sizeof(req->newKey));

    m->type = MSGTYPE_SUPERUSERMGR_REQUEST;
    
    return 0;
}


static int systemInfo(msg *m)
{
    m->type = MSGTYPE_SYSTEMINFO;
    
    return 0;
}

static int systemTimeGet(msg *m)
{
    m->type = MSGTYPE_SYSTIMEGET_REQUEST;
    
    return 0;
}

static int systemTimeSet(msg *m)
{
    char CorrectTime[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    sysTimeSetRequest * req = (sysTimeSetRequest *)m->data;

    cgiFormString("CorrectTime", CorrectTime, FORM_ELEMENT_STRING_LEN_MAX);    
    stringLenZeroChkReturn(CorrectTime);

    req->correctTime = format2time(CorrectTime);    

    m->type = MSGTYPE_SYSTIMESET_REQUEST;
    
    return 0;
}

static int getVersion(msg *m)
{
    m->type = MSGTYPE_GETVERSION_REQUEST;
    
    return 0;
}

static int factoryReset(msg *m)
{
    m->type = MSGTYPE_FACTORYRESET_REQUEST;
    
    return 0;
}

static int reboot(msg *m)
{
    m->type = MSGTYPE_REBOOT_REQUEST;
    
    return 0;
}


static int logLookUp(msg *m)
{
    logLookUpRequest *logLookUpCtrl = (logLookUpRequest *)m->data;
    char LogType[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char Significance[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char StartTime[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char EndTime[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char start[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char length[FORM_ELEMENT_STRING_LEN_MAX] = {0};
    char draw[FORM_ELEMENT_STRING_LEN_MAX] = {0}; 

    cgiFormString("LogType", LogType, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("Significance", Significance, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("StartTime", StartTime, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("EndTime", EndTime, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("start", start, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("length", length, FORM_ELEMENT_STRING_LEN_MAX);
    cgiFormString("draw", draw, FORM_ELEMENT_STRING_LEN_MAX);

    stringLenZeroChkReturn(LogType);
    stringLenZeroChkReturn(Significance);
    stringLenZeroChkReturn(StartTime);
    stringLenZeroChkReturn(EndTime);
    stringLenZeroChkReturn(start);
    stringLenZeroChkReturn(length);
    stringLenZeroChkReturn(draw);    

    logLookUpCtrl->start = atoi(start);
    logLookUpCtrl->length = atoi(length);
    logLookUpCtrl->draw = atoi(draw);

    if (strstr(LogType, "all"))
        logLookUpCtrl->logType = LOGTYPE_ALL;
    else if (strstr(LogType, "user"))
        logLookUpCtrl->logType = LOGTYPE_USER;
    else if (strstr(LogType, "system"))
        logLookUpCtrl->logType = LOGTYPE_SYSTEM;
    else
    {
        CGIDEBUG ("LogType[%s] invalid\n", LogType);
//        CGICASSERT(0);
        return -1;
    }

    if (strstr(Significance, "all"))
        logLookUpCtrl->logSignificance = LOGSIGNIFICANCE_ALL;
    else if (strstr(Significance, "general"))
        logLookUpCtrl->logSignificance = LOGSIGNIFICANCE_GENERAL;
    else if (strstr(Significance, "key"))
        logLookUpCtrl->logSignificance = LOGSIGNIFICANCE_KEY;
    else
    {
        CGIDEBUG ("Significance[%s] invalid\n", Significance);
//        CGICASSERT(0);
        return -1;
    }

    logLookUpCtrl->startTime = format2time(StartTime);
    logLookUpCtrl->endTime = format2time(EndTime);

    m->type = MSGTYPE_LOGLOOKUP_REQUEST;
    
    return 0;
}

static const formMethod formMethodTable[] = 
{
    {"Function_Login", login},
    {"Function_Lan1Test", lan1Test},
    {"Function_Lan2Test", lan2Test},
    {"Function_NetConfigSave", netConfigSave},
    {"Function_NetCapture", netCapture},
    {"Function_NetFilter", netFilter},
    {"Function_FileLookUp", fileLookUp},
    {"Function_FileUpLoad", fileUpLoad},    
    {"Function_DiskInfo", diskInfo},
    {"Function_NormalUserMgr", normalUserMgr},
    {"Function_SuperUserMgr", superUserMgr},
    {"Function_SystemInfo", systemInfo},
    {"Function_SystemTimeGet", systemTimeGet},
    {"Function_SystemTimeSet", systemTimeSet},
    {"Function_GetVersion", getVersion},
    {"Function_FactoryReset", factoryReset},
    {"Function_Reboot", reboot},
    {"Function_LogLookUp", logLookUp},
};

static formMethod *formMethodLookUp (char *formName)
{
    int i;    

    for(i = 0; i < sizeof(formMethodTable)/sizeof(formMethodTable[0]); i++)
    {
        if (strstr(formName, formMethodTable[i].formName))
            return (formMethod *)&formMethodTable[i];
    }

    return NULL;
}

static void confirm2json (msg *m)
{
    confirmResponse *resp = (confirmResponse *)m->data;

    cgiHeaderContentType("text/html");
    
    fprintf(cgiOut, "{\r\n");
    fprintf(cgiOut, "\"status\":\"%d\",\r\n", resp->status);
    fprintf(cgiOut, "\"message\":\"%s\"\r\n", resp->errMessage);
    fprintf(cgiOut, "}");
}

static in_addr_t getNetIp(char *netName)
{
    int sock;    
    int res;    
    struct ifreq ifr;     

    sock = socket(AF_INET, SOCK_STREAM, 0);    
    strcpy(ifr.ifr_name, netName);    
    res = ioctl(sock, SIOCGIFADDR, &ifr);     
//    CGIDEBUG("IP: %s\n",inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));     

    return ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
}

static void fileLookUpResp2json(msg *m)
{
    int i;
    char buffer[50];
    struct hostent *hent;
    fileLookUpResponse *resp = (fileLookUpResponse *)m->data;
    in_addr_t hostAddr;

    cgiHeaderContentType("text/html");
    
    fprintf(cgiOut, "{\r\n");
    fprintf(cgiOut, "\"recordsFiltered\":\"%d\",\r\n", resp->recordsTotal);
    fprintf(cgiOut, "\"draw\":\"%d\",\r\n", resp->draw);
    fprintf(cgiOut, "\"recordsTotal\":\"%d\",\r\n", resp->recordsTotal);
    fprintf(cgiOut, "\"data\":[\r\n");
    for (i = 0; i < resp->length; i++)
    {
        fprintf(cgiOut, "{\"fileName\":\"%s\",\r\n", resp->elements[i].fileName);
        time2format(resp->elements[i].modifyTime, buffer);
        fprintf(cgiOut, "\"modifyTime\":\"%s\",\r\n", buffer);
        fprintf(cgiOut, "\"sizeMB\":\"%d\",\r\n", resp->elements[i].sizeMB);
        gethostname(buffer, sizeof(buffer));
//        hostAddr = getNetIp(NET1_NAME);
//        inet_ntop(AF_INET, (void *)&hostAddr, buffer, 50);
        fprintf(cgiOut, "\"url\":\"/NetFiles/%s\"}\r\n", resp->elements[i].fileName);
        if (i != (resp->length - 1))
            fprintf(cgiOut, ",");
    }
    
    fprintf(cgiOut, "]}");
}

static void fileUpLoadResp2http(msg *m)
{
    char fileName[50];
    char buffer[512];
    fileUpLoadResponse *resp = (fileLookUpResponse *)m->data;
    struct stat s;
    FILE * fp;
    int len;

    snprintf(fileName, sizeof(fileName), "%s%s", NET_FILES_PATH, resp->fileName);
    stat(fileName, &s);

//    cgiHeaderContentType("text/html;charset=UTF-8");
    cgiHeaderContentType("application/octet-stream");
    fprintf(cgiOut, "Content-Disposition:filename=\"%s\"", resp->fileName);
    fprintf(cgiOut, "Content-Length:%d", s.st_size);
    

    if (fp = fopen(fileName, "r"))
    {
        while((len = fread(buffer, sizeof(buffer), 1, fp)) > 0)
        {
            fwrite(buffer, len, 1, cgiOut);
        }
        fclose(fp);
    }
    else
    {
        CGIDEBUG("file %s fopen failed !!!", fileName);
    }
}

static void sysTimeGetResp2json(msg *m)
{
    sysTimeGetResponse *resp = (sysTimeGetResponse *)m->data;
    char fmt[20];

    cgiHeaderContentType("text/html");
    
    time2format(resp->currentTime, fmt);    
    fprintf(cgiOut, "{\"currentTime\":\"%d\"}\r\n", fmt);
}

static void getVersionResp2json(msg *m)
{
    versionGetResponse *resp = (versionGetResponse *)m->data;

    cgiHeaderContentType("text/html");
    
    fprintf(cgiOut, "{\r\n");
    fprintf(cgiOut, "\"LogicVersion\":\"%s\",\r\n", resp->logicVersion);
    fprintf(cgiOut, "\"CfgMgrVersion\":\"%s\"\r\n", resp->cfgMgrVersion);
    fprintf(cgiOut, "}");
}

static void logLookUpResp2json(msg *m)
{
    int i;
    char buffer[50];
    struct hostent *hent;
    logLookUpResponse *resp = (logLookUpResponse *)m->data;
    in_addr_t hostAddr;

    cgiHeaderContentType("text/html");
    
    fprintf(cgiOut, "{\r\n");
    fprintf(cgiOut, "\"recordsFiltered\":\"%d\",\r\n", resp->recordsTotal);
    fprintf(cgiOut, "\"draw\":\"%d\",\r\n", resp->draw);
    fprintf(cgiOut, "\"recordsTotal\":\"%d\",\r\n", resp->recordsTotal);
    fprintf(cgiOut, "\"data\":[\r\n");
    for (i = 0; i < resp->length; i++)
    {
        time2format(resp->elements[i].occurTime, buffer);
        fprintf(cgiOut, "\"LogTime\":\"%s\",\r\n", buffer);
        fprintf(cgiOut, "{\"LogType\":\"%s\",\r\n", 
            (resp->elements[i].typ == LOGTYPE_SYSTEM) ? "system":"user";
        fprintf(cgiOut, "\"Significance\":\"%s\",\r\n", 
            (resp->elements[i].sgnfcc == LOGSIGNIFICANCE_GENERAL) ? "genarl":"key");
        fprintf(cgiOut, "\"content\":\"%s\",\r\n", resp->elements[i].content);
        if (i != (resp->length - 1))
            fprintf(cgiOut, ",");
    }
    
    fprintf(cgiOut, "]}");
}


static void msg2json(msg *m)
{
    switch(m->type)
    {
        case MSGTYPE_COMFIRM:
            confirm2json(m);
            break;
        case MSGTYPE_FILELOOKUP_RESPONSE:
            fileLookUpResp2json(m);
            break;
        case MSGTYPE_FILEUPLOAD_RESPONSE:
            fileUpLoadResp2http(m);
            break;
        case MSGTYPE_SYSTIMEGET_RESPONSE:
            sysTimeGetResp2json(m);
            break;
        case MSGTYPE_GETVERSION_RESPONSE:
            getVersionResp2json(m);
            break;
        case MSGTYPE_LOGLOOKUP_RESPONSE:
            logLookUpResp2json(m);
            break;
        default:
            break;
    }
}

int cgiMain()
{
    int                ret = 0;
    msg                m;
    msgID              mId;
    formMethod         *fmMethod;
    char formName      [FORM_ELEMENT_STRING_LEN_MAX] = {0};

    functionName = NULL;
	cgiFormString("FunctionName", formName, FORM_ELEMENT_STRING_LEN_MAX);

    if (!strlen(formName))
    {
        CGIDEBUG("FunctionName didn't found.\n");
        return -1;
    }
    functionName = formName;

    if (NULL != (fmMethod = formMethodLookUp(formName)))
    {
        if (fmMethod->func(&m) != 0)
        {
            CGIDEBUG("fmMethod done failed !!!\n");
            return -1;
        }
    }
    else
    {
        CGIDEBUG("formMethodLookUp failed, formName :%s\n", formName);
//        CGICASSERT(0);
        return -1;
    }


    if ((msgID)-1 == (mId = msgOpen (CGI_CFGMGR_MSG_NAME)))
	{
        CGIDEBUG("msgOpen %s failed !!!\n", CGI_CFGMGR_MSG_NAME);
//        CGICASSERT(0);
        return -1;
    }

    if(0 != msgSend(mId, &m))
    {
        CGIDEBUG("msgSend failed !!!\n");
        ret = -1;
        goto closeMsg;
    }

    if(0 >= msgRecv(mId, &m))
    {
        CGIDEBUG("msgRecv failed !!!\n");
        ret = -1;
        goto closeMsg;
    }

    msg2json(&m);

closeMsg:
    msgClose(mId);
    
	return ret;
}

