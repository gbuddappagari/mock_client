#include <stdio.h>
#include "libparodus.h"
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "wrp-c.h"
#include "wdmp-c.h"
#include "cJSON.h"
#include <string.h>
#include <cimplog.h>

#define CONTENT_TYPE_JSON  "application/json"
#define LOGGING_MODULE     "MOCK_CLIENT"
#define Error(...)      cimplog_error(LOGGING_MODULE, __VA_ARGS__)
#define Info(...)       cimplog_info(LOGGING_MODULE, __VA_ARGS__)
#define Print(...)      cimplog_debug(LOGGING_MODULE, __VA_ARGS__)

libpd_instance_t iot_instance;

const char *rdk_logger_module_fetch(void)
{
    return "LOG.RDK.MOCK_CLIENT";
}

static void processRequest(char *reqPayload, char **resPayload)
{
    req_struct *reqObj = NULL;
    res_struct *resObj = NULL;
    char *payload = NULL;
    int paramCount,i;
    WDMP_STATUS ret = WDMP_SUCCESS;
    wdmp_parse_request(reqPayload,&reqObj);

    if(reqObj != NULL)
    {
        Print("Request:> Type : %d\n",reqObj->reqType);
        resObj = (res_struct *) malloc(sizeof(res_struct));
        memset(resObj, 0, sizeof(res_struct));
        resObj->reqType = reqObj->reqType;
        Print("Response:> type = %d\n", resObj->reqType);

        if(reqObj->reqType == GET)
        {
            Print("Request:> ParamCount = %zu\n",reqObj->u.getReq->paramCnt);
            resObj->paramCnt = reqObj->u.getReq->paramCnt;
            Print("Response:> paramCnt = %zu\n", resObj->paramCnt);
            resObj->retStatus = (WDMP_STATUS *) malloc(sizeof(WDMP_STATUS)*resObj->paramCnt);
            resObj->timeSpan = NULL;
            paramCount = (int)reqObj->u.getReq->paramCnt;
            
            resObj->u.getRes = (get_res_t *) malloc(sizeof(get_res_t));
            memset(resObj->u.getRes, 0, sizeof(get_res_t));

            resObj->u.getRes->paramCnt = reqObj->u.getReq->paramCnt;
            resObj->u.getRes->paramNames = (char **) malloc(sizeof(char *) * resObj->u.getRes->paramCnt);
            resObj->u.getRes->retParamCnt = (size_t *) malloc(sizeof(size_t)*paramCount);

            resObj->u.getRes->params = (param_t **) malloc(sizeof(param_t*)*paramCount);
            memset(resObj->u.getRes->params, 0, sizeof(param_t*)*paramCount);

            for(i = 0; i < paramCount; i++)
            {
                resObj->u.getRes->params[i] = (param_t *) malloc(sizeof(param_t));
                resObj->u.getRes->params[i][0].name = (char*) malloc (sizeof(char)*100);
                resObj->u.getRes->params[i][0].value = (char*) malloc (sizeof(char)*100);
                strcpy(resObj->u.getRes->params[i][0].name, reqObj->u.getReq->paramNames[i]);
                strcpy(resObj->u.getRes->params[i][0].value, "value");
                resObj->u.getRes->params[i][0].type = 0;
                
                resObj->u.getRes->paramNames[i] = reqObj->u.getReq->paramNames[i];
                Print("Response:> paramNames[%d] = %s\n",i,resObj->u.getRes->paramNames[i]);
                resObj->u.getRes->retParamCnt[i] = 1;
                Print("Response:> retParamCnt[%d] = %zu\n",i,resObj->u.getRes->retParamCnt[i]);
                resObj->retStatus[i] = ret;
                Print("Response:> retStatus[%d] = %d\n",i,resObj->retStatus[i]);
            }
        }
        else if(reqObj->reqType == GET_ATTRIBUTES)
        {
            Print("Request:> ParamCount = %zu\n",reqObj->u.getReq->paramCnt);
            resObj->paramCnt = reqObj->u.getReq->paramCnt;
            Print("Response:> paramCnt = %zu\n", resObj->paramCnt);
            resObj->retStatus = (WDMP_STATUS *) malloc(sizeof(WDMP_STATUS)*resObj->paramCnt);
            resObj->timeSpan = NULL;
            paramCount = (int)reqObj->u.getReq->paramCnt;
            resObj->u.paramRes = (param_res_t *) malloc(sizeof(param_res_t));
            memset(resObj->u.paramRes, 0, sizeof(param_res_t));

            resObj->u.paramRes->params = (param_t *) malloc(sizeof(param_t)*paramCount);
            memset(resObj->u.paramRes->params, 0, sizeof(param_t)*paramCount);

            for (i = 0; i < paramCount; i++) 
            {
                resObj->u.paramRes->params[i].name = (char*) malloc (sizeof(char)*100);
                resObj->u.paramRes->params[i].value = (char*) malloc (sizeof(char)*100);
                
                strcpy(resObj->u.paramRes->params[i].name, reqObj->u.getReq->paramNames[i]);
                Print("Response:> params[%d].name = %s\n",i,resObj->u.paramRes->params[i].name);
                strcpy(resObj->u.paramRes->params[i].value, "1");
                Print("Response:> params[%d].value = %s\n",i,resObj->u.paramRes->params[i].value);
                resObj->u.paramRes->params[i].type = WDMP_INT;
                Print("Response:> params[%d].type = %d\n",i,resObj->u.paramRes->params[i].type);

                resObj->retStatus[i] = ret;
                Print("Response:> retStatus[%d] = %d\n",i,resObj->retStatus[i]);
            }
        }
        else if((reqObj->reqType == SET) || (reqObj->reqType == SET_ATTRIBUTES))            
        {
            Print("Request:> ParamCount = %zu\n",reqObj->u.setReq->paramCnt);
            resObj->paramCnt = reqObj->u.setReq->paramCnt;
            Print("Response:> paramCnt = %zu\n", resObj->paramCnt);
            resObj->retStatus = (WDMP_STATUS *) malloc(sizeof(WDMP_STATUS)*resObj->paramCnt);
            resObj->timeSpan = NULL;
            paramCount = (int)reqObj->u.setReq->paramCnt;
            resObj->u.paramRes = (param_res_t *) malloc(sizeof(param_res_t));
            memset(resObj->u.paramRes, 0, sizeof(param_res_t));
            resObj->u.paramRes->params = (param_t *) malloc(sizeof(param_t)*paramCount);
            memset(resObj->u.paramRes->params, 0, sizeof(param_t)*paramCount);
            
            for (i = 0; i < paramCount; i++) 
            {
                Print("Request:> param[%d].name = %s\n",i,reqObj->u.setReq->param[i].name);
                Print("Request:> param[%d].value = %s\n",i,reqObj->u.setReq->param[i].value);
                Print("Request:> param[%d].type = %d\n",i,reqObj->u.setReq->param[i].type);
                
                resObj->u.paramRes->params[i].name = (char *) malloc(sizeof(char) * 512);
                strcpy(resObj->u.paramRes->params[i].name, reqObj->u.setReq->param[i].name);
                Print("Response:> params[%d].name = %s\n",i,resObj->u.paramRes->params[i].name);
                resObj->u.paramRes->params[i].value = NULL;
                resObj->u.paramRes->params[i].type = 0;
                
                resObj->retStatus[i] = ret;
                Print("Response:> retStatus[%d] = %d\n",i,resObj->retStatus[i]);
            }
        }
    }
    
    wdmp_form_response(resObj,&payload);
    Print("payload : %s\n",payload);
    *resPayload = payload;

    Info("Response:> Payload = %s\n", *resPayload);

    if(NULL != reqObj)
    {
        wdmp_free_req_struct(reqObj);
    }
    if(NULL != resObj)
    {
        wdmp_free_res_struct(resObj);
    }
}

static void connect_parodus()
{
    
    libpd_cfg_t cfg = { .service_name = "config",
                        .receive = true, 
                        .keepalive_timeout_secs = 64,
                        .parodus_url = "tcp://127.0.0.1:6666",
                        .client_url = "tcp://127.0.0.1:6663"
                      };
                      
    Info("Configurations => service_name : %s parodus_url : %s client_url : %s\n", cfg.service_name, cfg.parodus_url, cfg.client_url );
    
    while(1)
    {
        int ret = libparodus_init (&iot_instance, &cfg);

        if(ret ==0)
        {
            Info("Init for parodus Success..!!\n");
            break;
        }
        else
        {
            Error("Init for parodus failed: '%s'\n",libparodus_strerror(ret));
            sleep(5);
        }
	    libparodus_shutdown(&iot_instance);
        
    }
}

void *parodus_receive_wait()
{
    int rtn;
    wrp_msg_t *wrp_msg;
    wrp_msg_t *res_wrp_msg ;
    char *contentType = NULL;

    while (1) 
    {
        rtn = libparodus_receive (iot_instance, &wrp_msg, 2000);
        if (rtn == 1) 
        {
            continue;
        }

        if (rtn != 0)
        {
            Error ("Libparodus failed to recieve message: '%s'\n",libparodus_strerror(rtn));
            sleep(5);
            continue;
        }

        if (wrp_msg->msg_type == WRP_MSG_TYPE__REQ)
        {
            res_wrp_msg = (wrp_msg_t *)malloc(sizeof(wrp_msg_t));
            memset(res_wrp_msg, 0, sizeof(wrp_msg_t));

            Info("Request message : %s\n",(char *)wrp_msg->u.req.payload);
            processRequest((char *)wrp_msg->u.req.payload, ((char **)(&(res_wrp_msg->u.req.payload))));

            Info("Response payload is %s\n",(char *)(res_wrp_msg->u.req.payload));
            res_wrp_msg->u.req.payload_size = strlen(res_wrp_msg->u.req.payload);
            res_wrp_msg->msg_type = wrp_msg->msg_type;
            res_wrp_msg->u.req.source = wrp_msg->u.req.dest;
            res_wrp_msg->u.req.dest = wrp_msg->u.req.source;
            res_wrp_msg->u.req.transaction_uuid = wrp_msg->u.req.transaction_uuid;
            contentType = (char *)malloc(sizeof(char)*(strlen(CONTENT_TYPE_JSON)+1));
            strncpy(contentType,CONTENT_TYPE_JSON,strlen(CONTENT_TYPE_JSON)+1);
            res_wrp_msg->u.req.content_type = contentType;

            int sendStatus = libparodus_send(iot_instance, res_wrp_msg);     
            Print("sendStatus is %d\n",sendStatus);
            if(sendStatus == 0)
            {
                Info("Sent message successfully to parodus\n");
            }
            else
            {
                Error("Failed to send message: '%s'\n",libparodus_strerror(sendStatus));
            }
            wrp_free_struct (res_wrp_msg); 
        }
        free(wrp_msg);
    }

    libparodus_shutdown(&iot_instance);
    Print ("End of parodus_upstream\n");
    return 0;
}

static void startParodusReceiveThread()
{
    int err = 0;
    pthread_t threadId;

    err = pthread_create(&threadId, NULL, parodus_receive_wait, NULL);
    if (err != 0) 
    {
        Error("Error creating thread :[%s]\n", strerror(err));
        exit(1);
    }
    else
    {
        Info("Parodus Receive wait thread created Successfully %d\n", (int ) threadId);
    }    
}

int main()
{
    connect_parodus();
    startParodusReceiveThread();
    sleep(5);

    while(1)
    {
        sleep(10);
    }
    return 0;	
}
