#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"

#define GPIO_IS_VALID_GPIO(gpio_num)              ((gpio_num < GPIO_PIN_COUNT && GPIO_PIN_MUX_REG[gpio_num] != 0))
#define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num)       ((GPIO_IS_VALID_GPIO(gpio_num)) && (gpio_num < 34))


const cmd_table_t gpio_set_table[] = {
  {'0', (uint16_t)0}, /*!< logic 0 */
  {'1', (uint16_t)1},  /*!< logic 1  */
  {NULL, NULL},
};
const cmd_table_t gpio_intr_table[] = {
  {'D', (uint16_t)GPIO_INTR_DISABLE},   /*!< Disable GPIO interrupt                             */
  {'P', (uint16_t)GPIO_INTR_POSEDGE},   /*!< GPIO interrupt type : rising edge                  */
  {'N', (uint16_t)GPIO_INTR_NEGEDGE},   /*!< GPIO interrupt type : falling edge                 */
  {'A', (uint16_t)GPIO_INTR_ANYEDGE},   /*!< GPIO interrupt type : both rising and falling edge */
  {'L', (uint16_t)GPIO_INTR_LOW_LEVEL}, /*!< GPIO interrupt type : input low level trigger      */
  {'H', (uint16_t)GPIO_INTR_HIGH_LEVEL}, /*!< GPIO interrupt type : input high level trigger     */
  {NULL, NULL},
};

const cmd_table_t gpio_mode_table[] = {
  {'D', (uint16_t)GPIO_MODE_DISABLE},                         /*!< GPIO mode : disable input and output             */
  {'I', (uint16_t)GPIO_MODE_INPUT},                           /*!< GPIO mode : input only                           */
  {'O', (uint16_t)GPIO_MODE_OUTPUT},                          /*!< GPIO mode : output only mode                     */
  {'U', (uint16_t)GPIO_MODE_OUTPUT_OD},                       /*!< GPIO mode : output only with open-drain mode     */
  {'X', (uint16_t)GPIO_MODE_INPUT_OUTPUT_OD},                 /*!< GPIO mode : output and input with open-drain mode*/
  {'B', (uint16_t)GPIO_MODE_INPUT_OUTPUT},                    /*!< GPIO mode : output and input mode                */
  {NULL, NULL},
};

const cmd_table_t gpio_pullup_table[] = {
  {'D', (uint16_t)GPIO_PULLUP_DISABLE},   /*!< Disable GPIO pull-up resistor */
  {'E', (uint16_t)GPIO_PULLUP_ENABLE},    /*!< Enable GPIO pull-up resistor */
  {NULL, NULL},
};

const cmd_table_t gpio_pulldown_table[] = {
  {'D', (uint16_t)GPIO_PULLDOWN_DISABLE}, /*!< Disable GPIO pull-down resistor */
  {'E', (uint16_t)GPIO_PULLDOWN_ENABLE},  /*!< Enable GPIO pull-down resistor  */
  {NULL, NULL},
};

static BaseType_t prvGPIOcfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvGPIOsetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvGPIOintenCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvGPIOintdisCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

static const CLI_Command_Definition_t xgpiocfgCommand =
{
  "gpiocfg",
  "gpiocfg (pin#)<..> (Mode)< D|I|O|U|X|B > (Pullup)< D|E > (Pulldown)< D|E >  (Interrupts) < D|P|N|A|L|H > \r\n"
  "configure a GPIO PIN\r\n"
  " Mode:  \r\n"
  "  D, DISABLE: disable input and output \r\n"
  "  I, INPUT: input only \r\n"
  "  O, OUTPUT: output only \r\n"
  "  U, OUTPUT_OD: output only with open-drain \r\n"
  "  X, INPUT_OUTPUT_OD: input/output open-drain \r\n"
  "  B, INPUT_OUTPUT: input/output \r\n"
  " Interrupt type:  \r\n"
  "  D, DISABLE\r\n"
  "  P, POSEDGE: rising edge \r\n"
  "  N, NEGEDGE: falling edge \r\n"
  "  A, ANYEDGE: rising and falling edge \r\n"
  "  L, LOW_LEVEL: input low level\r\n"
  "  H, HIGH_LEVEL: input high level\r\n"
  "ex: gpiocfg 14 O D D D \r\n",
  prvGPIOcfgCommand,
  5
};

static const CLI_Command_Definition_t xgpiosetCommand =
{
  "gpioset",
  "gpioset (pin#)<..> (level) < 0|1 > \r\n"
  "set PIN level ( 0 | 1 ) \r\n"
  "ex: gpioset 14 1 \r\n",
  prvGPIOsetCommand,
  2
};

static const CLI_Command_Definition_t xGPIOintenCommand =
{
  "gpiointen",
  "gpiointen (pin#)<..>  \r\n"
  "GPIO Interrupt Enable\r\n"
  "example: gpiointen 14 \r\n",
  prvGPIOintenCommand,
  1
};
static const CLI_Command_Definition_t xGPIOintdisCommand =
{
  "gpiointdis",
  "gpiointdis (pin#)<..> \r\n"
  "GPIO Interrupt Disable\r\n"
  "example: gpiointdis 14 \r\n",
  prvGPIOintdisCommand,
  1
};

/*------------------------------------------------*/

static BaseType_t prvGPIOcfgCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
  esp_err_t esp_Return;

  uint16_t upin_num, uvalue;
  uint64_t ulpinbitmask;
  int16_t ivalue;

  /* common parameters */
#define Paramgpiopin   1
  /* config parameters */
#define Paramgpiomode   2
#define ParamgpioPup    3
#define ParamgpioPdn    4
#define ParamgpioInt    5
  /* set parameter */

#define ParamgpioIntmode 3

  /* sorry, unimplemented: non-trivial designated initializers not supported */

  gpio_config_t gpio_cfg;

  upin_num = uvParamStr2uInt (pcCommandString, (UBaseType_t) Paramgpiopin);
  ulpinbitmask = (1ULL << upin_num);

  gpio_cfg.pin_bit_mask = ulpinbitmask;
  gpio_cfg.mode = (gpio_mode_t)uvLookupTable (Paramgpiomode, pcCommandString, &gpio_mode_table[0], &esp_Return);
  gpio_cfg.pull_up_en = (gpio_pullup_t)uvLookupTable (ParamgpioPup, pcCommandString, &gpio_pullup_table[0], &esp_Return);
  gpio_cfg.pull_down_en = (gpio_pulldown_t)uvLookupTable (ParamgpioPdn, pcCommandString, &gpio_pulldown_table[0], &esp_Return);
  gpio_cfg.intr_type = (gpio_int_type_t)uvLookupTable (ParamgpioInt, pcCommandString, &gpio_intr_table[0], &esp_Return);

  esp_Return = gpio_config(&gpio_cfg);
  sprintf(pcWriteBuffer, "\r\n %s gpio_config(pin %d,Mode %d,Pup %d,Pdn %d,Int %d )", esp_err_to_name(esp_Return), upin_num, gpio_cfg.mode, gpio_cfg.pull_up_en, \
          gpio_cfg.pull_down_en, gpio_cfg.intr_type);
  return pdFALSE;
}// end cfg command

static BaseType_t prvGPIOsetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
#define Paramgpiopin   1
#define Numbergpiosetl   2

  esp_err_t esp_Return;

  uint16_t upin_num, uvalue;
  int16_t ivalue;

  upin_num = uvParamStr2uInt (pcCommandString, (UBaseType_t) Paramgpiopin);

  uvalue = uvLookupTable (Numbergpiosetl, pcCommandString, &gpio_set_table[0], &esp_Return);

  esp_Return = gpio_set_level((gpio_num_t)upin_num, uvalue);
  sprintf(pcWriteBuffer , "\r\n %s : gpio_set_level( pin %d, set %d )", esp_err_to_name(esp_Return), upin_num, uvalue );
  return pdFALSE;
}// end set


static BaseType_t prvGPIOintenCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
#define Paramgpiopin   1
  BaseType_t xReturn, pxParamStringLength ;
  esp_err_t esp_Return;

  const char *pcgralptr, *pcIntParam;
  char cgralbuffer[7];
  uint16_t upin_num, uvalue;
  uint64_t ulpinbitmask;
  int16_t ivalue;
  upin_num = uvParamStr2uInt (pcCommandString, (UBaseType_t) Paramgpiopin);

  esp_Return = gpio_intr_enable((gpio_num_t) upin_num);
  sprintf(pcWriteBuffer , "\r\n %s gpio_int_enable(pin %d)", esp_err_to_name(esp_Return), upin_num );
    return pdFALSE;
} // end Int enable

static BaseType_t prvGPIOintdisCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
#define Paramgpiopin   1
  BaseType_t xReturn, pxParamStringLength ;
  esp_err_t esp_Return;

  const char *pcgralptr, *pcIntParam;
  char cgralbuffer[7];
  uint16_t upin_num, uvalue;
  uint64_t ulpinbitmask;
  int16_t ivalue;
  upin_num = uvParamStr2uInt (pcCommandString, (UBaseType_t) Paramgpiopin);
  esp_Return = gpio_intr_disable((gpio_num_t) upin_num);
  sprintf(pcWriteBuffer , "\r\n %s gpio_int_disable(pin %d)", esp_err_to_name(esp_Return), upin_num );
    return pdFALSE;
}// end Interrup disable

void registergpioCommands() {
  esp_err_t esp_Return, xerrorReturn ;

  
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xgpiocfgCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "gpiocfg, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xgpiosetCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "gpioset, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xGPIOintdisCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "gpiointdis, ");
  }
  xerrorReturn = FreeRTOS_CLIRegisterCommand( &xGPIOintenCommand );
  if (xerrorReturn == pdPASS) {
    strcat(cOutBuffer, "gpiointen\n\r");
  }
}
