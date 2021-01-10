

static BaseType_t prvHWMCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvtaskdelayCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static const CLI_Command_Definition_t xHWMCommand =
{
  "hwmark",
  " hwmark\r\n"
  " returns the minimum amount of remaining stack space that was available to the tasks since the tasks \r\n"
  " started executing – that is the amount of stack that remained unused when the task stack was at its greatest (deepest) value. \r\n"
  " This is what is referred to as the stack ‘high water mark’.\r\n"
  " example: hwmark \r\n",
  prvHWMCommand,
  0
};

static BaseType_t prvHWMCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  UBaseType_t uxHWM0, uxHWM1, uxHWM2;

  uxHWM0 = uxTaskGetStackHighWaterMark( xHandleSpool );
  uxHWM1 = uxTaskGetStackHighWaterMark( xHandleSerial );

  sprintf(pcWriteBuffer , "\r\n unused stack when the task stack was at its greatest (deepest) value \r\n$Spool: %d  \r\n$Serial: %d  ", \
          (uint16_t) uxHWM0, (uint16_t) uxHWM1 );
  return pdFALSE;

}

static const CLI_Command_Definition_t xtaskdelayCommand =
{
  "taskdelay",
  " taskdelay (delay_ms)<..> \r\n"
  " executes RTOS vTaskDelay function for the specified time interval in microsec. \r\n"
  " example: taskdelay 1000 \r\n",
  prvtaskdelayCommand,
  0
};

static BaseType_t prvtaskdelayCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  TickType_t xDelay;
  unsigned long time1, time2;

  time1 = micros();
  uint16_t param = uvParamStr2uInt (pcCommandString, (UBaseType_t) 1);
  xDelay = param / portTICK_PERIOD_MS ;
  vTaskDelay( xDelay );
  time2 = micros();
  sprintf(pcWriteBuffer , "\r\n elapsed time: %d microsec" ,  (time2 - time1)  );
  return pdFALSE;

}// end


void registerrtosCommands() {
  esp_err_t esp_Return, xerrorReturn ;

  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xHWMCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "hwmark, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xtaskdelayCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "taskdelay \n\r");
  }
  prvOut();
}
