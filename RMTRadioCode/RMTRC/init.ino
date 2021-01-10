#include "freertos/FreeRTOS.h"

void setupgpio() {
  pinMode(RMTXmitPIN, OUTPUT);
  pinMode(led, OUTPUT); /* pilot for testing purposes */

}
