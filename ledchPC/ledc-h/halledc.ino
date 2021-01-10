
static BaseType_t prvhalledcCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvgetdutyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static const CLI_Command_Definition_t xhalledcCommand =
{
  "halledc",
  "halledc < S(etup)| W(rite)| T(one)| R(eadFreq)| A(ttachPin) | D(etachPin) > <additional parameter(s)for each command> \r\n"
  "hal-ledc functions to generate PWM signals\r\n"
  " S(etup): (channel)<'0-7'> (freq)<double> (resolution_bits)<uint8_t> \r\n"
  " W(rite): (channel)<'0-7'> (duty)<uint32_t > \r\n"
  " T(one): (channel)<'0-7'> (freq)<double> \r\n"
  " R(ead): (channel)<'0-7'> \r\n"
  " A(ttachPin): (channel)<'0-7'> (PIN)<uint8_t> \r\n"
  " D(etachPin): (PIN)<uint8_t> \r\n"
  " example:  \r\n"
  " halledc S 1 4186 8 \r\n",
  prvhalledcCommand,
  -1
};

static const CLI_Command_Definition_t xgetpulseinCommand =
{
  "getpulsein",
  "getpulsein (PIN)<uint8_t> \r\n"
  "Arduino PulseIn function, returns HIGH and LOW timings, estimated frequency and duty cycle\r\n"
  "a continuous pulse stream is expected\r\n"
  " ex: getpulsein 14 \r\n",
  prvgetpulseinCommand,
  1
};

/*-----------------------------------------*/

static BaseType_t prvhalledcCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  BaseType_t xReturn;
  esp_err_t esp_Return;
  uint8_t uChannel, uPin;
  double vdouble;
  uint32_t value32;

  BaseType_t pxParamStringLength ;
  const char *pcgralptr;

  char ccommand[3];

  pcgralptr = FreeRTOS_CLIGetParameter( pcCommandString, (UBaseType_t) 1, &pxParamStringLength );
  strncpy (ccommand, pcgralptr, 1);
  ccommand[1] = NULL;

  uChannel = (uint8_t) uvParamStr2uInt (pcCommandString, (UBaseType_t) 2);

  if (ccommand[0] == 'S')  {
    vdouble = ledcSetup(uChannel, ( uvParamStr2double (pcCommandString, (UBaseType_t) 3)), \
                        ((uint8_t) uvParamStr2uInt (pcCommandString, (UBaseType_t) 4)));
    sprintf(pcWriteBuffer, "\r\n Channel %d frequency set to %f Hz \r\n", uChannel,  vdouble );
    return pdFALSE;
  } else if (ccommand[0] == 'W') {
    value32 = (uint32_t) uvParamStr2u32Int (pcCommandString, (UBaseType_t) 3);
    ledcWrite((uint8_t) uChannel, value32 );
    sprintf(pcWriteBuffer, "\r\n Channel %d duty cycle changed to %d  \r\n", uChannel,  value32 );
    return pdFALSE;
  } else if (ccommand[0] == 'T') {
    vdouble = uvParamStr2double (pcCommandString, (UBaseType_t) 3);
    ledcWriteTone((uint8_t) uChannel, vdouble);
    sprintf(pcWriteBuffer, "\r\n Channel %d tone with frequency = %f Hz \r\n", uChannel,  vdouble );
    return pdFALSE;
  } else if (ccommand[0] == 'R') {
    vdouble = ledcReadFreq((uint8_t) uChannel);
    sprintf(pcWriteBuffer, "\r\n Channel %d, frequency = %f Hz \r\n", uChannel,  vdouble );
    return pdFALSE;
  } else if (ccommand[0] == 'A') {
    uint8_t pin = (uint8_t) uvParamStr2uInt (pcCommandString, (UBaseType_t) 3);
    ledcAttachPin((uint8_t) pin, (uint8_t) uChannel);
    sprintf(pcWriteBuffer, "\r\n PIN %d attached to Channel %d \r\n", pin , uChannel );
    return pdFALSE;
  } else if (ccommand[0] == 'D') {
    uint8_t pin = (uint8_t) uvParamStr2uInt (pcCommandString, (UBaseType_t) 2);
    ledcDetachPin((uint8_t) pin);
    sprintf(pcWriteBuffer, "\r\n PIN %d detTached \r\n", pin );
    return pdFALSE;
  } else
  { sprintf(pcWriteBuffer, "\r\n Wrong parameter #1 %c  \r\n", ccommand[0] );
    return pdFALSE;
  };
};

static BaseType_t prvgetpulseinCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  BaseType_t xReturn;
  esp_err_t esp_Return;
  int uPin;
  double vdfreq, vdT , vdduty;

  unsigned long THigh, TLow, timeout = 100000;

  uPin = (int) uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);
  THigh = pulseIn(uPin, HIGH, timeout);
  TLow = pulseIn(uPin, LOW, timeout);
  if ((THigh != 0) && (TLow != 0))
  {
    vdT = double(THigh + TLow) ;
    vdduty = THigh / vdT ;
    vdfreq = 1000000 / vdT ;
  } else
  { vdT = 0 ;
    vdfreq = 0;
  }

  sprintf(pcWriteBuffer, "\r\nMeasured PulseIn (microsec): THigh %d  TLow %d T %f \r\n frequency: %f(Hz)  duty %f  ", THigh, TLow, vdT, vdfreq, vdduty );
  return pdFALSE;
}

void registerhalledcCommands() {
  esp_err_t esp_Return, xerrorReturn ;

  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xhalledcCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "halledc, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xgetpulseinCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "getpulsein\n\r");
  }

}
