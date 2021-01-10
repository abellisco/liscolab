/*
RC-Switch protocol definitions                                                                      __
  {T, {TSynHigh, TSynLow} , {T0High, T0Low} , {T1High, T1Low}, start 'false' = pulse starts High   |  |______|
  example Protocol 1: { 350 , {350,10850} , {350, 1050}, {1050, 350}, starts high} (microseconds)
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true }      // protocol 6 (HT6P20B) --> not supported in this version
  };

*/

RCSwitch rcvSwitch = RCSwitch();
RCSwitch xmtSwitch = RCSwitch();

void RCSinit() {
  xmtSwitch.enableTransmit(RCSXmitPIN);

  // Optional set protocol (default is 1, will work for most outlets)
  xmtSwitch.setProtocol(1);
  // Optional set pulse length.
  xmtSwitch.setPulseLength(350);
  // Optional set number of transmission repetitions.
  xmtSwitch.setRepeatTransmit(4);

  rcvSwitch.enableReceive(digitalPinToInterrupt(RCSRcvPIN));  //
}


void $RCSrcv (void* pvParameters) {
  RCS_rcvd_t RCS_rcvd;

  for (;;) {
    if (rcvSwitch.available()) {
      RCS_rcvd.Value = rcvSwitch.getReceivedValue();
      RCS_rcvd.Bitlength = rcvSwitch.getReceivedBitlength();
      RCS_rcvd.Delay = rcvSwitch.getReceivedDelay();
      RCS_rcvd.rawData = rcvSwitch.getReceivedRawdata();
      RCS_rcvd.Protocol = rcvSwitch.getReceivedProtocol();

      rcvSwitch.resetAvailable();
      xQueueSend(xQrcs_rcvd, &RCS_rcvd, NULL);

    } else
    { vTaskDelay (10 / portTICK_PERIOD_MS);
    }
  }
}


BaseType_t prvRCSXmitCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  BaseType_t xReturn;
  esp_err_t esp_Return;
  uint32_t u32value;

  u32value = uvParamStr2u32Int (pcCommandString, (UBaseType_t) 1);
  sprintf(pcWriteBuffer, "\r\nRCS ready to xmit uint32_t: %d \r\n", u32value);

  xmtSwitch.send(u32value, 24);
  return pdFALSE;
}

BaseType_t prvRCScfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
  uint16_t uvalue1, uvalue2, uvalue3 ;

  uvalue1 = uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);
  xmtSwitch.setProtocol(uvalue1);

  uvalue2 = uvParamStr2uInt (pcCommandString, (UBaseType_t) 2);
  xmtSwitch.setPulseLength(uvalue2);

  uvalue3 = uvParamStr2uInt (pcCommandString, (UBaseType_t) 3);
  xmtSwitch.setRepeatTransmit(uvalue3);
 
  sprintf(pcWriteBuffer, "\r\nRCSwitch parameters changed: Protocol %d, Pulselength %d (microsec), Repeat n %d ", uvalue1, uvalue2, uvalue3 );
  return pdFALSE;
}
