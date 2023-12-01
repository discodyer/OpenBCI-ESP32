/**
 * Name: OpenBCI_Wifi_Definitions.h
 * Date: 8/30/2016
 * Purpose: This is the header file for the OpenBCI wifi definitions.
 *
 * Author: Push The World LLC (AJ Keller)
 */

#ifndef __OpenBCI_Wifi_Definitions__
#define __OpenBCI_Wifi_Definitions__

#define SOFTWARE_VERSION "v2.0.5"

// #define DEBUG 1

#ifdef RAW_TO_JSON
#define ADS1299_VREF 4.5

#define ADC_24BIT_RES 8388607.0
#define ADC_24BIT_MAX_VAL_NANO_VOLT 8388607000000000.0

#define ADS_GAIN_1 1
#define ADS_GAIN_2 2
#define ADS_GAIN_4 4
#define ADS_GAIN_6 6
#define ADS_GAIN_8 8
#define ADS_GAIN_12 12
#define ADS_GAIN_24 24

#define ADS_SCALE_FACTOR_VOLTS_1 0.000000536441867
#define ADS_SCALE_FACTOR_VOLTS_2 0.000000268220934
#define ADS_SCALE_FACTOR_VOLTS_4 0.000000134110467
#define ADS_SCALE_FACTOR_VOLTS_6 0.000000089406978
#define ADS_SCALE_FACTOR_VOLTS_8 0.000000067055233
#define ADS_SCALE_FACTOR_VOLTS_12 0.000000044703489
#define ADS_SCALE_FACTOR_VOLTS_24 0.000000022351744

#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_ADDITIONAL_BYTES_4_CHAN 115
#define ARDUINOJSON_ADDITIONAL_BYTES_8_CHAN 195
#define ARDUINOJSON_ADDITIONAL_BYTES_16_CHAN 355
#define ARDUINOJSON_ADDITIONAL_BYTES_24_CHAN 515
#define ARDUINOJSON_ADDITIONAL_BYTES_32_CHAN 675

#define GANGLION_GAIN 51

#define MCP_SCALE_FACTOR_VOLTS 0.00000000186995
#define MCP3912_VREF 1.2
#define NUM_PACKETS_IN_RING_BUFFER_JSON 28
#define NUM_PACKETS_IN_RING_BUFFER_RAW 1
#else
#define NUM_PACKETS_IN_RING_BUFFER_RAW 200
#define MAX_PACKETS_PER_SEND_TCP 42
#endif
#define BYTES_PER_SPI_PACKET 32
#define BYTES_PER_OBCI_PACKET 33
#define BYTES_PER_CHANNEL 3
#define WIFI_SPI_MSG_LAST 0x01
#define WIFI_SPI_MSG_MULTI 0x02
#define WIFI_SPI_MSG_GAINS 0x03
#define MAX_CHANNELS 16
#define MAX_CHANNELS_PER_PACKET 8
#define NUM_CHANNELS_CYTON 8
#define NUM_CHANNELS_CYTON_DAISY 16
#define NUM_CHANNELS_GANGLION 4
#define LED_PROG 0
#define LED_NOTIFY 5
#define DEFAULT_LATENCY 10000
#define DEFAULT_MQTT_PORT 1883
// #define bit(b) (1UL << (b)) // Taken directly from Arduino.h
// Arduino JSON needs bytes for duplication
// to recalculate visit:
//   https://bblanchon.github.io/ArduinoJson/assistant/index.html
// #define ARDUINOJSON_USE_DOUBLE 1

#define MICROS_IN_SECONDS 1000000
#define SPI_MASTER_POLL_TIMEOUT_MS 100
#define SPI_TIMEOUT_CLIENT_RESPONSE 400
#define SPI_NO_MASTER 401
#define CLIENT_RESPONSE_NO_BODY_IN_POST 402
#define CLIENT_RESPONSE_MISSING_REQUIRED_CMD 403
#define NANO_VOLTS_IN_VOLTS 1000000000.0
#define MAX_JSON_BUFFER_SIZE 4000

#define BOARD_TYPE_CYTON "cyton"
#define BOARD_TYPE_CYTON_DAISY "daisy"
#define BOARD_TYPE_GANGLION "ganglion"
#define BOARD_TYPE_NONE "none"

#define OUTPUT_JSON "json"
#define OUTPUT_MQTT "mqtt"
#define OUTPUT_NONE "none"
#define OUTPUT_RAW "raw"
#define OUTPUT_SERIAL "serial"
#define OUTPUT_TCP "tcp"
#define OUTPUT_WEB_SOCKETS "ws"

#define JSON_BOARD_CONNECTED "board_connected"
#define JSON_BOARD_TYPE "board_type"
#define JSON_COMMAND "command"
#define JSON_CONNECTED "connected"
#define JSON_GAINS "gains"
#define JSON_HEAP "heap"
#define JSON_LATENCY "latency"
#define JSON_MAC "mac"
#define JSON_MQTT_BROKER_ADDR "broker_address"
#define JSON_MQTT_PASSWORD "password"
#define JSON_MQTT_USERNAME "username"
#define JSON_MQTT_PORT "port"
#define JSON_NAME "name"
#define JSON_NUM_CHANNELS "num_channels"
#define JSON_REDUNDANCY "redundancy"
#define JSON_SAMPLE_NUMBERS "sample_numbers"
#define JSON_SAMPLE_NUMBER "sampleNumber"
#define JSON_TCP_DELIMITER "delimiter"
#define JSON_TCP_IP "ip"
#define JSON_TCP_OUTPUT "output"
#define JSON_TCP_PORT "port"
#define JSON_TIMESTAMPS "timestamps"
#define JSON_VERSION "version"

// Used to determine the result of processing a packet
#define PASSTHROUGH_FAIL_TOO_MANY_CHARS 0x00
#define PASSTHROUGH_FAIL_NO_CHARS 0x01
#define PASSTHROUGH_FAIL_QUEUE_FILLED 0x02
#define PASSTHROUGH_PASS 0x03
#define PROCESS_RAW_FAIL_OVERFLOW_FIRST 0x00
#define PROCESS_RAW_FAIL_OVERFLOW_FIRST_AFTER_SWITCH 0x01
#define PROCESS_RAW_FAIL_SWITCH 0x02
#define PROCESS_RAW_FAIL_OVERFLOW_MIDDLE 0x03
#define PROCESS_RAW_PASS_FIRST 0x04
#define PROCESS_RAW_PASS_SWITCH 0x05
#define PROCESS_RAW_PASS_MIDDLE 0x06
#define STREAM_PACKET_BYTE_START 0xA0
#define STREAM_PACKET_BYTE_STOP 0xC0

#define MQTT_ROUTE_KEY "openbci:eeg"

#define HTTP_ROUTE_MQTT "/mqtt"
#define HTTP_ROUTE_TCP "/tcp"
#define HTTP_ROUTE_UDP "/udp"
#define HTTP_ROUTE "/"
#define HTTP_ROUTE_CLOUD "/cloud"
#define HTTP_ROUTE_YT "/yt"
#define HTTP_ROUTE_OUTPUT_JSON "/output/json"
#define HTTP_ROUTE_OUTPUT_RAW "/output/raw"
#define HTTP_ROUTE_STREAM_START "/stream/start"
#define HTTP_ROUTE_STREAM_STOP "/stream/stop"
#define HTTP_ROUTE_VERSION "/version"
#define HTTP_ROUTE_COMMAND "/command"
#define HTTP_ROUTE_LATENCY "/latency"
#define HTTP_ROUTE_ALL "/all"
#define HTTP_ROUTE_BOARD "/board"
#define HTTP_ROUTE_WIFI "/wifi"
#define HTTP_ROUTE_WIFI_CONFIG "/wifi/config"
#define HTTP_ROUTE_WIFI_DELETE "/wifi/delete"

#define RETURN_TEXT_JSON "text/json"

// OPENBCI_COMMANDS
/** Turning channels off */
#define OPENBCI_CHANNEL_OFF_1 '1'
#define OPENBCI_CHANNEL_OFF_2 '2'
#define OPENBCI_CHANNEL_OFF_3 '3'
#define OPENBCI_CHANNEL_OFF_4 '4'
#define OPENBCI_CHANNEL_OFF_5 '5'
#define OPENBCI_CHANNEL_OFF_6 '6'
#define OPENBCI_CHANNEL_OFF_7 '7'
#define OPENBCI_CHANNEL_OFF_8 '8'
#define OPENBCI_CHANNEL_OFF_9 'q'
#define OPENBCI_CHANNEL_OFF_10 'w'
#define OPENBCI_CHANNEL_OFF_11 'e'
#define OPENBCI_CHANNEL_OFF_12 'r'
#define OPENBCI_CHANNEL_OFF_13 't'
#define OPENBCI_CHANNEL_OFF_14 'y'
#define OPENBCI_CHANNEL_OFF_15 'u'
#define OPENBCI_CHANNEL_OFF_16 'i'

/** Turn channels on */
#define OPENBCI_CHANNEL_ON_1 '!'
#define OPENBCI_CHANNEL_ON_2 '@'
#define OPENBCI_CHANNEL_ON_3 '#'
#define OPENBCI_CHANNEL_ON_4 '$'
#define OPENBCI_CHANNEL_ON_5 '%'
#define OPENBCI_CHANNEL_ON_6 '^'
#define OPENBCI_CHANNEL_ON_7 '&'
#define OPENBCI_CHANNEL_ON_8 '*'
#define OPENBCI_CHANNEL_ON_9 'Q'
#define OPENBCI_CHANNEL_ON_10 'W'
#define OPENBCI_CHANNEL_ON_11 'E'
#define OPENBCI_CHANNEL_ON_12 'R'
#define OPENBCI_CHANNEL_ON_13 'T'
#define OPENBCI_CHANNEL_ON_14 'Y'
#define OPENBCI_CHANNEL_ON_15 'U'
#define OPENBCI_CHANNEL_ON_16 'I'

/** Test Signal Control Commands
 * 1x - Voltage will be 1 * (VREFP - VREFN) / 2.4 mV
 * 2x - Voltage will be 2 * (VREFP - VREFN) / 2.4 mV
 */
#define OPENBCI_TEST_SIGNAL_CONNECT_TO_DC            'p'
#define OPENBCI_TEST_SIGNAL_CONNECT_TO_GROUND        '0'
#define OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_1X_FAST '='
#define OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_1X_SLOW '-'
#define OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_2X_FAST ']'
#define OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_2X_SLOW '['

/** Channel Setting Commands */
#define OPENBCI_CHANNEL_CMD_ADC_Normal      '0'
#define OPENBCI_CHANNEL_CMD_ADC_Shorted     '1'
#define OPENBCI_CHANNEL_CMD_ADC_BiasDRP     '6'
#define OPENBCI_CHANNEL_CMD_ADC_BiasDRN     '7'
#define OPENBCI_CHANNEL_CMD_ADC_BiasMethod  '2'
#define OPENBCI_CHANNEL_CMD_ADC_MVDD        '3'
#define OPENBCI_CHANNEL_CMD_ADC_Temp        '4'
#define OPENBCI_CHANNEL_CMD_ADC_TestSig     '5'
#define OPENBCI_CHANNEL_CMD_BIAS_INCLUDE    '1'
#define OPENBCI_CHANNEL_CMD_BIAS_REMOVE     '0'
#define OPENBCI_CHANNEL_CMD_CHANNEL_1       '1'
#define OPENBCI_CHANNEL_CMD_CHANNEL_2       '2'
#define OPENBCI_CHANNEL_CMD_CHANNEL_3       '3'
#define OPENBCI_CHANNEL_CMD_CHANNEL_4       '4'
#define OPENBCI_CHANNEL_CMD_CHANNEL_5       '5'
#define OPENBCI_CHANNEL_CMD_CHANNEL_6       '6'
#define OPENBCI_CHANNEL_CMD_CHANNEL_7       '7'
#define OPENBCI_CHANNEL_CMD_CHANNEL_8       '8'
#define OPENBCI_CHANNEL_CMD_CHANNEL_9       'Q'
#define OPENBCI_CHANNEL_CMD_CHANNEL_10      'W'
#define OPENBCI_CHANNEL_CMD_CHANNEL_11      'E'
#define OPENBCI_CHANNEL_CMD_CHANNEL_12      'R'
#define OPENBCI_CHANNEL_CMD_CHANNEL_13      'T'
#define OPENBCI_CHANNEL_CMD_CHANNEL_14      'Y'
#define OPENBCI_CHANNEL_CMD_CHANNEL_15      'U'
#define OPENBCI_CHANNEL_CMD_CHANNEL_16      'I'
#define OPENBCI_CHANNEL_CMD_GAIN_1          '0'
#define OPENBCI_CHANNEL_CMD_GAIN_2          '1'
#define OPENBCI_CHANNEL_CMD_GAIN_4          '2'
#define OPENBCI_CHANNEL_CMD_GAIN_6          '3'
#define OPENBCI_CHANNEL_CMD_GAIN_8          '4'
#define OPENBCI_CHANNEL_CMD_GAIN_12         '5'
#define OPENBCI_CHANNEL_CMD_GAIN_24         '6'
#define OPENBCI_CHANNEL_CMD_LATCH           'X'
#define OPENBCI_CHANNEL_CMD_POWER_OFF       '1'
#define OPENBCI_CHANNEL_CMD_POWER_ON        '0'
#define OPENBCI_CHANNEL_CMD_SET             'x'
#define OPENBCI_CHANNEL_CMD_SRB1_CONNECT    '1'
#define OPENBCI_CHANNEL_CMD_SRB1_DISCONNECT '0'
#define OPENBCI_CHANNEL_CMD_SRB2_CONNECT    '1'
#define OPENBCI_CHANNEL_CMD_SRB2_DISCONNECT '0'

/** Default Channel Settings */
#define OPENBCI_CHANNEL_DEFAULT_ALL_SET 'd'
#define OPENBCI_CHANNEL_DEFAULT_ALL_REPORT 'D'

/** LeadOff Impedance Commands */
#define OPENBCI_CHANNEL_IMPEDANCE_LATCH                'Z'
#define OPENBCI_CHANNEL_IMPEDANCE_SET                  'z'
#define OPENBCI_CHANNEL_IMPEDANCE_TEST_SIGNAL_APPLIED    '1'
#define OPENBCI_CHANNEL_IMPEDANCE_TEST_SIGNAL_APPLIED_NOT '0'

/** SD card Commands */
#define OPENBCI_SD_LOG_FOR_HOUR_1    'G'
#define OPENBCI_SD_LOG_FOR_HOUR_2    'H'
#define OPENBCI_SD_LOG_FOR_HOUR_4    'J'
#define OPENBCI_SD_LOG_FOR_HOUR_12   'K'
#define OPENBCI_SD_LOG_FOR_HOUR_24   'L'
#define OPENBCI_SD_LOG_FOR_MIN_5     'A'
#define OPENBCI_SD_LOG_FOR_MIN_15    'S'
#define OPENBCI_SD_LOG_FOR_MIN_30    'F'
#define OPENBCI_SD_LOG_FOR_SEC_14    'a'
#define OPENBCI_SD_LOG_STOP        'j'

/** Stream Data Commands */
#define OPENBCI_STREAM_START  'b'
#define OPENBCI_STREAM_STOP   's'

/** Miscellaneous */
#define OPENBCI_MISC_QUERY_REGISTER_SETTINGS '?'
#define OPENBCI_MISC_SOFT_RESET              'v'

/** 16 Channel Commands */
#define OPENBCI_CHANNEL_MAX_NUMBER_8    'c'
#define OPENBCI_CHANNEL_MAX_NUMBER_16   'C'

#define OPENBCI_BOARD_MODE_SET '/'

#define OPENBCI_GET_VERSION 'V'

/** Set sample rate */
#define OPENBCI_SAMPLE_RATE_SET '~'

/** Insert marker into the stream */
#define OPENBCI_INSERT_MARKER '`'

/** Sync Clocks */
#define OPENBCI_TIME_SET '<'
#define OPENBCI_TIME_STOP '>'

/** Wifi Stuff */
#define OPENBCI_WIFI_ATTACH '{'
#define OPENBCI_WIFI_REMOVE '}'
#define OPENBCI_WIFI_STATUS ':'
#define OPENBCI_WIFI_RESET ';'

/** Possible Sample Rates*/
#define OPENBCI_SAMPLE_RATE_125 125
#define OPENBCI_SAMPLE_RATE_250 250

/** Time out for multi char commands **/
#define MULTI_CHAR_COMMAND_TIMEOUT_MS 1000

/** Packet Size */
#define OPENBCI_PACKET_SIZE 33

/** Impedance Calculation Variables */
#define OPENBCI_LEAD_OFF_DRIVE_AMPS 0.000000006
#define OPENBCI_LEAD_OFF_FREQUENCY_HZ 31

#define OPENBCI_TIME_OUT_MS_1 1
#define OPENBCI_TIME_OUT_MS_3 3

#define OPENBCI_NUMBER_OF_BYTES_SETTINGS_CHANNEL 9
#define OPENBCI_NUMBER_OF_BYTES_SETTINGS_LEAD_OFF 5

#define OPENBCI_NUMBER_OF_BYTES_AUX 6

#define OPENBCI_FIRMWARE_VERSION_V1 1
#define OPENBCI_FIRMWARE_VERSION_V2 1

/** BLE Packet Information */
#define BLE_BYTES_PER_PACKET 20
#define BLE_BYTES_PER_SAMPLE 6
#define BLE_SAMPLES_PER_PACKET 3
#define BLE_TOTAL_DATA_BYTES 18
#define BLE_RING_BUFFER_SIZE 50


#endif
