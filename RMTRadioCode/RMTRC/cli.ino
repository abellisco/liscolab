

void registerCommands() {
  BaseType_t xReturn, pxParamLength ;
  esp_err_t esp_Return, xerrorReturn ;

  strcat(cOutBuffer, "Commands registered:\r\nhelp,  ");
  
  registerRCSCommands();
  
  registerRMTCommands();
  
  prvOut();
}
