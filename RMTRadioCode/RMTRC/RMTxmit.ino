/* 2^24 = 16777216 */
/*
  typedef struct {
    union {
        struct {
            uint32_t duration0 :15;
            uint32_t level0 :1;
            uint32_t duration1 :15;
            uint32_t level1 :1;
        };
        uint32_t val;
    };
  } rmt_data_t;

*/
static const CLI_Command_Definition_t xrmtxmitCommand =
{
  "rmtxmit",
  "rmtxmit <....>(24bits value, max = (2^24)-1 = 16777215) \r\n"
  "example transmit the value 23456 (decimal) using RMT: rmtxmit 23456 \r\n",
  prvRMTXmitCommand,
  1
};

static const CLI_Command_Definition_t xRMTtimingcfg =
{
  "rmttimingcfg",
  "rmttimingcfg <TSynHigh> <TSynLow> <T0High> <T0Low> <T1High> <T1Low> (microseconds)\r\n"
  "ex: rmttiming 350 10850 350 1050 1050 350 \r\n",
  prvRMTtimingcfg,
  6
};

static const CLI_Command_Definition_t xRMTcfgCommand =
{
  "rmtcfg",
  "rmtcfg (Protocol)<1..5> \r\n"
  "example RMTXmit Protocol 1 configuration: rmtcfg 1 \r\n",
  prvRMTcfgProtocol,
  1
};

static const CLI_Command_Definition_t xDeltacfgCommand =
{
  "deltacfg",
  "deltacfg <..> (microseconds)\r\n"
  "example default setup for protocol 1: deltacfg 50; (tolerance for protocol timings) \r\n",
  prvDeltacfgCommand,
  1
};

static const CLI_Command_Definition_t xDeltashowCommand =
{
  "deltashow",
  "deltashow \r\n"
  "returns current Delta (example default Protocol 1): \r\n"
  "Current Delta tolerance:50 (microseconds)\r\n",
  prvDeltashowCommand,
  0
};
static const CLI_Command_Definition_t xRMTshowCommand =
{
  "rmtshow",
  "rmtshow \r\n"
  "returns current configured protocol parameters (example default Protocol 1):\r\n"
  "T0High: 350, T0Low: 1050, T1High: 1050, T1Low: 350, TSynHigh: 350, TSynLow: 10850 (microseconds) \r\n",
  prvRMTshowCommand,
  0
};

/* RMT Xmit */
rmt_obj_t* rmt_send = NULL;

rmt_data_t my_data[150];

BaseType_t prvRMTXmitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn, pxParamLength ;
  esp_err_t esp_Return;
  uint32_t u32value, xt;

  u32value = uvParamStr2u32Int (pcCommandString, (UBaseType_t) 1);
  sprintf(pcWriteBuffer, "\r\nRMTxmitRC value to xmit: %d \r\n", u32value);

  for (uint16_t i = 0; i < 24; i++) {
    if ((u32value & (1ULL << (23 - i))) != 0) {
      my_data[i] = {{{ timings[0].T1High, 1, timings[0].T1Low, 0 }}}; // "1"
    }
    else
    {
      my_data[i] = {{{ timings[0].T0High, 1, timings[0].T0Low, 0 }}}; // "0"
    }
  }
  my_data[24] = {{{ timings[0].TSynHigh, 1, timings[0].TSynLow, 0 }}};

  for (uint16_t n = 1; n < 5; n++) {  //repeat 4 * code word
    for (uint16_t j = 0; j < 25; j++) {
      my_data[((25 * n) + j)] = my_data[j];
    }
  }

  rmtWrite(rmt_send, my_data, 100);

  return pdFALSE;
}

BaseType_t prvRMTcfgProtocol( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;
  uint16_t uvalue;

  uvalue = uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);
  if ((uvalue > 5) || (uvalue < 1)) {
    sprintf(pcWriteBuffer, "\r\nRMTXmit invalid Protocol specified: %d, ", uvalue );
    return pdFALSE;
  } else {
    timings[0].T0High = timings[uvalue].T0High;
    timings[0].T1High = timings[uvalue].T1High;
    timings[0].T0Low = timings[uvalue].T0Low;
    timings[0].T1Low = timings[uvalue].T1Low;
    timings[0].TSynHigh = timings[uvalue].TSynHigh;
    timings[0].TSynLow = timings[uvalue].TSynLow;

    sprintf(pcWriteBuffer, "\r\nRMTXmit Protocol changed, new Protocol: %d, \r\n", uvalue );
    return pdFALSE;
  }
}

BaseType_t prvRMTtimingcfg( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;
  uint16_t uvalue;

  timings[0].TSynHigh = uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);
  timings[0].TSynLow = uvParamStr2uInt (pcCommandString, (UBaseType_t) 2);
  timings[0].T0High = uvParamStr2uInt (pcCommandString, (UBaseType_t) 3);
  timings[0].T0Low = uvParamStr2uInt (pcCommandString, (UBaseType_t) 4);
  timings[0].T1High = uvParamStr2uInt (pcCommandString, (UBaseType_t) 5);
  timings[0].T1Low = uvParamStr2uInt (pcCommandString, (UBaseType_t) 6);

  sprintf(pcWriteBuffer, "\r\nRMT timings[0] changed, TSynH %d, TSynH %d,T0H %d,T0L %d,T1H %d,T1L %d,\r\n", \
          timings[0].TSynHigh, timings[0].TSynLow, timings[0].T0High, timings[0].T0Low, timings[0].T1High, timings[0].T1Low  );
  return pdFALSE;
}

BaseType_t prvRMTshowCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;

  sprintf(pcWriteBuffer, "\r\nRMTXmit current settings: \r\n" \
          "T0High: %d, T0Low: %d, T1High: %d, T1Low: %d, TSynHigh: %d, TSynLow: %d (microsec) \r\n", \
          timings[0].T0High, timings[0].T0Low, timings[0].T1High, timings[0].T1Low, timings[0].TSynHigh, timings[0].TSynLow  );
  return pdFALSE;
}

void xvinitRMT() {
  if ((rmt_send = rmtInit(RMTXmitPIN, true, RMT_MEM_64)) == NULL)
  {
    Serial.println("init sender failed\n");
  }

  float txTick = rmtSetTick(rmt_send, 1000);
  Serial.printf("RMT clock set to: %g nsec \n", txTick);
}

void registerRMTCommands() {
  BaseType_t xReturn, pxParamLength ;
  esp_err_t esp_Return, xerrorReturn ;
  
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xrmtxmitCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "rmtxmit, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xRMTcfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "rmtcfg, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xRMTtimingcfg );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "rmttimingcfg, ");
  }  
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xRMTshowCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "rmtshow, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xDeltacfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "deltacfg, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xDeltashowCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "deltashow \r\n");
  }
}
