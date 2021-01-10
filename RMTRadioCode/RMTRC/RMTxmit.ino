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

  sprintf(pcWriteBuffer, "\r\nRMT timings[0] changed, TSynH %d, TSynH %d,T0H %d,T0L %d,T1H %d,T1L %d,(microsec)\r\n", \
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


BaseType_t prvDeltacfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;
  uint16_t uvalue;

  Delta = uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);

  sprintf(pcWriteBuffer, "\r\nDelta tolerance changed: %d (microsec)\r\n", Delta);

  return pdFALSE;
}

BaseType_t prvDeltashowCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;

  sprintf(pcWriteBuffer, "\r\nCurrent Delta tolerance: %d (microsec) \r\n", Delta);
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
