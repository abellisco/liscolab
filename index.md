## Welcome to LiscoLab Pages

The LiscoLab team members expect you find our contributions valuable for you.

We are currently focusing on tools to help you in "quick prototyping" with your ESP32 board using the Arduino IDE.

In our repository you may find:

### RMT Radio Code

RMTRC (RMT Radio Code) is an interactive Arduino Sketch that can be used to test and prototype with the RMT peripheral using ISM RF bands (433Mhz, 315Mhz) Radio Codes,  usually found in radio remote controls.  It also provides an interface to use the RC-Switch(c) library.

### Free-RTOS-Plus-CLI-vESP32

This is a modified version of the FreeRTOS+CLI(c) library, that runs on ESP32.  It can be included as an Arduino library, and it is the basis of the interactive Arduino sketches found in this repository.  

## ledc-h (Protoboard "companion")

ledc-h is an interactive Arduino sketch, built using FreeRTOS+CLI(c) and ESP-IDF(c) APIs.

It allows interactive access to the following APIs:

    ESP-IDF: ledc, GPIO
    hal-ledc (hardware-adaptation-layer, used in the Arduino environment)
    FreeRTOS Task Utilities: uxTaskGetStackHighWaterMark, vTaskDelay 


