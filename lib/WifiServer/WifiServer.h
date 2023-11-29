#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include "ADS1299.h"
#include "OpenBCI_Wifi_Definitions.h"
#include "Config.h"

extern ADS1299 ads1299;
extern AsyncWebServer server;
extern WiFiClass WiFi;
extern WiFiUDP clientUDP;
extern WiFiClient clientTCP;

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1440
#endif

#define PIN_LED 38
#define WIFI_SSID "your-ssid"
#define WIFI_PASSWD "your-password"

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
    boolean noBodyInParam(AsyncWebServerRequest *request);
    void debugPrintDelete(AsyncWebServerRequest *request);
    void debugPrintGet(AsyncWebServerRequest *request);
    void debugPrintPost(AsyncWebServerRequest *request);
    void sendHeadersForCORS(AsyncWebServerRequest *request);
    void sendHeadersForOptions(AsyncWebServerRequest *request);
    void serverReturn(AsyncWebServerRequest *, int, String);
    void returnOK(AsyncWebServerRequest *, String); 
    void returnOK(AsyncWebServerRequest *);
    void returnNoSPIMaster(AsyncWebServerRequest *request);
    void returnNoBodyInPost(AsyncWebServerRequest *request);
    void returnMissingRequiredParam(AsyncWebServerRequest *request, const char *err);
    void returnFail(AsyncWebServerRequest *request, int code, String msg);
    JsonObject& getRequestParams(AsyncWebServerRequest *request);
    void requestWifiManagerStart(AsyncWebServerRequest *request);
    JsonObject& getArgFromArgs(AsyncWebServerRequest *request, int args);
    JsonObject& getArgFromArgs(AsyncWebServerRequest *request);
    void setLatency(AsyncWebServerRequest *request);
    void passthroughCommand(AsyncWebServerRequest *request);
    void tcpSetup(AsyncWebServerRequest *request);
    void udpSetup(AsyncWebServerRequest *request);
    void removeWifiAPInfo(void);

    ~WifiServer();

private:
    // Variables
    size_t _jsonBufferSize;

    uint8_t _gains[MAX_CHANNELS];
    uint8_t curNumChannels;

    unsigned long _counter;
    unsigned long _latency;
    unsigned long _ntpOffset;

    ADS1299 &_ads1299;
    HardwareSerial &_serial;
    AsyncWebServer &_server;
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
