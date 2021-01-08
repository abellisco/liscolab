rmt_obj_t* rmt_rcv = NULL;


void $RMTRx (void* pvParameters)
{
  pulseRcvd_t pulseMsg;
  bool xReturn;

  if ((rmt_rcv = rmtInit(RMTRcvPIN, false, RMT_MEM_64)) == NULL)
  {
    Serial.println("init RMT Rcv failed\n ");
  }

  float rxTick = rmtSetTick(rmt_rcv, 1000);

  xReturn = rmtSetFilter(rmt_rcv, true , 240);
  /*     if (!rmt || filter_level > 0xFF) {
          return false;
  */
  if (xReturn != true)
  {
    Serial.println("RMT filter not set");
  }

  // Start receiving

  for (;;)
  {
    rmtSetRxThreshold(rmt_rcv, ( timings[0].TSynLow >> 1) );
    pulseMsg.low = pulseInLong(RMTRcvPIN, LOW );
    if ((pulseMsg.low > (timings[0].TSynLow - Delta)) && (pulseMsg.low < (timings[0].TSynLow + Delta)))
    { pulseMsg.symbol = 'S';
      xQueueSend(xQcode, &pulseMsg, NULL);
      rmtRead(rmt_rcv, rmtrcvcb );
    }
    vTaskDelay (10 / portTICK_PERIOD_MS);
  }
}


BaseType_t prvDeltacfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;
  uint16_t uvalue;

  Delta = uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);

  sprintf(pcWriteBuffer, "\r\nDelta tolerance changed: %d \r\n", Delta);

  return pdFALSE;
}

BaseType_t prvDeltashowCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;

  sprintf(pcWriteBuffer, "\r\nCurrent Delta tolerance: %d \r\n", Delta);
  return pdFALSE;
}
