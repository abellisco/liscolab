

static const CLI_Command_Definition_t xrcsxmitCommand =
{
  "rcsxmit",
  "rcsxmit <...>(24bits value, max = (2^24)-1 = 16777215)  \r\n"
  "transmit a code (a decimal value) using RC-Switch \r\n"
  "example: rcsxmit 23456 \r\n",
  prvRCSXmitCommand,
  1
};

static const CLI_Command_Definition_t xrcscfgCommand =
{
  "rcscfg",
  "rcscfg (Protocol)<1..5> (Pulselength usec)<1...400>) (Repeat)<1...16> \r\n"
  "configure RC-Switch(c) protocol for transmission \r\n"
  "example RC-Switch Protocol 1 configuration: rcscfg 1 350 4 \r\n",
  prvRCScfgCommand,
  3
};

static const CLI_Command_Definition_t xrmtxmitCommand =
{
  "rmtxmit",
  "rmtxmit <....>(24bits value, max = (2^24)-1 = 16777215) \r\n"
  " transmit a decimal value using RMT peri[pheral\r\n"
  "example: rmtxmit 23456 \r\n",
  prvRMTXmitCommand,
  1
};

static const CLI_Command_Definition_t xRMTcfgCommand =
{
  "rmtcfg",
  "rmtcfg (Protocol)<1..5> \r\n"
  "configure RMT preipheral according to RC-Switch(c) Protocols identification \r\n"
  "example RMTXmit Protocol 1 configuration: rmtcfg 1 \r\n",
  prvRMTcfgProtocol,
  1
};

static const CLI_Command_Definition_t xRMTtimingcfg =
{
  "rmttimingcfg",
  "rmttimingcfg <TSynHigh> <TSynLow> <T0High> <T0Low> <T1High> <T1Low> (microseconds)\r\n"
  "set timing for RMT transmission signals; also sets SYN pulse timing for the RMT receiver (start receiving signal)\r\n"
  "it can be used if the protocols configured with rmtcfg do not provide adequate timing configuration \r\n"
  "example: rmttiming 350 10850 350 1050 1050 350 \r\n",
  prvRMTtimingcfg,
  6
};

static const CLI_Command_Definition_t xDeltacfgCommand =
{
  "deltacfg",
  "deltacfg <..> (microseconds)\r\n"
  "configure Delta, timing tolerance allowed while decoding the received bits in the RMT peripheral\r\n"
  "example default setup for protocol 1: deltacfg 50 \r\n",
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


void registerCommands() {
  esp_err_t esp_Return, xerrorReturn ;

  strcat(cOutBuffer, "Commands registered:\r\nhelp,  ");

  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xrcsxmitCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "rcsxmit, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xrcscfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "rcscfg\r\n");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xrmtxmitCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "xrmtxmit, ");
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
    strcat(cOutBuffer, "rmtshow\r\n");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xDeltacfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "deltacfg, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xDeltashowCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "deltashow \r\n");
  }

  prvOut();
}
