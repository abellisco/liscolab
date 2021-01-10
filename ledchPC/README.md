## ledc-h (Protoboard "companion")

ledc-h is an interactive Arduino sketch, built using FreeRTOS+CLI(c) and ESP-IDF(c) APIs.

It allows interactive access to the following APIs:
- ESP-IDF: ledc, GPIO 
- hal-ledc (hardware-adaptation-layer, used in the Arduino environment)

Note: not all ledc.h APIs are available, but the available ones allow the reproduction of the peripherals/ledc_example.c ESP-IDF example.

Please read the Installation and User Guide.
