#include "driver/ledc.h"

cmd_table_t ledc_mode_table[] = {
  {'H', (uint16_t)LEDC_HIGH_SPEED_MODE}, /*!< LEDC high speed speed_mode */
  {'L', (uint16_t)LEDC_LOW_SPEED_MODE},      /*!< LEDC low speed speed_mode */
  {NULL, NULL},
} ;

cmd_table_t ledc_intr_type_table[] = {
  {'D', (uint16_t)LEDC_INTR_DISABLE} ,    /*!< Disable LEDC interrupt */
  {'E', (uint16_t)LEDC_INTR_FADE_END},       /*!< Enable LEDC interrupt */
  {NULL, NULL},
} ;

cmd_table_t ledc_duty_direction_table[] = {
  {'D', (uint16_t)LEDC_DUTY_DIR_DECREASE},    /*!< LEDC duty decrease direction */
  {'I', (uint16_t)LEDC_DUTY_DIR_INCREASE},    /*!< LEDC duty increase direction */
  {NULL, NULL},
} ;

cmd_table_t ledc_clk_src_table[] =  {
  {'R', (uint16_t)LEDC_REF_TICK}, /*!< LEDC timer clock divided from reference tick (1Mhz) */
  {'A', (uint16_t)LEDC_APB_CLK},      /*!< LEDC timer clock divided from APB clock (80Mhz) */
  {NULL, NULL},
} ;

cmd_table_t ledc_timer_table[] = {
  {'0', (uint16_t)LEDC_TIMER_0}, /*!< LEDC timer 0 */
  {'1', (uint16_t)LEDC_TIMER_1},     /*!< LEDC timer 1 */
  {'2', (uint16_t)LEDC_TIMER_2},     /*!< LEDC timer 2 */
  {'3', (uint16_t)LEDC_TIMER_3},     /*!< LEDC timer 3 */
  {NULL, NULL},
} ;

cmd_table_t ledc_channel_table[] {
  {'0', (uint16_t)LEDC_CHANNEL_0},     /*!< LEDC channel 0 */
  {'1', (uint16_t)LEDC_CHANNEL_1},     /*!< LEDC channel 1 */
  {'2', (uint16_t)LEDC_CHANNEL_2},     /*!< LEDC channel 2 */
  {'3', (uint16_t)LEDC_CHANNEL_3},     /*!< LEDC channel 3 */
  {'4', (uint16_t)LEDC_CHANNEL_4},     /*!< LEDC channel 4 */
  {'5', (uint16_t)LEDC_CHANNEL_5},     /*!< LEDC channel 5 */
  {'6', (uint16_t)LEDC_CHANNEL_6},     /*!< LEDC channel 6 */
  {'7', (uint16_t)LEDC_CHANNEL_7},     /*!< LEDC channel 7 */
  {NULL, NULL},
} ;


cmd_table_t ledc_fade_mode_table[] {
  {'N', (uint16_t)LEDC_FADE_NO_WAIT},  /*!< LEDC fade function will return immediately */
  {'W', (uint16_t)LEDC_FADE_WAIT_DONE},    /*!< LEDC fade function will block until fading to the target duty */
  {NULL, NULL},
} ;

static BaseType_t prvledcchannelcfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvledctimercfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static BaseType_t prvledcfadeinstallCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvledcfadeuninstallCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static BaseType_t prvledcfadetimeCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvledcfadestartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static BaseType_t prvledcupdatedutyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvledcsetdutyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static BaseType_t prvledcstopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvledcpauseCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvledcresumeCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );


static const CLI_Command_Definition_t xledcchannelcfgCommand =
{
  "ledcchcfg",
  "ledcchcfg (pin)< > (speed mode)<H|L> (channel)<0 - 7> (intr_type)< D|E > (timer)<0 - 3> (duty)< ... > (hpoint)< ... > \r\n"
  "Configure LEDC channel \r\n"
  "speed_mode:  \r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "intr_type: \r\n"
  "-D, INTR_DISABLE\r\n"
  "-E, INTR_FADE_END\r\n"
  "duty: [0, (2**duty_resolution)] \r\n"
  "hpoint: max value is 0xfffff \r\n"
  "ex: ledcchcfg 2 H 0 D 0 0 0 \r\n",
  prvledcchannelcfgCommand,
  7
};

static const CLI_Command_Definition_t xledctimercfgCommand =
{
  "ledctimercfg",
  "ledctimercfg (speed mode)<H|L> (bits duty resolution)< ... > (timer)<0 - 3> (frequency Hz)< ... >  \r\n"
  "Configure LEDC timer \r\n"
  "duty resolution: the range of duty setting is [0, (2**duty_resolution)] \r\n"
  "ex: ledctimercfg H 13 0 5000 \r\n"
  "return\r\n"
  "- ESP_OK Success  \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error  \r\n"
  "- ESP_FAIL Can not find a proper pre-divider number base on the given frequency and the current duty_resolution.\r\n",
  prvledctimercfgCommand,
  4
};

static const CLI_Command_Definition_t xledcfadeinstallCommand =
{
  "ledcfadeinstall",
  "ledcfadeinstall < (int)intr_alloc_flags > \r\n"
  "Install LEDC fade function. \r\n"
  "parameter intr_alloc_flags: ESP_INTR_FLAG_* values. See esp_intr_alloc.h for more info. \r\n"
  "ex: ledcfadeinstall 0  \r\n"
  "return \r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_STATE Fade function already installed. \r\n",
  prvledcfadeinstallCommand,
  1
};

static const CLI_Command_Definition_t xledcfadeuninstallCommand =
{
  "ledcfadeuninstall",
  "ledcfadeuninstall \r\n"
  "Uninstall LEDC fade function \r\n",
  prvledcfadeuninstallCommand,
  0
};


static const CLI_Command_Definition_t xledcfadetimeCommand =
{
  "ledcfadetime",
  "ledcfadetime (speed mode)<H|L> (channel)<0 - 7> (target_duty)<..> (fade_time_ms)<..> \r\n"
  "Set LEDC fade function, with a limited time \r\n"
  "target_duty of fading.<0 - (2**duty_resolution-1)>\r\n"
  "max_fade_time_ms: maximum time of the fading (ms)\r\n"
  "ex: ledcfadetime H 0 4000 3000\r\n"
  "return\r\n"
  "- ESP_ERR_INVALID_ARG Parameter error\r\n"
  "- ESP_OK Success\r\n"
  "- ESP_ERR_INVALID_STATE Fade function not installed. \r\n"
  "- ESP_FAIL Fade function init error \r\n",
  prvledcfadetimeCommand,
  4
};


static const CLI_Command_Definition_t xledcfadestartCommand =
{
  "ledcfadestart",
  "ledcfadestart (speed mode)<H|L> (channel)<0 - 7> (fade_mode) < N | W > \r\n"
  "Start LEDC fading.\r\n"
  "speed_mode:\r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "fade_mode\r\n"
  "-N, FADE_NO_WAIT\r\n"
  "-W, FADE_WAIT_DONE will block until fading to the target duty\r\n"
  "ex: ledcfadestart H 0 N  \r\n"
  "return  \r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_STATE Fade function not installed. \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error \r\n",
  prvledcfadestartCommand,
  3
};

static const CLI_Command_Definition_t xledcupdatedutyCommand =
{
  "ledcupdateduty",
  "ledcupdateduty (speed mode)<H|L> (channel)<0 - 7>\r\n"
  "LEDC update channel parameters.\r\n"
  "Call this function to activate the LEDC updated parameters.\r\n"
  "speed_mode:\r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "ex: ledcupdateduty L 0  \r\n"
  "return  \r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error \r\n",
  prvledcupdatedutyCommand,
  2
};

static const CLI_Command_Definition_t xledcsetdutyCommand =
{
  "ledcsetduty",
  "ledcsetduty (speed mode)<H|L> (channel)<0 - 7> (duty)<....> \r\n"
  "LEDC set duty.\r\n"
  "only after calling ledc_update_duty will the duty update.\r\n"
  "speed_mode:\r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "ex: ledcupdateduty L 0 128 \r\n"
  "return\r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error \r\n",
  prvledcsetdutyCommand,
  3
};
static const CLI_Command_Definition_t xledcstopCommand =
{
  "ledcstop",
  "ledcstop (speed mode)<H|L> (channel)<0 - 7> (idle_level)<....> \r\n"
  "LEDC stop.\r\n"
  "Disable LEDC output, and set idle level.\r\n"
  "speed_mode:\r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "ex: ledcstop H 0 0 \r\n"
  "return  \r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error \r\n",
  prvledcstopCommand,
  3
};
static const CLI_Command_Definition_t xledcpauseCommand =
{
  "ledcpause",
  "ledcpause (speed mode)<H|L> (timer)<0 - 3>  \r\n"
  "Pause LEDC timer counter.\r\n"
  "speed_mode:  \r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "ex: ledcpause L 0  \r\n"
  "return\r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error \r\n",
  prvledcpauseCommand,
  2
};

static const CLI_Command_Definition_t xledcresumeCommand =
{
  "ledcresume",
  "ledcresume (speed mode)<H|L> (timer)<0 - 3>  \r\n"
  "Resume LEDC timer counter.\r\n"
  "speed_mode:  \r\n"
  "-H, HIGH_SPEED_MODE \r\n"
  "-L, LOW_SPEED_MODE \r\n"
  "ex: ledcresume L 0  \r\n"
  "return  \r\n"
  "- ESP_OK Success \r\n"
  "- ESP_ERR_INVALID_ARG Parameter error \r\n",
  prvledcresumeCommand,
  2
};

/*-----------------------------------------------*/
static BaseType_t prvledcchannelcfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;

  ledc_channel_config_t channel_conf;

  channel_conf.gpio_num = (int)ivParamStr2signedInt (pcCommandString, (UBaseType_t) 1);
  channel_conf.speed_mode = (ledc_mode_t)uvLookupTable (2 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  channel_conf.channel = (ledc_channel_t)uvLookupTable (3 , pcCommandString, &ledc_channel_table[0], &esp_Return);
  channel_conf.intr_type = (ledc_intr_type_t)uvLookupTable (4 , pcCommandString, &ledc_intr_type_table[0], &esp_Return);
  channel_conf.timer_sel = (ledc_timer_t)uvParamStr2uInt (pcCommandString, (UBaseType_t) 5);  
  channel_conf.duty = (uint32_t)uvParamStr2u32Int (pcCommandString, (UBaseType_t) 6);
  channel_conf.hpoint = (int)ivParamStr2signedInt (pcCommandString, (UBaseType_t) 7);

  esp_Return = ledc_channel_config(&channel_conf);
  sprintf(pcWriteBuffer, "\r\n %s \r\n ledc_channel_config(gpio %d, speed_mode %d, channel %d, int_type %d,timer %d, duty %d, hpoint %d )", \
          esp_err_to_name(esp_Return), channel_conf.gpio_num, channel_conf.speed_mode, channel_conf.channel, channel_conf.intr_type, \
          channel_conf.timer_sel, channel_conf.duty, channel_conf.hpoint );
  return pdFALSE;
}


static BaseType_t prvledctimercfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;

  ledc_timer_config_t timer_conf;

  timer_conf.speed_mode = (ledc_mode_t)uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  timer_conf.duty_resolution = (ledc_timer_bit_t)uvParamStr2uInt (pcCommandString, (UBaseType_t) 2); //(uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  timer_conf.timer_num = (ledc_timer_t)uvParamStr2uInt (pcCommandString, (UBaseType_t) 3); //uvLookupTable (3 , pcCommandString, &ledc_timer_table[0], &esp_Return);
  timer_conf.freq_hz = (uint32_t)uvParamStr2u32Int (pcCommandString, (UBaseType_t) 4);

  esp_Return = ledc_timer_config(&timer_conf);
  sprintf(pcWriteBuffer, "\r\n %s \r\n ledc_timer_config(speed_mode %d,duty_resolution %d,timer_num %d,freq_hz %d)",   //,clk_cfg %d )",
          esp_err_to_name(esp_Return), timer_conf.speed_mode, timer_conf.duty_resolution, timer_conf.timer_num, \
          timer_conf.freq_hz); 
  return pdFALSE;
}


static BaseType_t prvledcfadeinstallCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  int intparam;

  intparam = (int) ivParamStr2signedInt (pcCommandString, (UBaseType_t) 1);
  esp_Return = ledc_fade_func_install(intparam);
  sprintf(pcWriteBuffer, "\r\n %s ledc_fade_func_install( %d )", esp_err_to_name(esp_Return), intparam );
  return pdFALSE;
}


static BaseType_t prvledcfadeuninstallCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  ledc_fade_func_uninstall();
  sprintf(pcWriteBuffer, "\r\n ledc_fade_func_uninstall() executed");
  return pdFALSE;
}


static BaseType_t prvledcfadetimeCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t ledmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t channel = uvLookupTable (2 , pcCommandString, &ledc_channel_table[0], &esp_Return);
  uint32_t target_duty = uvParamStr2u32Int (pcCommandString, (UBaseType_t) 3);
  int fade_time = ivParamStr2signedInt (pcCommandString, (UBaseType_t) 4);

  esp_Return = ledc_set_fade_with_time((ledc_mode_t) ledmode, (ledc_channel_t) channel, (uint32_t)target_duty, (int)fade_time);

  sprintf(pcWriteBuffer, "\r\n %s \r\n ledc_set_fade_with_time(speed_mode %d, channel %d, target_duty %d, fade_time %d)", \
          esp_err_to_name(esp_Return), ledmode, channel, target_duty, fade_time );
  return pdFALSE;
}

static BaseType_t prvledcfadestartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t speedmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t channel = uvLookupTable (2 , pcCommandString, &ledc_channel_table[0], &esp_Return);
  uint16_t fademode = uvLookupTable (3 , pcCommandString, &ledc_fade_mode_table[0], &esp_Return);

  esp_Return = ledc_fade_start((ledc_mode_t) speedmode, (ledc_channel_t) channel, (ledc_fade_mode_t) fademode);
  sprintf(pcWriteBuffer, "\r\n % s \r\n ledc_fade_start(speed_mode % d, channel % d, fade_mode % d)", \
          esp_err_to_name(esp_Return), speedmode, channel, fademode );
  return pdFALSE;
}


static BaseType_t prvledcupdatedutyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t speedmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t channel = uvLookupTable (2 , pcCommandString, &ledc_channel_table[0], &esp_Return);

  esp_Return = ledc_update_duty((ledc_mode_t) speedmode, (ledc_channel_t) channel);
  sprintf(pcWriteBuffer, "\r\n % s \r\n ledc_update_duty(speed_mode % d, channel % d )", \
          esp_err_to_name(esp_Return), speedmode, channel );
  return pdFALSE;
}


static BaseType_t prvledcsetdutyCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t speedmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t channel = uvLookupTable (2 , pcCommandString, &ledc_channel_table[0], &esp_Return);
  uint32_t duty = uvParamStr2u32Int (pcCommandString, (UBaseType_t) 3);
  
  esp_Return = ledc_set_duty((ledc_mode_t) speedmode, (ledc_channel_t) channel, (uint32_t) duty);
  sprintf(pcWriteBuffer, "\r\n % s \r\n ledc_set_duty(speed_mode % d, channel % d, duty % d )", \
          esp_err_to_name(esp_Return), speedmode, channel, duty );
  return pdFALSE;
}


static BaseType_t prvledcstopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t speedmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t channel = uvLookupTable (2 , pcCommandString, &ledc_channel_table[0], &esp_Return);
  uint32_t level = uvParamStr2u32Int (pcCommandString, (UBaseType_t) 3);
  
  esp_Return = ledc_stop((ledc_mode_t) speedmode, (ledc_channel_t) channel, (uint32_t) level);
  sprintf(pcWriteBuffer, "\r\n % s \r\n ledc_stop(speed_mode % d, channel % d, level % d )", \
          esp_err_to_name(esp_Return), speedmode, channel, level );
  return pdFALSE;
}

static BaseType_t prvledcpauseCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t speedmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t timer = (ledc_timer_t)uvParamStr2uInt (pcCommandString, (UBaseType_t) 2);
  
  esp_Return = ledc_timer_pause((ledc_mode_t) speedmode, (uint32_t)timer );
  sprintf(pcWriteBuffer, "\r\n % s \r\n ledc_timer_pause(speed_mode % d, timer % d )", \
          esp_err_to_name(esp_Return), speedmode, timer );
  return pdFALSE;
}


static BaseType_t prvledcresumeCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;
  uint16_t speedmode = uvLookupTable (1 , pcCommandString, &ledc_mode_table[0], &esp_Return);
  uint16_t timer = (ledc_timer_t)uvParamStr2uInt (pcCommandString, (UBaseType_t) 2);
  
  esp_Return = ledc_timer_resume((ledc_mode_t) speedmode, (uint32_t)timer );
  sprintf(pcWriteBuffer, "\r\n % s \r\n ledc_timer_pause(speed_mode % d, timer % d )", \
          esp_err_to_name(esp_Return), speedmode, timer );
  return pdFALSE;
}

void registerledfadeCommands() {
  esp_err_t esp_Return, xerrorReturn ;

  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcchannelcfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcchannelcfg, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledctimercfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledctimercfg, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcfadeinstallCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcfadeinstall, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcfadeuninstallCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcfadeuninstall, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcfadetimeCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcfadetime, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcfadestartCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcfadestart,\n\r");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcupdatedutyCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcupdateduty, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcsetdutyCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcsetduty, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcstopCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcsetduty, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcpauseCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcpause, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xledcresumeCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "ledcresume\n\r");
  }
}
