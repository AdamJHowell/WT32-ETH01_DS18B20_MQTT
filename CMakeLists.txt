cmake_minimum_required( VERSION 3.24 )
project( WT32-ETH01_DS18B20_MQTT )

set( CMAKE_CXX_STANDARD 17 )

add_executable( WT32-ETH01_DS18B20_MQTT
		WT32-ETH01_DS18B20_MQTT.ino
		WT32-ETH01_DS18B20_MQTT.h
		Ethernet.ino
		Ethernet.h )

set_target_properties( WT32-ETH01_DS18B20_MQTT PROPERTIES LINKER_LANGUAGE CXX )
