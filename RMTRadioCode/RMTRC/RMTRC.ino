/*
   FreeRTOS+CLI V1.0.4
   Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
   the Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   http://www.FreeRTOS.org
   http://aws.amazon.com/freertos
*/
#include <FreeRTOS_CLI_vESP32.h>
#include <FreeRTOS_CLI_vESP32.c>
#include <RCSwitch.h>

/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.

  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
  - Dominik Fischer / dom_fischer(at)web(dot)de
  - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
  - Max Horn / max(at)quendi(dot)de
  - Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com

  Project home: https://github.com/sui77/rc-switch/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "driver/pcnt.h"

/* PIN assignements */

#define RCSRcvPIN  14           /* Receive Pin for rc-switch  */
#define RMTRcvPIN  27           /* Receive Pin for RMT Rx  */

#define RCSXmitPIN 16    /* Transmit PIN for rc-switch */
#define RMTXmitPIN 17    /* Xmit PIN for RMT Tx */

#define led 2            /*on board led */

/* added */
#define CLI_OUT_SIZE  550
#define SPOOL_SIZE    600
#define LINE_BUF_SIZE 280

#ifndef configAPPLICATION_PROVIDES_cOutputBuffer
#define configAPPLICATION_PROVIDES_cOutputBuffer 0
#endif

BaseType_t errorReturn  = pdFALSE;
static char *cOutBuffer ,  cspool[SPOOL_SIZE];
static char vcharline[LINE_BUF_SIZE], cline[LINE_BUF_SIZE], csend[LINE_BUF_SIZE] ;

uint16_t Delta = 50; //microseconds tolerance

/*-----------------*/
/* new added sub routine commands */
static BaseType_t prvRCSXmitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvRCScfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static BaseType_t prvDeltacfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvDeltashowCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static BaseType_t prvRMTcfgProtocol( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvRMTXmitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvRMTshowCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

struct cmd_table_t {
  char cmd_token;
  uint16_t ivalue;
};

struct table_str {
  char idxname[30];
  uint16_t idxvalue;
};

struct pulseRcvd_t {
  char symbol;
  uint16_t high;
  uint16_t low;
};

struct RCS_rcvd_t {
  unsigned long Value;
  unsigned int Bitlength;
  unsigned int Delay;
  unsigned int Protocol;
  unsigned int* rawData;
};

struct rmt_t {
  uint32_t u32RMTresult;
  uint16_t datalen;
  uint16_t oot;
  rmt_data_t rcv_data[25];
};

struct timings_t {
  uint16_t TSynHigh;
  uint16_t TSynLow;
  uint16_t T0High;
  uint16_t T0Low;
  uint16_t T1High;
  uint16_t T1Low;
} timings[] = {
  {350, 10850, 350, 1050, 1050, 350}, // timings[0]-> values in use
  {350, 10850, 350, 1050, 1050, 350},
  {650, 6500, 650, 1300, 1300, 650},
  {3000, 7100, 400, 1100, 900, 600},
  {380, 2280, 380, 1140, 1140, 380},
  {3000, 7000, 500, 1000, 1000, 500},
};

/*-------------lookup -----------------*/
BaseType_t exactfind(const table_str * table, const char * str2, esp_err_t *OpResult );

uint8_t usParamlookup (char pccmd, const struct cmd_table_t *ptr, bool * lookupResult );
uint16_t uvLookupTable (uint16_t ParamNbr, const char *pcCommand, const cmd_table_t *ParamList, esp_err_t *OpResult);
uint16_t uvParamStr2uInt (const char *pcCommandString, UBaseType_t IntParam);
uint16_t ivParamStr2signedInt (const char *pcCommandString, UBaseType_t signedIntParam);
uint32_t uvParamStr2u32Int (const char *pcCommandString, UBaseType_t IntParam);

void prvOut();

static xQueueHandle xQSerialIn = NULL, xQrcs_rcvd = NULL,  xQcode = NULL , xQrmtrx = NULL, xQSpooler = NULL ;

extern "C" void rmtrcvcb(uint32_t *data, size_t len)
{
  rmt_t rmt_msg;
  struct {
    union {
      uint32_t databit;
      rmt_data_t rmtbit;
    };
  } rxbit;
  uint32_t u32code = 0;
  uint16_t value0, value1;
  uint16_t bit_tolerance, diff, oot;
  bool error_found = false;

  if ( len == 25 )
  {
    bit_tolerance = (timings[0].T1High - timings[0].T1Low - (2 * Delta));
    rmt_msg.u32RMTresult = 0 ;
    rmt_msg.oot = 0;
    for (uint16_t i = 0; i < 24; i++)
    {
      rxbit.databit = data[i];
      rmt_msg.rcv_data[i] = rxbit.rmtbit;
      value0 = (uint16_t)rxbit.rmtbit.duration0;
      value1 = (uint16_t)rxbit.rmtbit.duration1;
      diff = abs(value0 - value1);
      if (diff < bit_tolerance)
      { rmt_msg.oot++;
      }
      if ((value0 - value1) > 0)
      {
        u32code = ( u32code | (1ULL << (23 - i))); /* "1" */
      }
    }
    rmt_msg.datalen = len;
    rmt_msg.u32RMTresult = u32code;
    xQueueSend(xQrmtrx, &rmt_msg, NULL );
    return;
  }
  else
  {
    return;
  }
}


void createQueues() {

  strcat(cOutBuffer, "\r\nQueues created: ");


  xQSerialIn = xQueueCreate(10, sizeof( vcharline ) ); //  char * ) );
  if (xQSerialIn != 0) {
    strcat(cOutBuffer, "xQSerialIn, ");
  };
  xQrcs_rcvd = xQueueCreate(10, sizeof(RCS_rcvd_t));
  if (xQrcs_rcvd != 0) {
    strcat(cOutBuffer, "xQrcs_rcvd, ");
  };
  xQcode = xQueueCreate(100, sizeof(pulseRcvd_t));
  if (xQcode != 0) {
    strcat(cOutBuffer, "xQcode, ");
  };
  xQrmtrx = xQueueCreate(10, sizeof(rmt_t));
  if (xQrmtrx != 0) {
    strcat(cOutBuffer, "xQrmtrx,  ");
  };
  xQSpooler = xQueueCreate(5, sizeof(cspool));
  if (xQSpooler != 0) {
    strcat(cOutBuffer, "xQSpooler \r\n");
  };
  prvOut();
}

void $Serial ( void* );
void $RCSrcv ( void* );
void $RMTRx ( void* );
void $Spool (void*);

TaskHandle_t xHandleRMT = NULL, xHandleSerial = NULL, xHandleRCSrcv = NULL, xHandleSpool = NULL ;

void createTasks() {
  BaseType_t xReturned;

  strcat(cOutBuffer, "\r\nTasks created: \r\n");

  xReturned = xTaskCreate($Spool, "$Spool", 2048 , NULL, 10, &xHandleSpool);
  if ( xReturned == pdPASS ) {
    strcat(cOutBuffer, "$Spool, ");
  };
  xReturned = xTaskCreate($RCSrcv, "$RCSrcv", 2048 , NULL, 10, &xHandleRCSrcv);
  if ( xReturned == pdPASS ) {
    strcat(cOutBuffer, "$RCSrcv, ");
  };
  xReturned = xTaskCreate($Serial, "$Serial", 2048, NULL, 10, &xHandleSerial);
  if ( xReturned == pdPASS ) {
    strcat(cOutBuffer, "$Serial, ");
  };
  xReturned = xTaskCreate($RMTRx, "$RMTRx", 2048, NULL, 10, &xHandleRMT);
  if ( xReturned == pdPASS ) {
    strcat(cOutBuffer, "$RMTRx \r\n");
  };
  prvOut();
}


void registerCommands();
void registerRCSCommands();
void registerRMTCommands();
void setupgpio();
void RCSinit();
void xvinitRMT();

/*---  Arduino main    */

void setup() {
  BaseType_t xerrorReturn;

  Serial.begin(115200);               /* check baud rate of Serial Monitor coonsole */
  cOutBuffer = FreeRTOS_CLIGetOutputBuffer();
  createQueues();
  RCSinit();
  createTasks();
  setupgpio();
  xvinitRMT();
  registerCommands();

  Serial.println ("\r\n RMT Radio Code v.00 \r\n based on FreeRTOS+CLI(c), esp-idf(c) API's and RCSwitch(c)");
  Serial.printf("\r\n Input  PINs, for RCSwitch PIN#: %d, for RMTRx PIN#: %d ", RCSRcvPIN, RMTRcvPIN );
  Serial.printf("\r\n Output PINs, for rcsxmit : PIN# %d, for rmtxmit: PIN# %d \r\n", RCSXmitPIN, RMTXmitPIN);
}


void loop()
{
  RCS_rcvd_t RCS_rcvd;
  rmt_t rmt;
  pulseRcvd_t pulseMsg;
  char rcvline[LINE_BUF_SIZE];

  if (xQueueReceive(xQSerialIn, rcvline, (20 / portTICK_PERIOD_MS)))
  {
    errorReturn = pdFALSE;
    errorReturn = FreeRTOS_CLIProcessCommand( rcvline, cOutBuffer, (size_t) CLI_OUT_SIZE  );
    strcpy ( cspool, cOutBuffer);
    memset ( cOutBuffer, NULL, configCOMMAND_INT_MAX_OUTPUT_SIZE);
    xQueueSend(xQSpooler, cspool, NULL);

    while (errorReturn == pdTRUE)
    {
      errorReturn = FreeRTOS_CLIProcessCommand( rcvline, cOutBuffer, (size_t) CLI_OUT_SIZE  );
      strcpy ( cspool, cOutBuffer);
      memset ( cOutBuffer, NULL, configCOMMAND_INT_MAX_OUTPUT_SIZE);
      xQueueSend(xQSpooler, cspool, NULL);
    };
  }

  if (xQueueReceive(xQcode, &pulseMsg , 0)) {
    if (pulseMsg.symbol == 'S')  {
      sprintf(cspool, "Pulse SYN rcvd, \tTLow: % d, (microsec)", pulseMsg.low);
    }
    xQueueSend(xQSpooler, &cspool, NULL);
  }


  if (xQueueReceive(xQrmtrx, &rmt, 0)) {
    sprintf(cspool, "RMT code % d  bit length  = %d oot: %d\r\nRMT data received \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n", \
            rmt.u32RMTresult, rmt.datalen , rmt.oot, \
            rmt.rcv_data[0].duration0, rmt.rcv_data[0].duration1, rmt.rcv_data[1].duration0, rmt.rcv_data[1].duration1, rmt.rcv_data[2].duration0, rmt.rcv_data[2].duration1, \
            rmt.rcv_data[3].duration0, rmt.rcv_data[3].duration1, rmt.rcv_data[4].duration0, rmt.rcv_data[4].duration1, rmt.rcv_data[5].duration0, rmt.rcv_data[5].duration1, \
            rmt.rcv_data[6].duration0, rmt.rcv_data[6].duration1, rmt.rcv_data[7].duration0, rmt.rcv_data[7].duration1, rmt.rcv_data[8].duration0, rmt.rcv_data[8].duration1, \
            rmt.rcv_data[9].duration0, rmt.rcv_data[9].duration1, rmt.rcv_data[10].duration0, rmt.rcv_data[10].duration1, rmt.rcv_data[11].duration0, rmt.rcv_data[11].duration1, \
            rmt.rcv_data[12].duration0, rmt.rcv_data[12].duration1, rmt.rcv_data[13].duration0, rmt.rcv_data[13].duration1, rmt.rcv_data[14].duration0, rmt.rcv_data[14].duration1, \
            rmt.rcv_data[15].duration0, rmt.rcv_data[15].duration1, rmt.rcv_data[16].duration0, rmt.rcv_data[16].duration1, rmt.rcv_data[17].duration0, rmt.rcv_data[17].duration1, \
            rmt.rcv_data[18].duration0, rmt.rcv_data[18].duration1, rmt.rcv_data[19].duration0, rmt.rcv_data[19].duration1, rmt.rcv_data[20].duration0, rmt.rcv_data[20].duration1, \
            rmt.rcv_data[21].duration0, rmt.rcv_data[21].duration1, rmt.rcv_data[22].duration0, rmt.rcv_data[22].duration1, rmt.rcv_data[23].duration0, rmt.rcv_data[23].duration1); \
    xQueueSend(xQSpooler, &cspool, NULL);
  }

  if (xQueueReceive(xQrcs_rcvd, &RCS_rcvd, 0)) {
    sprintf(cspool, "RC-Switch(c) Code rcvd : % d, protocol : % d, bit length : % d \r\n" \
            "SYN : % d \r\n", \
            RCS_rcvd.Value, RCS_rcvd.Protocol, RCS_rcvd.Bitlength, \
            RCS_rcvd.rawData[0]);
    xQueueSend(xQSpooler, &cspool, NULL);

    sprintf(cspool, "RC-Switch(c) Raw data : \r\n " \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n" \
            "{%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} , {%d , %d} \r\n", \
            RCS_rcvd.rawData[1], RCS_rcvd.rawData[2], RCS_rcvd.rawData[3], RCS_rcvd.rawData[4], RCS_rcvd.rawData[5], RCS_rcvd.rawData[6], \
            RCS_rcvd.rawData[7], RCS_rcvd.rawData[8], RCS_rcvd.rawData[9], RCS_rcvd.rawData[10], RCS_rcvd.rawData[11], RCS_rcvd.rawData[12], \
            RCS_rcvd.rawData[13], RCS_rcvd.rawData[14], RCS_rcvd.rawData[15], RCS_rcvd.rawData[16], RCS_rcvd.rawData[17], RCS_rcvd.rawData[18], \
            RCS_rcvd.rawData[19], RCS_rcvd.rawData[20], RCS_rcvd.rawData[21], RCS_rcvd.rawData[22], RCS_rcvd.rawData[23], RCS_rcvd.rawData[24], \
            RCS_rcvd.rawData[25], RCS_rcvd.rawData[26], RCS_rcvd.rawData[27], RCS_rcvd.rawData[28], RCS_rcvd.rawData[29], RCS_rcvd.rawData[30], \
            RCS_rcvd.rawData[31], RCS_rcvd.rawData[32], RCS_rcvd.rawData[33], RCS_rcvd.rawData[34], RCS_rcvd.rawData[35], RCS_rcvd.rawData[36], \
            RCS_rcvd.rawData[37], RCS_rcvd.rawData[38], RCS_rcvd.rawData[39], RCS_rcvd.rawData[40], RCS_rcvd.rawData[41], RCS_rcvd.rawData[42], \
            RCS_rcvd.rawData[43], RCS_rcvd.rawData[44], RCS_rcvd.rawData[45], RCS_rcvd.rawData[46], RCS_rcvd.rawData[47], RCS_rcvd.rawData[48]);
    xQueueSend(xQSpooler, &cspool, NULL);
  }

}
