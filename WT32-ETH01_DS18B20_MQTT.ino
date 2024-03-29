/**
 * This program will use the Ethernet port on a WT32-ETH01 devkit to publish telemetry to a MQTT broker.
 */
#include "WT32-ETH01_DS18B20_MQTT.h"


void setup()
{
	delay( 1000 );
	Serial.begin( 115200 );
	if( !Serial )
		delay( 1000 );
	Serial.println( "\n" );
	Serial.println( "Function setup() is beginning." );
	WiFi.onEvent( NetworkEvent );
	Serial.println( "Network callbacks configured." );
	ETH.begin();
	// This delay give the Ethernet hardware time to initialize.
	delay( 300 );

	// Start up the Dallas DS18B20 temperature sensors.
	ds18b20.begin();

	pinMode( RX_LED, OUTPUT );
	pinMode( TX_LED, OUTPUT );

	digitalWrite( RX_LED, LED_OFF );
	digitalWrite( TX_LED, LED_OFF );

	// Read from the sensors twice, to populate telemetry arrays.
	pollTelemetry();
	pollTelemetry();

	Serial.println( "Function setup() has completed." );
}  // End of the setup() function.


/**
 * @brief toggleLED() will change the state of the LED.
 * This function does not manage any timings.
 */
void toggleLED()
{
	if( digitalRead( TX_LED ) != LED_ON )
	{
		digitalWrite( RX_LED, LED_OFF );
		digitalWrite( TX_LED, LED_ON );
		Serial.println( "LED on" );
	}
	else
	{
		digitalWrite( RX_LED, LED_ON );
		digitalWrite( TX_LED, LED_OFF );
		Serial.println( "LED off" );
	}
}  // End of toggleLED() function.


/**
 * @brief addValue() will add the passed value to the 0th element of the passed array, after moving the existing array values to higher indexes.
 * If value is less than minValue, or greater than maxValue, it will be discarded and nothing will be added to valueArray.
 */
void addValue( float valueArray[], unsigned int size, float value, float minValue, float maxValue )
{
	// Prevent sensor anomalies from getting into the array.
	if( value < minValue || value > maxValue )
	{
		Serial.printf( "\n\nValue %f is not between %f and %f!\n\n", value, minValue, maxValue );
		invalidValueCount++;
		return;
	}

	// Detect outliers.
	float minArrayValue = findMinimum( valueArray, size );
	float maxArrayValue = findMaximum( valueArray, size );
	if( value < ( minArrayValue / 2 ) )
	{
		Serial.printf( "\n\nValue %f is less than half the smallest existing value of %f!\n\n", value, minArrayValue );
		invalidValueCount++;
		return;
	}
	if( value > ( maxArrayValue * 2 ) )
	{
		Serial.printf( "\n\nValue %f is more than double the largest existing value of %f!\n\n", value, maxArrayValue );
		invalidValueCount++;
		return;
	}

	valueArray[2] = valueArray[1];
	valueArray[1] = valueArray[0];
	valueArray[0] = value;
}  // End of the addValue() function.


/**
 * @brief findMaximum() will return the largest value in the passed array.
 */
float findMaximum( float valueArray[], unsigned int size )
{
	float maxValue = valueArray[0];
	for( int i = 1; i < size; ++i )
	{
		if( valueArray[i] > maxValue )
			maxValue = valueArray[i];
	}
	return maxValue;
}  // End of the findMaximum() function.


/**
 * @brief findMinimum() will return the smallest value in the passed array.
 */
float findMinimum( float valueArray[], unsigned int size )
{
	float minValue = valueArray[0];
	for( int i = 1; i < size; ++i )
	{
		if( valueArray[i] < minValue )
			minValue = valueArray[i];
	}
	return minValue;
}  // End of the findMinimum() function.


/**
 * @brief averageArray() will return the average of values in the passed array.
 */
float averageArray( float valueArray[] )
{
	const unsigned int arraySize = 3;
	float tempValue              = 0;
	for( int i = 0; i < arraySize; ++i )
	{
		tempValue += valueArray[i];
	}
	return tempValue / arraySize;
}  // End of the averageArray() function.


/**
 * @brief cToF() will convert Celsius to Fahrenheit.
 */
float cToF( float value )
{
	return value * 1.8 + 32;
}  // End of the cToF() function.


/**
 * @brief getTemp() will read the Celsius temperature and save it into the passed array.
 */
void getTemp( unsigned int index, float valueArray[] )
{
	float ds18TempC = ds18b20.getTempCByIndex( index );
	if( ds18TempC != DEVICE_DISCONNECTED_C )
		addValue( valueArray, 3, ds18TempC, -42, 212 );
	else
		Serial.println( "Error: Failed to read temperature data!" );
}  // End of the getTemp() function.


void pollTelemetry()
{
	// Request all Dallas sensors on the bus to read their temperature.
	ds18b20.requestTemperatures();
	// Read the temperature from the sensor at index 0.
	getTemp( 0, ds18TempCArray0 );
// 	getTemp( 1, ds18TempCArray1 );
// 	getTemp( 2, ds18TempCArray2 );
// 	getTemp( 3, ds18TempCArray3 );
// 	getTemp( 4, ds18TempCArray4 );
// 	getTemp( 5, ds18TempCArray5 );
}  // End of the pollTelemetry() function.


/**
 * @brief printTelemetry() will print the telemetry to the serial port.
 */
void printTelemetry()
{
	Serial.println();
	printCount++;
	Serial.println( __FILE__ );
	Serial.printf( "Print count %ld\n", printCount );
	Serial.println();

	if( eth_connected )
	{
		Serial.println( "Ethernet stats:" );
		Serial.printf( "  MAC address: %s\n", macAddress );
		Serial.printf( "  IP address: %s\n", ipAddress );
		Serial.printf( "  Duplex: %s\n", duplex );
		Serial.printf( "  Link speed: %u Mbps\n", linkSpeed );
		Serial.printf( "  Network callback count: %lu\n", networkCallbackCount );
		Serial.println();
	}

	Serial.println( "MQTT stats:" );
	Serial.printf( "  mqttConnectCount: %u\n", mqttConnectCount );
	Serial.printf( "  mqttCoolDownInterval: %lu\n", mqttCoolDownInterval );
	Serial.printf( "  Broker: %s:%d\n", BROKER_ADDRESS, BROKER_PORT );
	Serial.printf( "  Client ID: %s\n", HOSTNAME );
	char buffer[29];
	lookupMQTTCode( mqttClient.state(), buffer );
	Serial.printf( "  MQTT state: %s\n", buffer );
	Serial.printf( "  Publish count: %lu\n", publishCount );
	Serial.printf( "  MQTT callback count: %lu\n", mqttCallbackCount );
	Serial.println();

	Serial.println( "Environmental stats:" );
	Serial.println( "  Sensor 0:" );
	Serial.printf( "    Temp: %.2f C\n", averageArray( ds18TempCArray0 ) );
	Serial.printf( "    Temp: %.2f F\n", cToF( averageArray( ds18TempCArray0 ) ) );
// 	Serial.println( "  Sensor 1:" );
// 	Serial.printf( "    Temp: %.2f C\n", averageArray( ds18TempCArray1 ) );
// 	Serial.printf( "    Temp: %.2f F\n", cToF( averageArray( ds18TempCArray1 ) ) );
// 	Serial.println( "  Sensor 2:" );
// 	Serial.printf( "    Temp: %.2f C\n", averageArray( ds18TempCArray2 ) );
// 	Serial.printf( "    Temp: %.2f F\n", cToF( averageArray( ds18TempCArray2 ) ) );
// 	Serial.println( "  Sensor 3:" );
// 	Serial.printf( "    Temp: %.2f C\n", averageArray( ds18TempCArray3 ) );
// 	Serial.printf( "    Temp: %.2f F\n", cToF( averageArray( ds18TempCArray3 ) ) );
// 	Serial.println( "  Sensor 4:" );
// 	Serial.printf( "    Temp: %.2f C\n", averageArray( ds18TempCArray4 ) );
// 	Serial.printf( "    Temp: %.2f F\n", cToF( averageArray( ds18TempCArray4 ) ) );
// 	Serial.println( "  Sensor 5:" );
// 	Serial.printf( "    Temp: %.2f C\n", averageArray( ds18TempCArray5 ) );
// 	Serial.printf( "    Temp: %.2f F\n", cToF( averageArray( ds18TempCArray5 ) ) );
	Serial.println();
}  // End of the printTelemetry() function.


/**
 * @brief publishTemperature() will publish the value from the array to the provided topics.
 */
void publishTemperature( float valueArray[], const char *topicC, const char *topicF )
{
	char valueBuffer[25] = "";
	snprintf( valueBuffer, 25, "%.3f", averageArray( valueArray ) );
	if( mqttClient.publish( topicC, valueBuffer ) )
		Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, topicC );
	snprintf( valueBuffer, 25, "%.3f", cToF( averageArray( valueArray ) ) );
	if( mqttClient.publish( topicF, valueBuffer ) )
		Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, topicF );
}  // End of the publishTemperature() function.


void loop()
{
	if( eth_connected && !mqttClient.connected() )
		mqttConnect( BROKER_ADDRESS, BROKER_PORT );
	else
		mqttClient.loop();

	unsigned int currentTime = millis();
	// Avoid overflow.  Print every interval.
	if( currentTime > printInterval && ( currentTime - printInterval ) > lastPrintTime )
	{
		toggleLED();
		pollTelemetry();
		printTelemetry();
		lastPrintTime = millis();
	}

	currentTime = millis();
	// Publish only if connected.  Publish the first time.  Avoid overflow.  Publish every interval.
	if( ( eth_connected && mqttClient.connected() ) && ( lastPublishTime == 0 || ( currentTime > publishInterval && ( currentTime - publishInterval ) > lastPublishTime ) ) )
	{
		publishCount++;
		char valueBuffer[25] = "";
		snprintf( valueBuffer, 25, "%lu", publishCount );
		if( mqttClient.publish( PUBLISH_TOPIC, valueBuffer ) )
			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, PUBLISH_TOPIC );
		if( mqttClient.publish( MAC_TOPIC, macAddress ) )
			Serial.printf( "Successfully published to '%s' to '%s'\n", macAddress, MAC_TOPIC );
		if( mqttClient.publish( IP_TOPIC, ipAddress ) )
			Serial.printf( "Successfully published to '%s' to '%s'\n", ipAddress, IP_TOPIC );
		snprintf( valueBuffer, 25, "%lu", networkCallbackCount );
		if( mqttClient.publish( CALLBACK_COUNT_TOPIC, valueBuffer ) )
			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, CALLBACK_COUNT_TOPIC );
		snprintf( valueBuffer, 25, "%lu", mqttCallbackCount );
		if( mqttClient.publish( MQTT_CALLBACK_COUNT_TOPIC, valueBuffer ) )
			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, MQTT_CALLBACK_COUNT_TOPIC );

		// Temperature data
//		snprintf( valueBuffer, 25, "%.3f", averageArray( ds18TempCArray0 ) );
//		if( mqttClient.publish( DS18_TEMP_C_TOPIC0, valueBuffer ) )
//			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, DS18_TEMP_C_TOPIC0 );
//		snprintf( valueBuffer, 25, "%.3f", cToF( averageArray( ds18TempCArray0 ) ) );
//		if( mqttClient.publish( DS18_TEMP_F_TOPIC0, valueBuffer ) )
//			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, DS18_TEMP_F_TOPIC0 );
//
//		snprintf( valueBuffer, 25, "%.3f", averageArray( ds18TempCArray1 ) );
//		if( mqttClient.publish( DS18_TEMP_C_TOPIC1, valueBuffer ) )
//			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, DS18_TEMP_C_TOPIC1 );
//		snprintf( valueBuffer, 25, "%.3f", cToF( averageArray( ds18TempCArray1 ) ) );
//		if( mqttClient.publish( DS18_TEMP_F_TOPIC1, valueBuffer ) )
//			Serial.printf( "Successfully published to '%s' to '%s'\n", valueBuffer, DS18_TEMP_F_TOPIC1 );

		publishTemperature( ds18TempCArray0, DS18_TEMP_C_TOPIC0, DS18_TEMP_F_TOPIC0 );
// 		publishTemperature( ds18TempCArray1, DS18_TEMP_C_TOPIC1, DS18_TEMP_F_TOPIC1 );
// 		publishTemperature( ds18TempCArray2, DS18_TEMP_C_TOPIC2, DS18_TEMP_F_TOPIC2 );
// 		publishTemperature( ds18TempCArray3, DS18_TEMP_C_TOPIC3, DS18_TEMP_F_TOPIC3 );
// 		publishTemperature( ds18TempCArray4, DS18_TEMP_C_TOPIC4, DS18_TEMP_F_TOPIC4 );
// 		publishTemperature( ds18TempCArray5, DS18_TEMP_C_TOPIC5, DS18_TEMP_F_TOPIC5 );

		Serial.printf( "Next publish in %u seconds.\n\n", publishInterval / 1000 );
		lastPublishTime = millis();
	}
}  // End of the loop() function.
