# RMT Radio Code.

RMT Radio Code relies in the functionalities provided by FreeRTOS and FreeRTOS+CLI. It provides 2 independent tasks to simultaneously receive and decode 24 bits Radio Code Words. It receives commands and sends the outputs of the decoders via the Arduino Serial Monitor console. The FreeRTOS+CLI(c) library provides all the structure for the CLI. It provides a very good platform to create an extensible interface, with support for "help", parameter control and command execution. Since FreeRTOS+CLI(c) is actually not supported by Espressif's FreeRTOS, minor modifications are needed to be able to compile the library (indicated in the sources). RMT Radio Code provides 2 outputs for transmission: one based on the RMT Transmit peripheral of the ESP32 chip, and the other based on the RC-Switch(c) library. There is one command to transmit with RMT ("rmtxmit") and another one to transmit using RC-Switch(c) ("rcsxmit"). Both methods can also be configured using commands ("rcscfg" and "rmtcfg"). Use the “help” oommands for instructions on its use.

RMT Radio Code provides 2 independent inputs and methods to receive and decode the Radio Codes; each method is handled by a separate FreeRTOS Task, running simultaneously in parallel. To choose which method will be used to decode the received Radio code, the user has to connect the corresponding input pin to the Data PIN of the RF Receiver. Using a protoboard, the 2 input PINS can be connected simultaneously to the RF Receiver Data PIN, so the user can compare the behavior and the results of the 2 methods. 

The RMT receiver has all the advantages of being based in a hardware peripheral: timing capture is very precise, and the API is simple to use. The end of reception is triggered by a configurable threshold. The main design challenge for this use case is at what point the RMT is to be "fired" to start receiving the RF signal, if you start the receiver at any point, it just keeps grabbing noise. To avoid that, RMT Radio Code relies on the detection of the PulseIn Arduino function to identify when a Radio Code starts (the "SYN" code, see the data sheets of the manufacturers in he Docs folder). 

To decode a bit, the usual method is to capture the pulse timing intervals ("high" and "low"), and based on that compare with the pre defined timings ("protocols") to define if it is a "1" or a "0". Very roughly, this is the method used by the RC-Switch(c) library (read the sources, it is actually much more elaborated). You can also find an Atmel's application Note in the Docs folder explaining this type of method. 

PLEASE READ THE INSTALLATION INSTRUCTIONS in the Docs folder, both for the hardware and software. It is recommended to initially start using RMT Radio Code by cross-connecting directly one of the Transmit PINs (either RCSXmit OR RMTXmit, NOT BOTH TOGETHER!!) to the Receive PINs (the receive PINS CAN be connected together) to ensure everything is working correctly. After you are comfortable with the results, start testing using an RF transmitter and a receiver.

Previous knowledge of RC-Switch (specially about protocols and timings), FreeRTOS and FreeRTOS+CLI are expected.

Please refer to the documentation for details.

https://www.freertos.org/index.html

https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_Command_Line_Interface.html

For RC-Switch documentation, refer to Project home: https://github.com/sui77/rc-switch/

For RMT API documentation, in your local Arduino/hardware/espressif/esp32/cores/esp32/ folder, you may find the API sources: esp32-hal-rmt.c and esp32-hal-rmt.h
