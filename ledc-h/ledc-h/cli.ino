
void registergpioCommands();
void registerhalledcCommands();
void registerledfadeCommands();
void registerrtosCommands();

void registerCommands() {
  esp_err_t esp_Return, xerrorReturn ;

  strcat(cOutBuffer, "Commands registered:\r\nhelp \r\n");

  registergpioCommands();
  registerhalledcCommands();
  registerledfadeCommands();
  registerrtosCommands();
  prvOut();

}
