/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/ipc_notify.h>
#include <drivers/ipc_rpmsg.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "FreeRTOS.h"
#include "timers.h"

#include <drivers/gtc.h>

#include <drivers/soc.h>

#include "timestamp.h"

/* This example shows message exchange between multiple cores.
 *
 * One of the core is designated as the 'main' core
 * and other cores are designated as `remote` cores.
 *
 * The main core initiates IPC with remote core's by sending it a message.
 * The remote cores echo the same message to the main core.
 *
 * The main core repeats this for gMsgEchoCount iterations.
 *
 * In each iteration of message exchange, the message value is incremented.
 *
 * When iteration count reaches gMsgEchoCount, the example is completed.
 *
 */

/* number of iterations of message exchange to do */
uint32_t gMsgEchoCount = 100000u;

#if defined(SOC_AM243X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};

#endif

#if defined (SOC_AM263X) || defined (SOC_AM263PX) || defined (SOC_AM261X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined(SOC_AM64X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_M4FSS0_0,
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined(SOC_AM273X) || defined(SOC_AWR294X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_C66SS0,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

/*
 * Remote core service end point
 *
 * pick any unique value on that core between 0..RPMESSAGE_MAX_LOCAL_ENDPT-1
 * the value need not be unique across cores
 */
uint16_t gRemoteServiceEndPt = 13u;

//uint64_t timeStamps[12u];
//uint64_t timeStampIterator = 0;

/* maximum size that message can have in this example */
#define MAX_MSG_SIZE        (1152u)

/* Main core ack reply end point
 *
 * pick any unique value on that core between 0..RPMESSAGE_MAX_LOCAL_ENDPT-1
 * the value need not be unique across cores
 */
#define MAIN_CORE_ACK_REPLY_END_PT  (12U)

/* RPMessage_Object MUST be global or static */
RPMessage_Object gAckReplyMsgObject;

uint32_t iteratorR500;
uint32_t max_iR500;

TimeStamps timestamp_arrayR500[1000u] = {0u};


//uint64_t timestampR500 __attribute((section(".bss.project_shared_mem")));

void calculate_time()
{
    uint32_t i;
    uint64_t latency = 0;

    for(i = 0; i < 1000u; i++)
    {
        DebugP_log("%u. ", i);
        //DebugP_log("main to remote R : %u\n", timestamp_arrayR500[i].main_to_remote_R);
        //DebugP_log("main to remote S : %u\n", timestamp_arrayR500[i].main_to_remote_S);
        latency = timestamp_arrayR500[i].main_to_remote_R - timestamp_arrayR500[i].main_to_remote_S;
        DebugP_log("  %u ", latency);

        //DebugP_log("remote to main R : %u\n", timestamp_arrayR500[i].remote_to_main_R);
        //DebugP_log("remote to main S : %u\n", timestamp_arrayR500[i].remote_to_main_S);
        latency = timestamp_arrayR500[i].remote_to_main_R - timestamp_arrayR500[i].remote_to_main_S;
        DebugP_log("  %u\n", latency);
    }
}



void ipc_rpmsg_echo_main_core_start(void)
{
    int32_t status_clk_set;

    uint32_t moduleIdR500 = TISCI_DEV_R5FSS0_CORE0;
    uint32_t clkIdR500 = TISCI_DEV_R5FSS0_CORE0_CPU_CLK;
    uint64_t clkRatetoSETR500 = 800000000U;

    status_clk_set = SOC_moduleSetClockFrequency(moduleIdR500, clkIdR500, clkRatetoSETR500);
    DebugP_assert(status_clk_set==SystemP_SUCCESS);

    RPMessage_CreateParams createParams;

    char txBuf[MAX_MSG_SIZE] = "aaaaaaaa";
    char rxBuf[MAX_MSG_SIZE];


    uint16_t rxBufSize;

    int32_t status;
    uint16_t remoteCoreId, remoteCoreEndPt, msgSize;

    max_iR500 = 1000u;

    GTC_enable();

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = MAIN_CORE_ACK_REPLY_END_PT;
    status = RPMessage_construct(&gAckReplyMsgObject, &createParams);
    DebugP_assert(status==SystemP_SUCCESS);

    uint16_t core_i;

    for(core_i = 0; gRemoteCoreId[core_i]!=CSL_CORE_ID_MAX; core_i++)
    {
        IpcNotify_sendSync(gRemoteCoreId[core_i]);

        /* send the same messages to all cores */
        for(iteratorR500 = 0; iteratorR500 < max_iR500; iteratorR500++ )
        {
            timestamp_arrayR500[iteratorR500].main_to_remote_S = GTC_getCount64();

            status = RPMessage_send(
                txBuf, strlen(txBuf),
                gRemoteCoreId[core_i], 13,
                RPMessage_getLocalEndPt(&gAckReplyMsgObject),
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);


            rxBufSize = sizeof(rxBuf);
            status = RPMessage_recv(&gAckReplyMsgObject,
                rxBuf, &rxBufSize,
                &remoteCoreId, &remoteCoreEndPt,
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);

            timestamp_arrayR500[iteratorR500].remote_to_main_R = GTC_getCount64();

        //DebugP_log("Round: %u\n", iteratorR500);
        }

        for(iteratorR500 = 0; iteratorR500 < max_iR500; iteratorR500++ )
        {
            rxBufSize = (2u * sizeof(timestamp_arrayR500[iteratorR500].main_to_remote_R));
            status = RPMessage_recv(&gAckReplyMsgObject,
                &timestamp_arrayR500[iteratorR500].main_to_remote_R, &rxBufSize,
                &remoteCoreId, &remoteCoreEndPt,
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
        }
        calculate_time();
    }


    //calculate_time();
    RPMessage_destruct(&gAckReplyMsgObject);

    DebugP_log("All tests have passed!!\r\n");
}

/* RPMessage_Object MUST be global or static */
static RPMessage_Object gRecvMsgObject;

void ipc_rpmsg_echo_remote_core_start(void)
{

/*
    int32_t status;
    RPMessage_CreateParams createParams;
    char recvMsg[MAX_MSG_SIZE];
    uint16_t recvMsgSize, remoteCoreId, remoteCoreEndPt;

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = gRemoteServiceEndPt;
    status = RPMessage_construct(&gRecvMsgObject, &createParams);
    DebugP_assert(status==SystemP_SUCCESS);


    IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    DebugP_log("[IPC RPMSG ECHO] Remote Core waiting for messages from main core ... !!!\r\n");


    while(1)
    {

        recvMsgSize = sizeof(recvMsg);
        status = RPMessage_recv(&gRecvMsgObject,
            recvMsg, &recvMsgSize,
            &remoteCoreId, &remoteCoreEndPt,
            SystemP_WAIT_FOREVER);
        DebugP_assert(status==SystemP_SUCCESS);




        status = RPMessage_send(
            recvMsg, recvMsgSize,
            remoteCoreId, remoteCoreEndPt,
            RPMessage_getLocalEndPt(&gRecvMsgObject),
            SystemP_WAIT_FOREVER);
        DebugP_assert(status==SystemP_SUCCESS);
    }
*/
}

void ipc_rpmsg_echo_main(void *args)
{
    Drivers_open();
    Board_driversOpen();

    if(IpcNotify_getSelfCoreId()==gMainCoreId)
    {
        ipc_rpmsg_echo_main_core_start();
    }
    else
    {
        ipc_rpmsg_echo_remote_core_start();
    }

    Board_driversClose();
    /* We dont close drivers to let the UART driver remain open and flush any pending messages to console */
    /* Drivers_close(); */
}
