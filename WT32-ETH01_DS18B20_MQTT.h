/**
 * Created by Adam on 2023-03-23.
 */

#ifndef WT32_ETH01_MQTT_WT32_ETH01_MQTT_H
#define WT32_ETH01_MQTT_WT32_ETH01_MQTT_H

#include "Ethernet.h"
#include "PubSubClient.h"
#include <DallasTemperature.h>
#include <OneWire.h>

const unsigned int RX_LED            = 5;                         // The GPIO used by the receive LED.
const unsigned int TX_LED            = 17;                        // The GPIO used by the transmit LED.
const unsigned int LED_OFF           = 0;                         // This allows the program to accommodate boards that use non-standard HIGH and LOW values.
const unsigned int LED_ON            = 1;                         // This allows the program to accommodate boards that use non-standard HIGH and LOW values.
const unsigned int ONE_WIRE_BUS_GPIO = 2;                         // The GPIO used by the Dallas One-Wire devices.
unsigned long lastMqttConnectionTime = 0;                         // The time of the last MQTT broker connection attempt.  Used by mqttConnect() to prevent swamping the broker with connection attempts.
unsigned long mqttCoolDownInterval   = 5000;                      // The time between MQTT broker connection attempts.
unsigned long lastPrintTime          = 0;                         // The last time stats were printed to the serial port.
unsigned long printInterval          = 5000;                      // The time between prints.
unsigned long printCount             = 0;                         // A count of how many times the printTelemetry() function has been called.
unsigned int invalidValueCount       = 0;                         // A count of how many reading were out of range.
float ds18TempCArray0[]              = { -21.12, 21.12, 88.88 };  // An array to hold the 3 most recent Celsius values.
float ds18TempCArray1[]              = { -21.12, 21.12, 88.88 };  // An array to hold the 3 most recent Celsius values.
float ds18TempCArray2[]              = { -21.12, 21.12, 88.88 };  // An array to hold the 3 most recent Celsius values.
float ds18TempCArray3[]              = { -21.12, 21.12, 88.88 };  // An array to hold the 3 most recent Celsius values.
float ds18TempCArray4[]              = { -21.12, 21.12, 88.88 };  // An array to hold the 3 most recent Celsius values.
float ds18TempCArray5[]              = { -21.12, 21.12, 88.88 };  // An array to hold the 3 most recent Celsius values.


// Set up a oneWire instance on GPIO2 which can communicate with any OneWire devices.
OneWire oneWire( ONE_WIRE_BUS_GPIO );
DallasTemperature ds18b20( &oneWire );

void setup();
void toggleLED();
void pollTelemetry();
void publishTemperature( float valueArray[], char *topicC, char *topicF );
void addValue( float valueArray[], unsigned int size, float value, float minValue, float maxValue );
float findMaximum( float valueArray[], unsigned int size );
float findMinimum( float valueArray[], unsigned int size );
void getTemp( unsigned int index, float valueArray[] );
void printTelemetry();
void loop();
float cToF( float value );
float averageArray( float valueArray[] );


#endif  //WT32_ETH01_MQTT_WT32_ETH01_MQTT_H
