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


/* PIN assignements */


#define led 2            /*on board led */

/* added  for FreeRTOS+CLI use */
#define CLI_OUT_SIZE  550
#define SPOOL_SIZE    600
#define LINE_BUF_SIZE 280

#ifndef configAPPLICATION_PROVIDES_cOutputBuffer
#define configAPPLICATION_PROVIDES_cOutputBuffer 0
#endif

BaseType_t errorReturn  = pdFALSE;
static char *cOutBuffer ,  cspool[SPOOL_SIZE];
static char vcharline[LINE_BUF_SIZE], cline[LINE_BUF_SIZE], csend[LINE_BUF_SIZE] ;
/*-----------------*/

struct cmd_table_t {
  char cmd_token;
  uint16_t ivalue;
};

struct table_str {
  char idxname[30];
  uint16_t idxvalue;
};

/*-------------lookup -----------------*/
BaseType_t exactfind(const table_str * table, const char * str2, esp_err_t *OpResult );

uint8_t usParamlookup (char pccmd, const struct cmd_table_t *ptr, bool * lookupResult );
uint16_t uvLookupTable (uint16_t ParamNbr, const char *pcCommand, const cmd_table_t *ParamList, esp_err_t *OpResult);
uint16_t uvParamStr2uInt (const char *pcCommandString, UBaseType_t IntParam);
uint16_t ivParamStr2signedInt (const char *pcCommandString, UBaseType_t signedIntParam);
uint32_t uvParamStr2u32Int (const char *pcCommandString, UBaseType_t IntParam);
double uvParamStr2double (const char *pcCommandString, UBaseType_t IntParam);
/*--------------------------------------*/

void prvOut();
void registerCommands();
void setupgpio();


/*--------- Queues -------------------*/

static xQueueHandle xQSerialIn = NULL,  xQSpooler = NULL ;

void createQueues() {

  strcat(cOutBuffer, "\r\nQueues created: ");

  xQSerialIn = xQueueCreate(10, sizeof( vcharline ) ); 
  if (xQSerialIn != 0) {
    strcat(cOutBuffer, "xQSerialIn, ");
  };
  xQSpooler = xQueueCreate(20, sizeof(cspool));
  if (xQSpooler != 0) {
    strcat(cOutBuffer, "xQSpooler \r\n");
  };
  prvOut();
}

/*-------------- Tasks -----------*/
void $Serial ( void* );
void $Spool (void*);

TaskHandle_t xHandleSerial = NULL, xHandleSpool = NULL ;

void createTasks() {
  BaseType_t xReturned;

  strcat(cOutBuffer, "\r\nTasks created: \r\n");

  xReturned = xTaskCreate($Spool, "$Spool", 2048 , NULL, 10, &xHandleSpool);
  if ( xReturned == pdPASS ) {
    strcat(cOutBuffer, "$Spool, ");
  };
  xReturned = xTaskCreate($Serial, "$Serial", 2048, NULL, 10, &xHandleSerial);
  if ( xReturned == pdPASS ) {
    strcat(cOutBuffer, "$Serial, ");
  };
  prvOut();
}


/*---  Arduino main    */

void setup() {
  BaseType_t xerrorReturn;

  Serial.begin(115200);               /* check baud rate of Serial Monitor coonsole */
  cOutBuffer = FreeRTOS_CLIGetOutputBuffer();
  createQueues();
  createTasks();
  setupgpio();
  registerCommands();

  Serial.println ("\r\n ledc-h (ESP32 ledc 'protoboard companion')\r\n based on FreeRTOS+CLI(c) and esp-idf(c) API's \r\n");
}


void loop()
{
  char rcvline[LINE_BUF_SIZE];

  if (xQueueReceive(xQSerialIn, rcvline, (100 / portTICK_PERIOD_MS)))
  {
    errorReturn = pdFALSE;
    errorReturn = FreeRTOS_CLIProcessCommand( rcvline, cOutBuffer, (size_t) CLI_OUT_SIZE  );
    strcpy ( cspool, cOutBuffer);
    memset ( cOutBuffer, NULL, configCOMMAND_INT_MAX_OUTPUT_SIZE);
    xQueueSend(xQSpooler, cspool, (10 / portTICK_PERIOD_MS));

    while (errorReturn == pdTRUE)
    {
      errorReturn = FreeRTOS_CLIProcessCommand( rcvline, cOutBuffer, (size_t) CLI_OUT_SIZE  );
      strcpy ( cspool, cOutBuffer);
      memset ( cOutBuffer, NULL, configCOMMAND_INT_MAX_OUTPUT_SIZE);
      xQueueSend(xQSpooler, cspool, (10 / portTICK_PERIOD_MS));
    };
  }
}
