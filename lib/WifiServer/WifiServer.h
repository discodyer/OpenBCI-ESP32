#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>
#include <WiFiUdp.h>
#include "OpenBCI_Wifi_Definitions.h"
#include "Config.h"
#include "ESP32SSDP.h"
#include "ESPmDNS.h"
#include "WebServer.h"

class ADS1299;

// #define DEBUG

extern WebServer server;
extern WiFiClass WiFi;
extern WiFiUDP clientUDP;
extern WiFiClient clientTCP;

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1440
#endif

#define PIN_LED 38
#define WIFI_SSID "HUAWEI-AE86_Wi-Fi5"
#define WIFI_PASSWD "20030717"

class WifiServer
{
public:
    // ENUMS
    enum CLIENT_RESPONSE
    {
        CLIENT_RESPONSE_NONE,
        CLIENT_RESPONSE_OUTPUT_STRING
    };

    enum OUTPUT_MODE
    {
        OUTPUT_MODE_RAW,
        OUTPUT_MODE_JSON
    };

    enum OUTPUT_PROTOCOL
    {
        OUTPUT_PROTOCOL_NONE,
        OUTPUT_PROTOCOL_TCP,
        OUTPUT_PROTOCOL_UDP,
        OUTPUT_PROTOCOL_MQTT,
        OUTPUT_PROTOCOL_WEB_SOCKETS,
        OUTPUT_PROTOCOL_SERIAL,
        OUTPUT_PROTOCOL_AZURE_EVENT_HUB
    };

    enum CYTON_GAIN
    {
        CYTON_GAIN_1,
        CYTON_GAIN_2,
        CYTON_GAIN_4,
        CYTON_GAIN_6,
        CYTON_GAIN_8,
        CYTON_GAIN_12,
        CYTON_GAIN_24
    };

    enum MULTI_CHAR_COMMAND
    {
        MULTI_CHAR_CMD_NONE,
        MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_CHANNEL,
        MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_LEADOFF,
        MULTI_CHAR_CMD_SERIAL_PASSTHROUGH,
        MULTI_CHAR_CMD_SETTINGS_BOARD_MODE,
        MULTI_CHAR_CMD_SETTINGS_SAMPLE_RATE,
        MULTI_CHAR_CMD_INSERT_MARKER
    };

    enum PACKET_TYPE
    {
        PACKET_TYPE_ACCEL,
        PACKET_TYPE_RAW_AUX,
        PACKET_TYPE_USER_DEFINED,
        PACKET_TYPE_ACCEL_TIME_SET,
        PACKET_TYPE_ACCEL_TIME_SYNC,
        PACKET_TYPE_RAW_AUX_TIME_SET,
        PACKET_TYPE_RAW_AUX_TIME_SYNC
    };

    enum TIME_SYNC_MODE
    {
        TIME_SYNC_MODE_ON,
        TIME_SYNC_MODE_OFF
    };

    enum DEBUG_MODE
    {
        DEBUG_MODE_ON,
        DEBUG_MODE_OFF
    };

    enum ACCEL_MODE
    {
        ACCEL_MODE_ON,
        ACCEL_MODE_OFF
    };
    
    enum BOARD_MODE {
    BOARD_MODE_DEFAULT,
    BOARD_MODE_DEBUG,
    BOARD_MODE_ANALOG,
    BOARD_MODE_DIGITAL,
    BOARD_MODE_MARKER,
    BOARD_MODE_BLE,
    BOARD_MODE_END_OF_MODES  // This must be the last entry-insert any new board modes above this line
  };

    // STRUCTS
#ifdef RAW_TO_JSON
    typedef struct
    {
        double channelData[NUM_CHANNELS_GANGLION];
        unsigned long long timestamp;
        uint8_t sampleNumber;
    } Sample;
#endif

    // Functions and Methods
    WifiServer();
    void begin(void);
    void printWifiStatus();
#ifdef RAW_TO_JSON
    void channelDataCompute(uint8_t *, uint8_t *, Sample *, uint8_t, uint8_t);
#endif
    void debugPrintLLNumber(long long);
    void debugPrintLLNumber(long long, uint8_t);
    void debugPrintLLNumber(unsigned long long);
    void debugPrintLLNumber(unsigned long long, uint8_t);
    void extractRaws(uint8_t *, int32_t *, uint8_t);
    void gainReset(void);
    String getBoardTypeString(uint8_t);
    String getCurBoardTypeString();
    String getCurOutputModeString();
    String getCurOutputProtocolString();
    uint8_t *getGains(void);
    uint8_t getGainCyton(uint8_t b);
    uint8_t getGainGanglion(void);
    uint8_t getHead(void);
    String getInfoAll(void);
    String getInfoBoard(void);
#ifdef MQTT
    String getInfoMQTT(boolean);
#endif
    String getInfoTCP(boolean);
    int getJSONAdditionalBytes(uint8_t);
    size_t getJSONBufferSize(void);
#ifdef RAW_TO_JSON
    void getJSONFromSamples(JsonObject &, uint8_t, uint8_t);
#endif
    uint8_t getJSONMaxPackets(void);
    uint8_t getJSONMaxPackets(uint8_t);
    unsigned long getLatency(void);
    String getMacLastFourBytes(void);
    String getMac(void);
    String getModelNumber(void);
    String getName(void);
    uint8_t getNumChannels(void);
    unsigned long getNTPOffset(void);
    String getOutputModeString(OUTPUT_MODE);
    String getOutputProtocolString(OUTPUT_PROTOCOL);
    String getStringLLNumber(long long);
    String getStringLLNumber(long long, uint8_t);
    String getStringLLNumber(unsigned long long);
    String getStringLLNumber(unsigned long long, uint8_t);
#ifdef RAW_TO_JSON
    double getScaleFactorVoltsCyton(uint8_t);
    double getScaleFactorVoltsGanglion(void);
#endif
    uint8_t getTail(void);
    unsigned long long getTime(void);
    String getVersion();
    int32_t int24To32(uint8_t *);
    boolean isAStreamByte(uint8_t);
    void loop(void);
    void ProcessPacketResponse(String message);
    void printfWifi(const char *format, ...);
    void printlnWifi(const char *msg);
    const char *getBoardMode(void);
    const char *getSampleRate(void);
    void reportDefaultChannelSettings();
    void printAllRegisters();
    void printWifi(const char *msg);
    void printFailureWifi(const char *msg);
    boolean ntpActive(void);
    unsigned long long ntpGetPreciseAdjustment(unsigned long);
    unsigned long long ntpGetTime(void);
    void ntpStart(void);
    void passthroughBufferClear(void);
    uint8_t passthroughCommands(String);
    String perfectPrintByteHex(uint8_t);
    double rawToScaled(int32_t, double);
    void reset(void);
#ifdef RAW_TO_JSON
    void sampleReset(void);
    void sampleReset(Sample *);
    void sampleReset(Sample *, uint8_t);
#endif
    void setGains(uint8_t *);
    void setGains(uint8_t *, uint8_t *);
#ifdef MQTT
    void setInfoMQTT(String, String, String, int);
#endif
    void setInfoUDP(String, int, boolean);
    void setInfoTCP(String, int, boolean);
    void setLatency(unsigned long);
    void setNumChannels(uint8_t);
    void setNTPOffset(unsigned long);
    void setOutputMode(OUTPUT_MODE);
    void processCommands(String commands);
    void setBoardMode(uint8_t newBoardMode);
    void setOutputProtocol(OUTPUT_PROTOCOL);
    boolean spiHasMaster(void);
    void spiOnDataSent(void);
    void spiProcessPacket(uint8_t *);
    void spiProcessPacketGain(uint8_t *);
    void spiProcessPacketStream(uint8_t *);
    void spiProcessPacketStreamJSON(uint8_t *);
    void spiProcessPacketStreamRaw(uint8_t *);
    void spiProcessPacketResponse(uint8_t *);
    void transformRawsToScaledCyton(int32_t *, uint8_t *, uint8_t, double *);
    void transformRawsToScaledGanglion(int32_t *, double *);

    // Variables
    boolean clientWaitingForResponse;
    boolean clientWaitingForResponseFullfilled;
    boolean passthroughBufferLoaded;
    boolean jsonHasSampleNumbers;
    boolean jsonHasTimeStamps;
    boolean redundancy;
    boolean tcpDelimiter;

    CLIENT_RESPONSE curClientResponse;

    IPAddress tcpAddress;

    int mqttPort;
    int tcpPort;

    OUTPUT_MODE curOutputMode;
    OUTPUT_PROTOCOL curOutputProtocol;

    uint8_t rawBuffer[NUM_PACKETS_IN_RING_BUFFER_RAW][BYTES_PER_SPI_PACKET];

#ifdef RAW_TO_JSON
    Sample sampleBuffer[NUM_PACKETS_IN_RING_BUFFER_JSON];
#endif

#ifdef MQTT
    String mqttBrokerAddress;
    String mqttUsername;
    String mqttPassword;
#endif
    String outputString;

    byte sampleCounter;
    uint8_t lastSampleNumber;
    uint8_t passthroughPosition;
    uint8_t passthroughBuffer[BYTES_PER_SPI_PACKET];

    unsigned long lastTimeWasPolled;
    unsigned long timePassthroughBufferLoaded;

    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint32_t rawBufferHead;
    volatile uint32_t rawBufferTail;

    void startWebServer(void);
    boolean connectToWiFi(const char *, const char *);

    // HTTP Rest Helpers
    boolean noBodyInParam();
    void debugPrintDelete();
    void debugPrintGet();
    void debugPrintPost();
    void sendHeadersForCORS();
    void sendHeadersForOptions();
    void serverReturn(int, String);
    void returnOK(String);
    void returnOK();
    void returnNoSPIMaster();
    void returnNoBodyInPost();
    void returnMissingRequiredParam(const char *err);
    void returnFail(int code, String msg);
    void requestWifiManagerStart();
    JsonObject &getArgFromArgs(int args);
    JsonObject &getArgFromArgs();
    void setLatency();
    void passthroughCommand();
    void tcpSetup();
    void udpSetup();
    void removeWifiAPInfo(void);

    boolean processChar(char character);
    boolean checkMultiCharCmdTimer(void);
    char getMultiCharCommand(void);
    void processIncomingChannelSettings(char);
    void processIncomingLeadOffSettings(char);
    void processIncomingBoardMode(char);
    void processIncomingSampleRate(char);
    void processInsertMarker(char);
    void startMultiCharCmdTimer(char cmd);
    void setCurPacketType(void);
    void endMultiCharCmdTimer(void);
    char getChannelCommandForAsciiChar(char asciiChar);
    char getNumberForAsciiChar(char asciiChar);
    char getGainForAsciiChar(char asciiChar);
    void sendChannelDataWifi(boolean daisy);
    void sendChannelDataWifi(PACKET_TYPE packetType, boolean daisy);

    ~WifiServer();

private:
    // Variables

    // OpenBCI command vars
    char multiCharCommand; // The type of command
    char currentChannelSetting;
    boolean isMultiCharCmd;            // A multi char command is in progress
    unsigned long multiCharCmdTimeout; // the timeout in millis of the current multi char command
    int numberOfIncomingSettingsProcessedChannel;
    int numberOfIncomingSettingsProcessedLeadOff;
    int numberOfIncomingSettingsProcessedBoardType;
    uint8_t optionalArgCounter;
    char optionalArgBuffer7[7];
    boolean newMarkerReceived;  // flag to indicate a new marker has been received
    char markerValue;

    TIME_SYNC_MODE curTimeSyncMode;
    PACKET_TYPE curPacketType;
    ACCEL_MODE curAccelMode;
    BOARD_MODE curBoardMode;

    // WebServer vars
    size_t _jsonBufferSize;

    uint8_t _gains[MAX_CHANNELS];
    // uint8_t curNumChannels;

    unsigned long _counter;
    unsigned long _latency;
    unsigned long _ntpOffset;

    ADS1299 &_ads1299;
    HardwareSerial &_serial;
    // AsyncWebServer &_server;
    WiFiClass &_WiFi;

    boolean startWifiManager;
    boolean underSelfTest;
    boolean tryConnectToAP;
    boolean wifiReset;
    boolean ledState;

    int ledFlashes;
    int ledInterval;
    unsigned long ledLastFlash;
    int udpPort;
    IPAddress udpAddress;

    String jsonStr;
    JsonObject _root;

    unsigned long lastSendToClient;
    unsigned long lastHeadMove;
    unsigned long wifiConnectTimeout;

    uint8_t buffer[BUFFER_SIZE];
    uint32_t bufferPosition;

    // Functions
    // void initArduino(void);
    void initArrays(void);
    void initObjects(void);
    void initVariables(void);
};
