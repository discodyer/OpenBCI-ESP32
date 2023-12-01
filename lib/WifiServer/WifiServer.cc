#include <WiFi.h>
#include "WifiServer.h"

WifiServer::WifiServer()
    : ledState(false), startWifiManager(false), tryConnectToAP(false), underSelfTest(false),
      wifiReset(false), lastHeadMove(0), lastSendToClient(0), ledFlashes(0), ledInterval(300),
      ledLastFlash(millis()), wifiConnectTimeout(millis()), jsonStr(""), bufferPosition(0),
      buffer{}, _serial(Serial0), _ads1299(ads1299), _server(server), _WiFi(WiFi),
      curPacketType(PACKET_TYPE_ACCEL), curTimeSyncMode(TIME_SYNC_MODE_OFF),
      curAccelMode(ACCEL_MODE_OFF), currentChannelSetting(0), optionalArgBuffer7{},
      newMarkerReceived(false), markerValue(0)
{
}

/// @brief The function that the radio will call in setup()
/// @param
void WifiServer::begin(void)
{
    initVariables();
    initArrays();
    initObjects();
    if (connectToWiFi(WIFI_SSID, WIFI_PASSWD))
    {
        printWifiStatus();
        startWebServer();
    }
}

void WifiServer::printWifiStatus()
{
    // print the SSID of the network you're attached to:
    _serial.print("SSID: ");
    _serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    _serial.print("IP Address: ");
    _serial.println(ip);
}

#ifdef RAW_TO_JSON
void WifiServer::channelDataCompute(uint8_t *arr, uint8_t *gains, Sample *sample, uint8_t packetOffset, uint8_t numChannels)
{
    const uint8_t byteOffset = 2;
    if (packetOffset == 0)
    {
        sample->timestamp = getTime();
        for (uint8_t i = 0; i < numChannels; i++)
        {
            sample->channelData[i] = 0.0;
        }
        sample->sampleNumber = arr[1];
    }

    if (numChannels == NUM_CHANNELS_GANGLION)
    {
        int32_t temp_raw[MAX_CHANNELS_PER_PACKET];
        // for (int i = 0; i < 24; i++) {
        //   _serial.printf("arr[%d]: %d\n", i+2, arr[i+2]);
        // }
        extractRaws(arr + 2, temp_raw, NUM_CHANNELS_GANGLION);
        transformRawsToScaledGanglion(temp_raw, sample->channelData);
    }
    else
    {
        int32_t temp_raw[MAX_CHANNELS_PER_PACKET];
        extractRaws(arr + 2, temp_raw, MAX_CHANNELS_PER_PACKET);
        transformRawsToScaledCyton(temp_raw, gains, packetOffset, sample->channelData);
    }
}
#endif

/// @brief Used to print out a long long number. Forces the base to be DEC
/// @param n    {uint64_t} The unsigned number
void WifiServer::debugPrintLLNumber(long long n)
{
    debugPrintLLNumber(n, DEC);
}

/// @brief Used to print out a long long number
/// @param n    {uint64_t} The signed number
/// @param base {uint8_t} The base you want to print in. DEC, HEX, BIN
void WifiServer::debugPrintLLNumber(long long n, uint8_t base)
{
    _serial.print(getStringLLNumber(n, base));
}

/// @brief Used to print out a long long number. Forces the base to be DEC
/// @param n    {uint64_t} The unsigned number
void WifiServer::debugPrintLLNumber(unsigned long long n)
{
    debugPrintLLNumber(n, DEC);
}

/// @brief Used to print out a long long number
/// @param n    {uint64_t} The signed number
/// @param base {uint8_t} The base you want to print in. DEC, HEX, BIN
void WifiServer::debugPrintLLNumber(unsigned long long n, uint8_t base)
{
    _serial.print(getStringLLNumber(n, base));
}

/// @brief Should extract raw int32_t array of length number of channels to get
/// @param arr          {uint8_t *} The array of 3byte signed 2's complement numbers
/// @param output       {int32_t *} An array of length `numChannels` to store
///                     the extracted values
/// @param numChannels  {uint8_t} The number of channels to pull out of `arr`
void WifiServer::extractRaws(uint8_t *arr, int32_t *output, uint8_t numChannels)
{
    for (uint8_t i = 0; i < numChannels; i++)
    {
        output[i] = int24To32(arr + (i * 3));
        // _serial.printf("%d\n", output[i]);
    }
}

void WifiServer::gainReset(void)
{
    for (uint8_t i = 0; i < MAX_CHANNELS; i++)
    {
        _gains[i] = 0;
    }
}

// todo!
/// @brief Used to get a pretty description of the board connected to the shield
/// @param numChannels  {uint8_t} - The number of channels
/// @return             {String} - The string representation of given `numChannels`
String WifiServer::getBoardTypeString(uint8_t numChannels)
{
    switch (numChannels)
    {
    case NUM_CHANNELS_CYTON_DAISY:
        return BOARD_TYPE_CYTON_DAISY;
    case NUM_CHANNELS_CYTON:
        return BOARD_TYPE_CYTON;
    case NUM_CHANNELS_GANGLION:
        return BOARD_TYPE_GANGLION;
    default:
        return BOARD_TYPE_NONE;
    }
}

/// @brief Use the internal `curNumChannels` value to get current board type.
/// @return {String} - A stringified version of the number of the board
///         type based on number of channels.
String WifiServer::getCurBoardTypeString()
{
    return getBoardTypeString(getNumChannels());
}

String WifiServer::getCurOutputModeString()
{
    return getOutputModeString(curOutputMode);
}

String WifiServer::getCurOutputProtocolString()
{
    return getOutputProtocolString(curOutputProtocol);
}

uint8_t *WifiServer::getGains(void)
{
    for (uint8_t i = 0; i < _ads1299.numChannels; i++)
    {
        _gains[i] = _ads1299.channelSettings[i][1];
    }
    return _gains;
}

// todo!

/// @brief Used to decode coded gain value from Cyton into actual gain
/// @param b {uint8_t} The encoded ADS1299 gain value. 0-6
/// @return  {uint8_t} The decoded gain value.
uint8_t WifiServer::getGainCyton(uint8_t b)
{
    switch (b)
    {
    case CYTON_GAIN_1:
        return 1;
    case CYTON_GAIN_2:
        return 2;
    case CYTON_GAIN_4:
        return 4;
    case CYTON_GAIN_6:
        return 6;
    case CYTON_GAIN_8:
        return 8;
    case CYTON_GAIN_12:
        return 12;
    case CYTON_GAIN_24:
    default:
        return 24;
    }
}

/// @brief Returns the gain for the Ganglion.
/// @param
/// @return {uint8_t} - The gain for the ganglion.
uint8_t WifiServer::getGainGanglion(void)
{
    return 51;
}

/// @brief Returns the current head of the buffer
/// @param
/// @return
uint8_t WifiServer::getHead(void)
{
    return head;
}

String WifiServer::getInfoAll(void)
{
    const size_t argBufferSize = JSON_OBJECT_SIZE(8) + 135;
    DynamicJsonDocument JsonDoc(argBufferSize);
    JsonObject root = JsonDoc.to<JsonObject>();

    root[JSON_BOARD_CONNECTED] = (bool)spiHasMaster();
    // _serial.println("spiHasMaster: "); _serial.println(spiHasMaster() ? "true" : "false");
    root[JSON_HEAP] = ESP.getFreeHeap();
    root[JSON_TCP_IP] = WiFi.localIP().toString();
    root[JSON_MAC] = getMac();
    root[JSON_NAME] = getName();
    root[JSON_NUM_CHANNELS] = getNumChannels();
    root[JSON_VERSION] = getVersion();
    root[JSON_LATENCY] = getLatency();

    String output;
    serializeJson(root, output);
    return output;
}

String WifiServer::getInfoBoard(void)
{
    const size_t argBufferSize = JSON_OBJECT_SIZE(4) + 150 + JSON_ARRAY_SIZE(getNumChannels());
    DynamicJsonDocument jsonDoc(argBufferSize);
    jsonDoc[JSON_BOARD_CONNECTED] = (bool)spiHasMaster();
    // _serial.println("spiHasMaster: "); _serial.println(spiHasMaster() ? "true" : "false");
    jsonDoc[JSON_BOARD_TYPE] = getCurBoardTypeString();
    jsonDoc[JSON_NUM_CHANNELS] = getNumChannels();

    JsonArray gainsArr = jsonDoc.createNestedArray(JSON_GAINS);
    getGains(); // update gains
    for (uint8_t i = 0; i < getNumChannels(); i++)
    {
        gainsArr.add(getGainCyton(_gains[i] >> 4));
    }
    String output;
    serializeJson(jsonDoc, output);
    return output;
}

#ifdef MQTT
String WifiServer::getInfoMQTT(boolean clientMQTTConnected)
{
    const size_t bufferSize = JSON_OBJECT_SIZE(7) + 2000;
    StaticJsonDocument<bufferSize> jsonDoc;

    jsonDoc[JSON_MQTT_BROKER_ADDR] = String(mqttBrokerAddress);
    jsonDoc[JSON_CONNECTED] = clientMQTTConnected ? true : false;
    jsonDoc[JSON_MQTT_USERNAME] = String(mqttUsername);
    jsonDoc[JSON_TCP_OUTPUT] = getCurOutputModeString();
    jsonDoc[JSON_LATENCY] = getLatency();
    jsonDoc[JSON_MQTT_PORT] = mqttPort;

    String json;
    serializeJson(jsonDoc, json);
    return json;
}
#endif

String WifiServer::getInfoTCP(boolean clientTCPConnected)
{
    const size_t bufferSize = JSON_OBJECT_SIZE(6) + 40 * 6;
    StaticJsonDocument<bufferSize> jsonDoc;

    jsonDoc[JSON_CONNECTED] = clientTCPConnected ? true : false;
    jsonDoc[JSON_TCP_DELIMITER] = tcpDelimiter ? true : false;
    jsonDoc[JSON_TCP_IP] = tcpAddress.toString();
    jsonDoc[JSON_TCP_OUTPUT] = getCurOutputModeString();
    jsonDoc[JSON_TCP_PORT] = tcpPort;
    jsonDoc[JSON_LATENCY] = getLatency();

    String json;
    serializeJson(jsonDoc, json);
    return json;
}

/// @brief The additional bytes needed for input duplication, follows max packets
/// @param
/// @return
int WifiServer::getJSONAdditionalBytes(uint8_t numChannels)
{
    const int extraDoubleSpace = 100;
    const int extraLongLongSpace = 50;
    switch (numChannels)
    {
    case NUM_CHANNELS_GANGLION:
        return 1014 + extraDoubleSpace + extraLongLongSpace;
    case NUM_CHANNELS_CYTON_DAISY:
        return 1062 + extraDoubleSpace + extraLongLongSpace;
    case NUM_CHANNELS_CYTON:
    default:
        return 966 + extraDoubleSpace + extraLongLongSpace;
    }
}

size_t WifiServer::getJSONBufferSize(void)
{
    return _jsonBufferSize;
}

#ifdef RAW_TO_JSON
/// @brief Used to pack Samples into a single JSON chunk to be sent out to client.
/// @param  numChannels {uint8_t} - The number of channels per sample
/// @param  numSamples  {uint8_t} - The number of samples to pack , should not
///                     exceed the `::getJSONMaxPackets()` for `numChannels`.
/// @param              {String} - Strinigified version of the "chunk"
void WifiServer::getJSONFromSamples(JsonObject &root, uint8_t numChannels, uint8_t numPackets)
{
    JsonArray &chunk = root.createNestedArray("chunk");

    root["count"] = _counter++;

    for (uint8_t i = 0; i < numPackets; i++)
    {
        if (tail >= NUM_PACKETS_IN_RING_BUFFER_JSON)
        {
            tail = 0;
        }
        JsonObject &sample = chunk.createNestedObject();

        if (jsonHasTimeStamps)
            sample["timestamp"] = (sampleBuffer + tail)->timestamp;
        if (jsonHasSampleNumbers)
            sample["sampleNumber"] = (sampleBuffer + tail)->sampleNumber;

        JsonArray &data = sample.createNestedArray("data");

        for (uint8_t j = 0; j < numChannels; j++)
        {
            if ((sampleBuffer + tail)->channelData[j] < 0)
            {
                data.add((long long)(sampleBuffer + tail)->channelData[j]);
            }
            else
            {
                data.add((unsigned long long)(sampleBuffer + tail)->channelData[j]);
            }
        }
        tail++;
    }
}
#endif

uint8_t WifiServer::getJSONMaxPackets(void)
{
    return getJSONMaxPackets(getNumChannels());
}

/// @brief We want to max the size out to < 2000bytes per json chunk
/// @param numChannels
/// @return
uint8_t WifiServer::getJSONMaxPackets(uint8_t numChannels)
{
    switch (numChannels)
    {
    case NUM_CHANNELS_GANGLION:
        return 10; // Size of
    case NUM_CHANNELS_CYTON_DAISY:
        return 6;
    case NUM_CHANNELS_CYTON:
    default:
        return 10;
    }
}

void WifiServer::startWebServer(void)
{
#ifdef DEBUG
    _serial.printf("Starting SSDP...\n");
#endif
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("PTW - OpenBCI Wifi Shield");
    SSDP.setSerialNumber(getName());
    SSDP.setURL("index.html");
    SSDP.setModelName(getModelNumber());
    SSDP.setModelNumber("929000226503");
    SSDP.setModelURL("http://www.openbci.com");
    SSDP.setManufacturer("Push The World LLC");
    SSDP.setManufacturerURL("http://www.pushtheworldllc.com");
    SSDP.setInterval(1000);
    SSDP.setDeviceType("Basic");
    SSDP.begin();

#ifdef DEBUG
    // printWifiStatus();
    _serial.printf("Starting HTTP...\n");
#endif
    _server.on(HTTP_ROUTE, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
    String ip = "10.0.0.1";
    String out = "<!DOCTYPE html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><html lang=\"en\"><h1 style=\"margin:  auto;width: 90%;text-align: center;\">Push The World</h1><br>";
    if (WiFi.localIP().toString().equals("10.0.0.1") || WiFi.localIP().toString().equals("0.0.0.0")) {
      if (WiFi.SSID().equals("")) {
        out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";
        out += "10.0.0.1";
        out += HTTP_ROUTE_WIFI_CONFIG;
        out += "'>Click to Configure Wifi</a><br>If the above link does not work type 10.0.0.1/wifi in web browser and press Enter or Go.<br>See updates on issue <a href='https://github.com/OpenBCI/OpenBCI_WIFI/issues/62'>#62</a> on Github.</p><br>";
      } else {
        out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";
        out += "10.0.0.1";
        out += HTTP_ROUTE_WIFI_CONFIG;
        out += "'>Click to Configure Wifi</a><br>If the above link does not work type 10.0.0.1/wifi in web browser and press Enter or Go.<br>See updates on issue <a href='https://github.com/OpenBCI/OpenBCI_WIFI/issues/62'>#62</a> on Github.</p><br>";
        out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";
        out += "10.0.0.1";
        out += HTTP_ROUTE_WIFI_DELETE;
        out += "'>Click to Erase Wifi Credentials</a></p><br>";
      }
    } else {
      out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";
      out += WiFi.localIP().toString();
      out += HTTP_ROUTE_WIFI_CONFIG;
      out += "'>Click to Configure Wifi</a><br>If the above link does not work type ";
      out += WiFi.localIP().toString();
      out += "/wifi in web browser and press Enter or Go.<br>See updates on issue <a href='https://github.com/OpenBCI/OpenBCI_WIFI/issues/62'>#62</a> on Github.</p><br>";
      out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";
      out += WiFi.localIP().toString();
      out += HTTP_ROUTE_WIFI_DELETE;
      out += "'>Click to Erase Wifi Credentials</a></p><br>";
    }
    out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";
    if (WiFi.localIP().toString().equals("10.0.0.1") || WiFi.localIP().toString().equals("0.0.0.0")) {
      out += "10.0.0.1/update";
    } else {
      out += WiFi.localIP().toString();
      out += "/update";
    }
    out += "'>Click to Update WiFi Firmware</a></p><br>";
    out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"> Please visit <a href='https://app.swaggerhub.com/apis/pushtheworld/openbci-wifi-server/2.0.0'>Swaggerhub</a> for the latest HTTP endpoints</p><br>";
    out += "<p style=\"margin:  auto;width: 80%;text-align: center;\"> Shield Firmware: " + String(SOFTWARE_VERSION) + "</p></html>";

    request->send(200, "text/html", out);
     });

    _server.on(HTTP_ROUTE, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });

    _server.on("/description.xml", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
                   _serial.println("SSDP HIT");
#endif
                   // digitalWrite(PIN_LED, LOW);
                   // SSDP.schema(request->client());
                   // 发送响应
                   request->send(200, "text/xml", SSDP.schema(false));
                   // digitalWrite(PIN_LED, HIGH);
               });

    // Add other routes...

    _server.on(HTTP_ROUTE_YT, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
        returnOK(request, "Keep going! Push The World!"); });
    _server.on(HTTP_ROUTE_YT, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });

    _server.on(HTTP_ROUTE_TCP, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
                   debugPrintGet(request);
#endif
                   String out = getInfoTCP(clientTCP.connected());

                   // _server.setContentLength(out.length());
                   // _server.send(200, "application/json", out.c_str());
                //    sendHeadersForCORS(request, 200, "application/json", out);
                   request->send(200, "application/json", out);
               });

    _server.on(HTTP_ROUTE_TCP, HTTP_POST, [this](AsyncWebServerRequest *request)
               { tcpSetup(request); });
    _server.on(HTTP_ROUTE_TCP, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_TCP, HTTP_DELETE, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintDelete(request);
#endif
    clientTCP.stop();
    setOutputProtocol(OUTPUT_PROTOCOL_NONE);
    jsonStr = getInfoTCP(false);
    // server.setContentLength(jsonStr.length());
    // server.send(200, "text/json", jsonStr.c_str());
    // sendHeadersForCORS(request, 200, "text/json", jsonStr);

    request->send(200, "text/json", jsonStr);
    jsonStr = ""; });

    _server.on(HTTP_ROUTE_UDP, HTTP_POST, [this](AsyncWebServerRequest *request)
               { udpSetup(request); });
    _server.on(HTTP_ROUTE_UDP, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_UDP, HTTP_DELETE, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintDelete(request);
#endif
    setOutputProtocol(OUTPUT_PROTOCOL_NONE);
    jsonStr = getInfoTCP(false);
    // server.setContentLength(jsonStr.length());
    // server.send(200, RETURN_TEXT_JSON, jsonStr.c_str());
    request->send(200, RETURN_TEXT_JSON, jsonStr);
    // sendHeadersForCORS(request, 200, RETURN_TEXT_JSON, jsonStr);
    jsonStr = ""; });
    // These could be helpful...
    _server.on(HTTP_ROUTE_STREAM_START, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
    // if (!spiHasMaster()) return returnNoSPIMaster(request);
    passthroughCommands("b");
    // SPISlave.setData(wifi.passthroughBuffer, BYTES_PER_SPI_PACKET);
    returnOK(request); });

    _server.on(HTTP_ROUTE_STREAM_START, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_STREAM_STOP, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
    // if (!spiHasMaster()) return returnNoSPIMaster();
    passthroughCommands("s");
    // SPISlave.setData(wifi.passthroughBuffer, BYTES_PER_SPI_PACKET);
    returnOK(request); });

    _server.on(HTTP_ROUTE_STREAM_STOP, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_VERSION, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
    returnOK(request, getVersion()); });

    _server.on(HTTP_ROUTE_VERSION, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });

    _server.on(HTTP_ROUTE_COMMAND, HTTP_POST, [this](AsyncWebServerRequest *request)
               { passthroughCommand(request); });
    _server.on(HTTP_ROUTE_COMMAND, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_LATENCY, HTTP_GET, [this](AsyncWebServerRequest *request)
               { returnOK(request, String(getLatency())); });
    _server.on(HTTP_ROUTE_LATENCY, HTTP_POST, [this](AsyncWebServerRequest *request)
               { setLatency(request); });
    _server.on(HTTP_ROUTE_LATENCY, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });

    // get heap status, analog input value and all GPIO statuses in one json call
    _server.on(HTTP_ROUTE_ALL, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
    String output = getInfoAll();
    // server.setContentLength(output.length());
    // server.send(200, RETURN_TEXT_JSON, output);
    request->send(200, RETURN_TEXT_JSON, output);
    // sendHeadersForCORS(request, 200, RETURN_TEXT_JSON, output); 
    });
    _server.on(HTTP_ROUTE_ALL, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_BOARD, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintGet(request);
#endif
    String output = getInfoBoard();
    // server.setContentLength(output.length());
    // server.send(200, RETURN_TEXT_JSON, output);
    request->send(200, RETURN_TEXT_JSON, output);
    // sendHeadersForCORS(request, 200, RETURN_TEXT_JSON, output); });
    // sendHeadersForCORS(request, 200, RETURN_TEXT_JSON, output); 
    });
    _server.on(HTTP_ROUTE_BOARD, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_WIFI, HTTP_GET, [this](AsyncWebServerRequest *request)
               { requestWifiManagerStart(request); });
    _server.on(HTTP_ROUTE_WIFI, HTTP_DELETE, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintDelete(request);
#endif
    returnOK(request, "Reseting wifi. Please power cycle your board in 10 seconds");
    wifiReset = true; });
    _server.on(HTTP_ROUTE_WIFI, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });

    _server.on(HTTP_ROUTE_WIFI_CONFIG, HTTP_GET, [this](AsyncWebServerRequest *request)
               { requestWifiManagerStart(request); });
    _server.on(HTTP_ROUTE_WIFI_CONFIG, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });
    _server.on(HTTP_ROUTE_WIFI_DELETE, HTTP_GET, [this](AsyncWebServerRequest *request)
               {
#ifdef DEBUG
    debugPrintDelete(request);
#endif
    returnOK(request, "Reseting wifi. Please power cycle your board in 10 seconds");
    wifiReset = true;
    digitalWrite(PIN_LED, LOW); });
    _server.on(HTTP_ROUTE_WIFI_DELETE, HTTP_OPTIONS, [this](AsyncWebServerRequest *request)
               { sendHeadersForOptions(request); });

    if (!MDNS.begin(getName().c_str()))
    {
#ifdef DEBUG
        _serial.println("Error setting up MDNS responder!");
#endif
    }
    else
    {
#ifdef DEBUG
        _serial.print("Your ESP is called ");
        _serial.println(getName());
#endif
    }

    // Set up not found route
    _server.onNotFound([this](AsyncWebServerRequest *request)
                       {
#ifdef DEBUG
    _serial.printf("HTTP NOT FOUND :%s", request->url());
#endif
    request->send(404, "text/plain", "404 Not Found");
     });
    // Start the server
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    _server.begin();

    MDNS.addService("http", "tcp", 80);

#ifdef DEBUG
    _serial.println("WebServer Ready!");
#endif
    ledFlashes = 10;
    ledInterval = 100;
    ledLastFlash = millis();
    ledState = false;
}

boolean WifiServer::connectToWiFi(const char *ssid, const char *password)
{
    _serial.println("Connecting to WiFi...");
    _WiFi.mode(WIFI_STA);
    _WiFi.begin(ssid, password);

    int attempts = 0, maxRetry = 3;
    while (WiFi.status() != WL_CONNECTED && attempts < maxRetry)
    {
        delay(1000);
        Serial0.printf("Connection failed. Retrying %d of %d...\n", attempts, maxRetry);
        attempts++;
    }

    if (_WiFi.status() == WL_CONNECTED)
    {
        _serial.println("Connected to WiFi");
        _serial.printf("IP address: %s \n", _WiFi.localIP().toString());
        return true;
    }
    else
    {
        _serial.println("Connection failed.");
        return false;
    }
}

boolean WifiServer::noBodyInParam(AsyncWebServerRequest *request)
{
    return request->args() == 0;
}

void WifiServer::debugPrintDelete(AsyncWebServerRequest *request)
{
    _serial.printf("HTTP DELETE %s HEAP: %u\n", request->url().c_str(), ESP.getFreeHeap());
}

void WifiServer::debugPrintGet(AsyncWebServerRequest *request)
{
    _serial.printf("HTTP GET %s HEAP: %u\n", request->url().c_str(), ESP.getFreeHeap());
}

void WifiServer::debugPrintPost(AsyncWebServerRequest *request)
{
    _serial.printf("HTTP POST %s HEAP: %u\n", request->url().c_str(), ESP.getFreeHeap());
}

void WifiServer::sendHeadersForCORS(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void WifiServer::sendHeadersForCORS(AsyncWebServerRequest *request, int code, const String &contentType, const String &content)
{
    request->send(addHeadersForCORS(request, code, contentType, content));
}

AsyncWebServerResponse *WifiServer::addHeadersForCORS(AsyncWebServerRequest *request, int code, const String &contentType, const String &content)
{
    AsyncWebServerResponse *response = request->beginResponse(code, contentType, content);
    response->addHeader("Access-Control-Allow-Origin", "*");
    return response;
}

void WifiServer::sendHeadersForOptions(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "POST,DELETE,GET,OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
}

void WifiServer::serverReturn(AsyncWebServerRequest *request, int code, String s)
{
    digitalWrite(PIN_LED, LOW);
    // AsyncWebServerResponse *response = request->beginResponse(code, "text/plain", s + "\r\n");
    // request->send(response);
    request->send(code, "text/plain", s + "\r\n");
    // sendHeadersForCORS(request, code, "text/plain", s + "\r\n");
    digitalWrite(PIN_LED, HIGH);
}

void WifiServer::returnOK(AsyncWebServerRequest *request, String s)
{
    serverReturn(request, 200, s);
}

void WifiServer::returnOK(AsyncWebServerRequest *request)
{
    returnOK(request, "OK");
}

/// @brief Used to send a response to the client that the board is not attached.
/// @param
void WifiServer::returnNoSPIMaster(AsyncWebServerRequest *request)
{
    if (lastTimeWasPolled < 1)
    {
        serverReturn(request, SPI_NO_MASTER, "Error: No OpenBCI board attached");
    }
    else
    {
        serverReturn(request, SPI_TIMEOUT_CLIENT_RESPONSE, "Error: Lost communication with OpenBCI board");
    }
}

/// @brief Used to send a response to the client that there is no body in the post request.
/// @param request
void WifiServer::returnNoBodyInPost(AsyncWebServerRequest *request)
{
    serverReturn(request, CLIENT_RESPONSE_NO_BODY_IN_POST, "Error: No body in POST request");
}

/// @brief Return if there is a missing param in the required command
/// @param request
/// @param err
void WifiServer::returnMissingRequiredParam(AsyncWebServerRequest *request, const char *err)
{
    serverReturn(request, CLIENT_RESPONSE_MISSING_REQUIRED_CMD, String(err));
}

void WifiServer::returnFail(AsyncWebServerRequest *request, int code, String msg)
{
    serverReturn(request, code, msg);
}

JsonObject &WifiServer::getArgFromArgs(AsyncWebServerRequest *request, int args)
{
    DynamicJsonDocument jsonDoc(JSON_OBJECT_SIZE(args) + (40 * args));

    // 使用 deserializeJson 函数解析 JSON 字符串
    DeserializationError error = deserializeJson(jsonDoc, request->arg((size_t)0));

    // 检查是否解析成功
    if (error)
    {
        _serial.print("Failed to parse JSON: ");
        _serial.println(error.c_str());
    }

    _root = jsonDoc.to<JsonObject>();
    return _root;
}

JsonObject &WifiServer::getArgFromArgs(AsyncWebServerRequest *request)
{
    return getArgFromArgs(request, 1);
}

void WifiServer::requestWifiManagerStart(AsyncWebServerRequest *request)
{
#ifdef DEBUG
    debugPrintGet(request);
#endif
    // 构建 HTML 响应
    String out = "<!DOCTYPE html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><html lang=\"en\"><h1 style=\"margin:  auto;width: 80%;text-align: center;\">Push The World</h1><br><p style=\"margin:  auto;width: 80%;text-align: center;\"><a href='http://";

    // 获取本地 IP 地址
    if (WiFi.localIP().toString().equals("10.0.0.1") || WiFi.localIP().toString().equals("0.0.0.0"))
    {
        out += "10.0.0.1";
    }
    else
    {
        out += WiFi.localIP().toString();
    }

    // 拼接路由
    out += HTTP_ROUTE;
    out += "'>Click to Go To WiFi Manager</a></p><html>";

    // 发送 HTML 响应
    request->send(200, "text/html", out);
    // sendHeadersForCORS(request, 200, "text/html", out);

    // 设置 LED 闪烁
    ledFlashes = 5;
    ledInterval = 250;
    ledLastFlash = millis();

    startWifiManager = true;
}

/// @brief Used to set the latency of the system.
/// @param request
void WifiServer::setLatency(AsyncWebServerRequest *request)
{
    if (noBodyInParam(request))
    {
        return returnNoBodyInPost(request);
    }

    JsonObject &root = getArgFromArgs(request);

    // 检查是否包含 JSON_LATENCY 键
    if (root.containsKey(JSON_LATENCY) && root[JSON_LATENCY].is<unsigned long>())
    {
        // 从 JSON 对象中获取 JSON_LATENCY 的值，并设置 Latency
        setLatency(static_cast<unsigned long>(root[JSON_LATENCY]));
        returnOK(request);
    }
    else
    {
        returnMissingRequiredParam(request, JSON_LATENCY);
    }
}

WifiServer::~WifiServer()
{
}

void WifiServer::passthroughCommand(AsyncWebServerRequest *request)
{
    if (noBodyInParam(request))
    {
        return returnNoBodyInPost(request);
    }
    if (spiHasMaster())
    {
        return returnNoSPIMaster(request);
    }

    JsonObject &root = getArgFromArgs(request);
#ifdef DEBUG
    // root.printTo(_serial);
#endif

    if (root.containsKey(JSON_COMMAND))
    {
        String cmds = root[JSON_COMMAND];
        uint8_t retVal = passthroughCommands(cmds);
        if (retVal < PASSTHROUGH_PASS)
        {
            switch (retVal)
            {
            case PASSTHROUGH_FAIL_TOO_MANY_CHARS:
                return returnFail(request, 501, "Error: Sent more than 31 chars");
            case PASSTHROUGH_FAIL_NO_CHARS:
                return returnFail(request, 505, "Error: No characters found for key 'command'");
            case PASSTHROUGH_FAIL_QUEUE_FILLED:
                return returnFail(request, 503, "Error: Queue is full, please wait 20ms and try again.");
            default:
                return returnFail(request, 504, "Error: Unknown error");
            }
        }
        return;
    }
    else
    {
        return returnMissingRequiredParam(request, JSON_COMMAND);
    }
}

void WifiServer::tcpSetup(AsyncWebServerRequest *request)
{
#ifdef DEBUG
    debugPrintGet(request);
#endif
    // Parse args
    if (noBodyInParam(request))
    {
        return returnNoBodyInPost(request); // no body
    }
    JsonObject &root = getArgFromArgs(request, 8);
    if (!root.containsKey(JSON_TCP_IP))
    {
        return returnMissingRequiredParam(request, JSON_TCP_IP);
    }
    String tempAddr = root[JSON_TCP_IP];
    IPAddress tempIPAddr;
    if (!tempIPAddr.fromString(tempAddr))
    {
        return returnFail(request, 505, "Error: unable to parse ip address. Please send as string in octets i.e. xxx.xxx.xxx.xxx");
    }
    if (!root.containsKey(JSON_TCP_PORT))
    {
        return returnMissingRequiredParam(request, JSON_TCP_PORT);
    }
    int port = root[JSON_TCP_PORT];
    if (root.containsKey(JSON_TCP_OUTPUT))
    {
        String outputModeStr = root[JSON_TCP_OUTPUT];
        if (outputModeStr.equals(getOutputModeString(OUTPUT_MODE_RAW)))
        {
            setOutputMode(OUTPUT_MODE_RAW);
        }
        else if (outputModeStr.equals(getOutputModeString(OUTPUT_MODE_JSON)))
        {
            setOutputMode(OUTPUT_MODE_JSON);
        }
        else
        {
            return returnFail(request, 506, "Error: '" + String(JSON_TCP_OUTPUT) + "' must be either " + getOutputModeString(OUTPUT_MODE_RAW) + " or " + getOutputModeString(OUTPUT_MODE_JSON));
        }
#ifdef DEBUG
        _serial.print("Set output mode to ");
        _serial.println(getCurOutputModeString());
#endif
    }

    if (root.containsKey(JSON_REDUNDANCY))
    {
        redundancy = root[JSON_REDUNDANCY];
#ifdef DEBUG
        _serial.print("Set redundancy to ");
        _serial.println(redundancy);
#endif
    }

    if (root.containsKey(JSON_LATENCY))
    {
        int latency = root[JSON_LATENCY];
        setLatency(latency);
#ifdef DEBUG
        _serial.print("Set latency to ");
        _serial.print(getLatency());
        _serial.println(" uS");
#endif
    }

    boolean _tcpDelimiter = this->tcpDelimiter;
    if (root.containsKey(JSON_TCP_DELIMITER))
    {
        _tcpDelimiter = root[JSON_TCP_DELIMITER];
#ifdef DEBUG
        _serial.print("Will use delimiter:");
        _serial.println(_tcpDelimiter ? "true" : "false");
#endif
    }
    setInfoTCP(tempAddr, port, _tcpDelimiter);

    if (root.containsKey(JSON_SAMPLE_NUMBERS))
    {
        jsonHasSampleNumbers = root[JSON_SAMPLE_NUMBERS];
#ifdef DEBUG
        _serial.print("Set jsonHasSampleNumbers to ");
        _serial.println(jsonHasSampleNumbers ? String("true") : String("false"));
#endif
    }

    if (root.containsKey(JSON_TIMESTAMPS))
    {
        jsonHasTimeStamps = root[JSON_TIMESTAMPS];
#ifdef DEBUG
        _serial.print("Set jsonHasTimeStamps to ");
        _serial.println(jsonHasTimeStamps ? String("true") : String("false"));
#endif
    }

#ifdef DEBUG
    _serial.print("Got ip: ");
    _serial.println(tcpAddress.toString());
    _serial.print("Got port: ");
    _serial.println(tcpPort);
    _serial.print("Current url: ");
    _serial.println(request->url());
    _serial.print("Starting socket to host: ");
    _serial.print(tcpAddress.toString());
    _serial.print(" on port: ");
    _serial.println(tcpPort);
#endif

    setOutputProtocol(OUTPUT_PROTOCOL_TCP);

    // const size_t bufferSize = JSON_OBJECT_SIZE(6) + 40*6;
    // DynamicJsonBuffer jsonBuffer(bufferSize);
    // JsonObject& rootOut = jsonBuffer.createObject();
    if (clientTCP.connect(tcpAddress, tcpPort))
    {
#ifdef DEBUG
        _serial.println("Connected to server");
#endif
        clientTCP.setNoDelay(1);
        jsonStr = getInfoTCP(true);
        // jsonStr = "";
        // rootOut.printTo(jsonStr);
        // server.setContentLength(jsonStr.length());
        request->send(200, RETURN_TEXT_JSON, jsonStr);
        // sendHeadersForCORS(request, 200, RETURN_TEXT_JSON, jsonStr);
    }
    else
    {
#ifdef DEBUG
        _serial.println("Failed to connect to server");
#endif
        jsonStr = getInfoTCP(false);
        // jsonStr = "";
        // rootOut.printTo(jsonStr);
        // server.setContentLength(jsonStr.length());
        request->send(504, RETURN_TEXT_JSON, jsonStr);
        // sendHeadersForCORS(request, 200, RETURN_TEXT_JSON, jsonStr);
    }
}

void WifiServer::udpSetup(AsyncWebServerRequest *request)
{
#ifdef DEBUG
    debugPrintGet(request);
#endif
    // Parse args
    if (noBodyInParam(request))
    {
        return returnNoBodyInPost(request); // no body
    }
    JsonObject &root = getArgFromArgs(request, 7);
    if (!root.containsKey(JSON_TCP_IP))
    {
        return returnMissingRequiredParam(request, JSON_TCP_IP);
    }
    String tempAddr = root[JSON_TCP_IP];
    IPAddress tempIPAddr;
    if (!tempIPAddr.fromString(tempAddr))
    {
        return returnFail(request, 505, "Error: unable to parse ip address. Please send as string in octets i.e. xxx.xxx.xxx.xxx");
    }
    if (!root.containsKey(JSON_TCP_PORT))
    {
        return returnMissingRequiredParam(request, JSON_TCP_PORT);
    }
    int port = root[JSON_TCP_PORT];

    setOutputMode(OUTPUT_MODE_RAW);
    if (root.containsKey(JSON_TCP_OUTPUT))
    {
        String outputModeStr = root[JSON_TCP_OUTPUT];
        if (outputModeStr.equals(getOutputModeString(OUTPUT_MODE_RAW)))
        {
            // No need to change output mode since it's already set to RAW
        }
        else if (outputModeStr.equals(getOutputModeString(OUTPUT_MODE_JSON)))
        {
            setOutputMode(OUTPUT_MODE_JSON);
        }
        else
        {
            return returnFail(request, 506, "Error: '" + String(JSON_TCP_OUTPUT) + "' must be either " + getOutputModeString(OUTPUT_MODE_RAW) + " or " + getOutputModeString(OUTPUT_MODE_JSON));
        }
#ifdef DEBUG
        _serial.print("Set output mode to ");
        _serial.println(getCurOutputModeString());
#endif
    }

    if (root.containsKey(JSON_REDUNDANCY))
    {
        redundancy = root[JSON_REDUNDANCY];
#ifdef DEBUG
        _serial.print("Set redundancy to ");
        _serial.println(redundancy);
#endif
    }

    if (root.containsKey(JSON_LATENCY))
    {
        int latency = root[JSON_LATENCY];
        setLatency(latency);
#ifdef DEBUG
        _serial.print("Set latency to ");
        _serial.print(getLatency());
        _serial.println(" uS");
#endif
    }

    boolean _tcpDelimiter = this->tcpDelimiter;
    if (root.containsKey(JSON_TCP_DELIMITER))
    {
        _tcpDelimiter = root[JSON_TCP_DELIMITER];
#ifdef DEBUG
        _serial.print("Will use delimiter:");
        _serial.println(_tcpDelimiter ? "true" : "false");
#endif
    }
    setInfoUDP(tempAddr, port, _tcpDelimiter);

#ifdef DEBUG
    _serial.print("Got ip: ");
    _serial.println(tcpAddress.toString());
    _serial.print("Got port: ");
    _serial.println(tcpPort);
    _serial.print("Current url: ");
    _serial.println(request->url());
    _serial.print("Ready to write to: ");
    _serial.print(tcpAddress.toString());
    _serial.print(" on port: ");
    _serial.println(tcpPort);
#endif

    jsonStr = "";
    request->send(200, "text/json", jsonStr.c_str());
    // sendHeadersForCORS(request, 200, "text/json", jsonStr.c_str());
}

void WifiServer::removeWifiAPInfo(void)
{
    // wifi.curClientResponse = wifi.CLIENT_RESPONSE_OUTPUT_STRING;
    // wifi.outputString = "Forgetting wifi credentials and rebooting";
    // wifi.clientWaitingForResponseFullfilled = true;

#ifdef DEBUG
    // _serial.println(outputString);
    _serial.println("Forgetting wifi credentials and rebooting");
#else
    WiFi.disconnect(true); // Disconnect and clear WiFi credentials
#endif
    delay(1000);
    ESP.restart(); // Restart the ESP32
}

/// @brief Process one char at a time from serial port. This is the main
///        command processor for the OpenBCI system. Considered mission critical for
///        normal operation.
/// @param character {char} - The character to process.
/// @return          {boolean} - `true` if the command was recognized, `false` if not
boolean WifiServer::processChar(char character)
{
    if (checkMultiCharCmdTimer())
    { // we are in a multi char command
        switch (getMultiCharCommand())
        {
        case MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_CHANNEL:
            processIncomingChannelSettings(character);
            break;
        case MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_LEADOFF:
            processIncomingLeadOffSettings(character);
            break;
        case MULTI_CHAR_CMD_SETTINGS_BOARD_MODE:
            processIncomingBoardMode(character);
            break;
        case MULTI_CHAR_CMD_SETTINGS_SAMPLE_RATE:
            processIncomingSampleRate(character);
            break;
        case MULTI_CHAR_CMD_INSERT_MARKER:
            processInsertMarker(character);
            break;
        default:
            break;
        }
    }
    else
    { // Normal...
        switch (character)
        {
        // TURN CHANNELS ON/OFF COMMANDS
        case OPENBCI_CHANNEL_OFF_1:
            _ads1299.streamSafeChannelDeactivate(1);
            break;
        case OPENBCI_CHANNEL_OFF_2:
            _ads1299.streamSafeChannelDeactivate(2);
            break;
        case OPENBCI_CHANNEL_OFF_3:
            _ads1299.streamSafeChannelDeactivate(3);
            break;
        case OPENBCI_CHANNEL_OFF_4:
            _ads1299.streamSafeChannelDeactivate(4);
            break;
        case OPENBCI_CHANNEL_OFF_5:
            _ads1299.streamSafeChannelDeactivate(5);
            break;
        case OPENBCI_CHANNEL_OFF_6:
            _ads1299.streamSafeChannelDeactivate(6);
            break;
        case OPENBCI_CHANNEL_OFF_7:
            _ads1299.streamSafeChannelDeactivate(7);
            break;
        case OPENBCI_CHANNEL_OFF_8:
            _ads1299.streamSafeChannelDeactivate(8);
            break;
        case OPENBCI_CHANNEL_OFF_9:
            _ads1299.streamSafeChannelDeactivate(9);
            break;
        case OPENBCI_CHANNEL_OFF_10:
            _ads1299.streamSafeChannelDeactivate(10);
            break;
        case OPENBCI_CHANNEL_OFF_11:
            _ads1299.streamSafeChannelDeactivate(11);
            break;
        case OPENBCI_CHANNEL_OFF_12:
            _ads1299.streamSafeChannelDeactivate(12);
            break;
        case OPENBCI_CHANNEL_OFF_13:
            _ads1299.streamSafeChannelDeactivate(13);
            break;
        case OPENBCI_CHANNEL_OFF_14:
            _ads1299.streamSafeChannelDeactivate(14);
            break;
        case OPENBCI_CHANNEL_OFF_15:
            _ads1299.streamSafeChannelDeactivate(15);
            break;
        case OPENBCI_CHANNEL_OFF_16:
            _ads1299.streamSafeChannelDeactivate(16);
            break;

        case OPENBCI_CHANNEL_ON_1:
            _ads1299.streamSafeChannelActivate(1);
            break;
        case OPENBCI_CHANNEL_ON_2:
            _ads1299.streamSafeChannelActivate(2);
            break;
        case OPENBCI_CHANNEL_ON_3:
            _ads1299.streamSafeChannelActivate(3);
            break;
        case OPENBCI_CHANNEL_ON_4:
            _ads1299.streamSafeChannelActivate(4);
            break;
        case OPENBCI_CHANNEL_ON_5:
            _ads1299.streamSafeChannelActivate(5);
            break;
        case OPENBCI_CHANNEL_ON_6:
            _ads1299.streamSafeChannelActivate(6);
            break;
        case OPENBCI_CHANNEL_ON_7:
            _ads1299.streamSafeChannelActivate(7);
            break;
        case OPENBCI_CHANNEL_ON_8:
            _ads1299.streamSafeChannelActivate(8);
            break;
        case OPENBCI_CHANNEL_ON_9:
            _ads1299.streamSafeChannelActivate(9);
            break;
        case OPENBCI_CHANNEL_ON_10:
            _ads1299.streamSafeChannelActivate(10);
            break;
        case OPENBCI_CHANNEL_ON_11:
            _ads1299.streamSafeChannelActivate(11);
            break;
        case OPENBCI_CHANNEL_ON_12:
            _ads1299.streamSafeChannelActivate(12);
            break;
        case OPENBCI_CHANNEL_ON_13:
            _ads1299.streamSafeChannelActivate(13);
            break;
        case OPENBCI_CHANNEL_ON_14:
            _ads1299.streamSafeChannelActivate(14);
            break;
        case OPENBCI_CHANNEL_ON_15:
            _ads1299.streamSafeChannelActivate(15);
            break;
        case OPENBCI_CHANNEL_ON_16:
            _ads1299.streamSafeChannelActivate(16);
            break;

        // TEST SIGNAL CONTROL COMMANDS
        case OPENBCI_TEST_SIGNAL_CONNECT_TO_GROUND:
            _ads1299.activateAllChannelsToTestCondition(ADSINPUT_SHORTED, ADSTESTSIG_NOCHANGE, ADSTESTSIG_NOCHANGE);
            break;
        case OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_1X_SLOW:
            _ads1299.activateAllChannelsToTestCondition(ADSINPUT_TESTSIG, ADSTESTSIG_AMP_1X, ADSTESTSIG_PULSE_SLOW);
            break;
        case OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_1X_FAST:
            _ads1299.activateAllChannelsToTestCondition(ADSINPUT_TESTSIG, ADSTESTSIG_AMP_1X, ADSTESTSIG_PULSE_FAST);
            break;
        case OPENBCI_TEST_SIGNAL_CONNECT_TO_DC:
            _ads1299.activateAllChannelsToTestCondition(ADSINPUT_TESTSIG, ADSTESTSIG_AMP_2X, ADSTESTSIG_DCSIG);
            break;
        case OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_2X_SLOW:
            _ads1299.activateAllChannelsToTestCondition(ADSINPUT_TESTSIG, ADSTESTSIG_AMP_2X, ADSTESTSIG_PULSE_SLOW);
            break;
        case OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_2X_FAST:
            _ads1299.activateAllChannelsToTestCondition(ADSINPUT_TESTSIG, ADSTESTSIG_AMP_2X, ADSTESTSIG_PULSE_FAST);
            break;

        // CHANNEL SETTING COMMANDS
        case OPENBCI_CHANNEL_CMD_SET: // This is a multi char command with a timeout
            startMultiCharCmdTimer(MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_CHANNEL);
            numberOfIncomingSettingsProcessedChannel = 1;
            break;

        // LEAD OFF IMPEDANCE DETECTION COMMANDS
        case OPENBCI_CHANNEL_IMPEDANCE_SET:
            startMultiCharCmdTimer(MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_LEADOFF);
            numberOfIncomingSettingsProcessedLeadOff = 1;
            break;

        case OPENBCI_CHANNEL_DEFAULT_ALL_SET: // reset all channel settings to default
            if (!_ads1299.streaming)
            {
                _serial.print("updating channel settings to");
                _serial.print(" default");
            }
            _ads1299.streamSafeSetAllChannelsToDefault();
            break;
        case OPENBCI_CHANNEL_DEFAULT_ALL_REPORT: // report the default settings
            // reportDefaultChannelSettings(); // todo!
            break;

        // DAISY MODULE COMMANDS
        case OPENBCI_CHANNEL_MAX_NUMBER_8: // use 8 channel mode
            if (_ads1299.daisyPresent)
            {
                _ads1299.removeDaisy();
            }
            // else if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringLast("No daisy to remove");
            // }
            else
            {
                _serial.println("No daisy to remove");
            }
            break;
        case OPENBCI_CHANNEL_MAX_NUMBER_16: // use 16 channel mode
            if (_ads1299.daisyPresent == false)
            {
                // _ads1299.attachDaisy();
            }
            if (_ads1299.daisyPresent)
            {
                // printAll("16");
            }
            else
            {
                // printAll("8");
            }
            // sendEOT();
            break;

        // STREAM DATA AND FILTER COMMANDS
        case OPENBCI_STREAM_START: // stream data
            // if (curAccelMode == ACCEL_MODE_ON)
            // {
            //     enable_accel(RATE_25HZ);
            // } // fire up the accelerometer if you want it
            // wifi.tx = commandFromSPI;
            // if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringLast("Stream started");
            //     iSerial0.tx = false;
            // }
            // Reads if the command is not from the SPI port and we are not in debug mode
            // if (!commandFromSPI && !iSerial1.tx)
            // {
            //     // If the sample rate is higher than 250, we need to drop down to 250Hz
            //     //  to not break the RFduino system that can't handle above 250SPS.
            //     if (curSampleRate != SAMPLE_RATE_250)
            //     {
            //         streamSafeSetSampleRate(SAMPLE_RATE_250);
            //         delay(50);
            //     }
            // }
            _ads1299.streamStart(); // turn on the fire hose
            break;
        case OPENBCI_STREAM_STOP: // stop streaming data
            // if (curAccelMode == ACCEL_MODE_ON)
            // {
            //     disable_accel();
            // } // shut down the accelerometer if you're using it
            // wifi.tx = true;
            _ads1299.streamStop();
            // if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringLast("Stream stopped");
            // }
            break;

        //  INITIALIZE AND VERIFY
        case OPENBCI_MISC_SOFT_RESET:
            // boardReset();
            _ads1299.softReset(); // initialize ADS and read device IDs
            break;
        //  QUERY THE ADS AND ACCEL REGITSTERS
        case OPENBCI_MISC_QUERY_REGISTER_SETTINGS:
            if (!_ads1299.streaming)
            {
                // todo!
                // _ads1299.printAllRegisters(); // print the ADS and accelerometer register values
            }
            break;

        // TIME SYNC
        case OPENBCI_TIME_SET:
            // Set flag to send time packet
            if (!_ads1299.streaming)
            {
                // printAll("Time stamp ON");
                // sendEOT();
            }
            curTimeSyncMode = TIME_SYNC_MODE_ON;
            setCurPacketType();
            break;

        case OPENBCI_TIME_STOP:
            // Stop the Sync
            if (!_ads1299.streaming)
            {
                // printAll("Time stamp OFF");
                // sendEOT();
            }
            curTimeSyncMode = TIME_SYNC_MODE_OFF;
            setCurPacketType();
            break;

        // BOARD TYPE SET TYPE
        case OPENBCI_BOARD_MODE_SET:
            startMultiCharCmdTimer(MULTI_CHAR_CMD_SETTINGS_BOARD_MODE);
            optionalArgCounter = 0;
            break;

        // Sample rate set
        case OPENBCI_SAMPLE_RATE_SET:
            startMultiCharCmdTimer(MULTI_CHAR_CMD_SETTINGS_SAMPLE_RATE);
            break;

        // Insert Marker into the EEG data stream
        case OPENBCI_INSERT_MARKER:
            startMultiCharCmdTimer(MULTI_CHAR_CMD_INSERT_MARKER);
            break;

        case OPENBCI_WIFI_ATTACH:
            // if (wifi.attach())
            // {
            //     printSuccess();
            //     printSerial("Wifi attached");
            //     sendEOT();
            // }
            // else
            // {
            //     printFailure();
            //     printSerial("Wifi not attached");
            //     sendEOT();
            // }
            break;
        case OPENBCI_WIFI_REMOVE:
            // if (wifi.remove())
            // {
            //     printSuccess();
            //     printSerial("Wifi removed");
            // }
            // else
            // {
            //     printFailure();
            //     printSerial("Wifi not removed");
            // }
            // sendEOT();
            break;
        case OPENBCI_WIFI_STATUS:
            // if (wifi.present)
            // {
            //     printAll("Wifi present");
            // }
            // else
            // {
            //     printAll("Wifi not present, send {");
            //     printAll(" to attach the shield");
            // }
            // sendEOT();
            break;
        case OPENBCI_WIFI_RESET:
            // wifi.reset();
            // printSerial("Wifi soft reset");
            // sendEOT();
            break;
        case OPENBCI_GET_VERSION:
            // printAll("v3.1.2");
            // sendEOT();
            break;
        default:
            return false;
        }
    }
    return true;
}

/// @brief Gets the active multi char command
/// @param
/// @return {char} multiCharCommand
char WifiServer::getMultiCharCommand(void)
{
    return multiCharCommand;
}

/// @brief When a 'x' is found on the serial port, we jump to this function
///         where we continue to read from the serial port and read the
///         remaining 7 bytes.
/// @param character
void WifiServer::processIncomingChannelSettings(char character)
{
    if (character == OPENBCI_CHANNEL_CMD_LATCH && numberOfIncomingSettingsProcessedChannel < OPENBCI_NUMBER_OF_BYTES_SETTINGS_CHANNEL - 1)
    {
        // We failed somehow and should just abort
        numberOfIncomingSettingsProcessedChannel = 0;

        // put flag back down
        endMultiCharCmdTimer();

        if (!_ads1299.streaming)
        {
            // printFailure();
            // printAll("too few chars");
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     wifi.sendStringLast("Failure: too few chars");
        // }

        return;
    }
    switch (numberOfIncomingSettingsProcessedChannel)
    {
    case 1: // channel number
        currentChannelSetting = getChannelCommandForAsciiChar(character);
        break;
    case 2: // POWER_DOWN
        optionalArgBuffer7[0] = getNumberForAsciiChar(character);
        break;
    case 3: // GAIN_SET
        optionalArgBuffer7[1] = getGainForAsciiChar(character);
        break;
    case 4: // INPUT_TYPE_SET
        optionalArgBuffer7[2] = getNumberForAsciiChar(character);
        break;
    case 5: // BIAS_SET
        optionalArgBuffer7[3] = getNumberForAsciiChar(character);
        break;
    case 6: // SRB2_SET
        optionalArgBuffer7[4] = getNumberForAsciiChar(character);

        break;
    case 7: // SRB1_SET
        optionalArgBuffer7[5] = getNumberForAsciiChar(character);
        break;
    case 8: // 'X' latch
        if (character != OPENBCI_CHANNEL_CMD_LATCH)
        {
            if (!_ads1299.streaming)
            {
                // printFailure();
                // printAll("Err: 9th char not X");
                // sendEOT();
            }
            // else if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringLast("Failure: Err: 9th char not X");
            // }
            // We failed somehow and should just abort
            numberOfIncomingSettingsProcessedChannel = 0;

            // put flag back down
            endMultiCharCmdTimer();
        }
        break;
    default: // should have exited
        if (!_ads1299.streaming)
        {
            // printFailure();
            // printAll("Err: too many chars");
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     wifi.sendStringLast("Failure: Err: too many chars");
        // }
        // We failed somehow and should just abort
        numberOfIncomingSettingsProcessedChannel = 0;

        // put flag back down
        endMultiCharCmdTimer();
        return;
    }

    // increment the number of bytes processed
    numberOfIncomingSettingsProcessedChannel++;

    if (numberOfIncomingSettingsProcessedChannel == (OPENBCI_NUMBER_OF_BYTES_SETTINGS_CHANNEL))
    {
        // We are done processing channel settings...
        if (!_ads1299.streaming)
        {
            // char buf[2];
            // printSuccess();
            // printAll("Channel set for ");
            // printAll(itoa(currentChannelSetting + 1, buf, 10));
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     char buf[3];
        //     wifi.sendStringMulti("Success: Channel set for ");
        //     delay(1);
        //     wifi.sendStringLast(itoa(currentChannelSetting + 1, buf, 10));
        // }

        _ads1299.channelSettings[currentChannelSetting][POWER_DOWN] = optionalArgBuffer7[0];
        _ads1299.channelSettings[currentChannelSetting][GAIN_SET] = optionalArgBuffer7[1];
        _ads1299.channelSettings[currentChannelSetting][INPUT_TYPE_SET] = optionalArgBuffer7[2];
        _ads1299.channelSettings[currentChannelSetting][BIAS_SET] = optionalArgBuffer7[3];
        _ads1299.channelSettings[currentChannelSetting][SRB2_SET] = optionalArgBuffer7[4];
        _ads1299.channelSettings[currentChannelSetting][SRB1_SET] = optionalArgBuffer7[5];

        // Set channel settings
        _ads1299.streamSafeChannelSettingsForChannel(
            currentChannelSetting + 1,
            _ads1299.channelSettings[currentChannelSetting][POWER_DOWN],
            _ads1299.channelSettings[currentChannelSetting][GAIN_SET],
            _ads1299.channelSettings[currentChannelSetting][INPUT_TYPE_SET],
            _ads1299.channelSettings[currentChannelSetting][BIAS_SET],
            _ads1299.channelSettings[currentChannelSetting][SRB2_SET],
            _ads1299.channelSettings[currentChannelSetting][SRB1_SET]);

        // Reset
        numberOfIncomingSettingsProcessedChannel = 0;

        // put flag back down
        endMultiCharCmdTimer();
    }
}

/// @brief When a 'z' is found on the serial port, we jump to this function
///         where we continue to read from the serial port and read the
///         remaining 4 bytes.
/// @param character {char} - The character you want to process...
void WifiServer::processIncomingLeadOffSettings(char character)
{
    if (character == OPENBCI_CHANNEL_IMPEDANCE_LATCH && numberOfIncomingSettingsProcessedLeadOff < OPENBCI_NUMBER_OF_BYTES_SETTINGS_LEAD_OFF - 1)
    {
        // We failed somehow and should just abort
        // reset numberOfIncomingSettingsProcessedLeadOff
        numberOfIncomingSettingsProcessedLeadOff = 0;

        // put flag back down
        endMultiCharCmdTimer();

        if (!_ads1299.streaming)
        {
            // printFailure();
            // printAll("too few chars");
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     wifi.sendStringLast("Failure: too few chars");
        // }

        return;
    }
    switch (numberOfIncomingSettingsProcessedLeadOff)
    {
    case 1: // channel number
        currentChannelSetting = getChannelCommandForAsciiChar(character);
        break;
    case 2: // pchannel setting
        optionalArgBuffer7[0] = getNumberForAsciiChar(character);
        break;
    case 3: // nchannel setting
        optionalArgBuffer7[1] = getNumberForAsciiChar(character);
        break;
    case 4: // 'Z' latch
        if (character != OPENBCI_CHANNEL_IMPEDANCE_LATCH)
        {
            if (!_ads1299.streaming)
            {
                // printFailure();
                // printAll("Err: 5th char not Z");
                // sendEOT();
            }
            // else if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringLast("Failure: Err: 5th char not Z");
            // }
            // We failed somehow and should just abort
            // reset numberOfIncomingSettingsProcessedLeadOff
            numberOfIncomingSettingsProcessedLeadOff = 0;

            // put flag back down
            endMultiCharCmdTimer();
        }
        break;
    default: // should have exited
        if (!_ads1299.streaming)
        {
            // printFailure();
            // printAll("Err: too many chars");
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     wifi.sendStringLast("Failure: Err: too many chars");
        // }
        // We failed somehow and should just abort
        // reset numberOfIncomingSettingsProcessedLeadOff
        numberOfIncomingSettingsProcessedLeadOff = 0;

        // put flag back down
        endMultiCharCmdTimer();
        return;
    }

    // increment the number of bytes processed
    numberOfIncomingSettingsProcessedLeadOff++;

    if (numberOfIncomingSettingsProcessedLeadOff == (OPENBCI_NUMBER_OF_BYTES_SETTINGS_LEAD_OFF))
    {
        // We are done processing lead off settings...

        if (!_ads1299.streaming)
        {
            // char buf[3];
            // printSuccess();
            // printAll("Lead off set for ");
            // printAll(itoa(currentChannelSetting + 1, buf, 10));
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     char buf[3];
        //     wifi.sendStringMulti("Success: Lead off set for ");
        //     delay(1);
        //     wifi.sendStringLast(itoa(currentChannelSetting + 1, buf, 10));
        // }

        _ads1299.leadOffSettings[currentChannelSetting][PCHAN] = optionalArgBuffer7[0];
        _ads1299.leadOffSettings[currentChannelSetting][NCHAN] = optionalArgBuffer7[1];

        // Set lead off settings
        _ads1299.streamSafeLeadOffSetForChannel(
            currentChannelSetting + 1,
            _ads1299.leadOffSettings[currentChannelSetting][PCHAN],
            _ads1299.leadOffSettings[currentChannelSetting][NCHAN]);

        // reset numberOfIncomingSettingsProcessedLeadOff
        numberOfIncomingSettingsProcessedLeadOff = 0;

        // put flag back down
        endMultiCharCmdTimer();
    }
}

void WifiServer::processIncomingBoardMode(char c)
{
    // if (c == OPENBCI_BOARD_MODE_SET)
    // {
    // printSuccess();
    // printAll(getBoardMode());
    // sendEOT();
    // }
    // else if (isDigit(c))
    // {
    //     uint8_t digit = c - '0';
    //     if (digit < BOARD_MODE_END_OF_MODES)
    //     {
    //         setBoardMode(digit);
    //         printSuccess();
    //         printAll(getBoardMode());
    //         sendEOT();
    //     }
    //     else
    //     {
    //         printFailure();
    //         printAll("board mode value");
    //         printAll(" out of bounds.");
    //         sendEOT();
    //     }
    // }
    // else
    // {
    //     printFailure();
    //     printAll("invalid board mode value.");
    //     sendEOT();
    // }
    endMultiCharCmdTimer();
}

void WifiServer::processIncomingSampleRate(char c)
{
    if (c == OPENBCI_SAMPLE_RATE_SET)
    {
        // printSuccess();
        // printAll("Sample rate is ");
        // printAll(getSampleRate());
        // printAll("Hz");
        // sendEOT();
    }
    else if (isDigit(c))
    {
        uint8_t digit = c - '0';
        if (digit <= ADS1299::SAMPLE_RATE_250)
        {
            _ads1299.streamSafeSetSampleRate((ADS1299::SAMPLE_RATE)digit);
            if (!_ads1299.streaming)
            {
                // printSuccess();
                // printAll("Sample rate is ");
                // printAll(getSampleRate());
                // printAll("Hz");
                // sendEOT();
            }
            // else if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringMulti("Success: Sample rate is ");
            //     delay(1);
            //     wifi.sendStringMulti(getSampleRate());
            //     delay(1);
            //     wifi.sendStringLast("Hz");
            // }
        }
        else
        {
            if (!_ads1299.streaming)
            {
                // printFailure();
                // printAll("sample value out of bounds");
                // sendEOT();
            }
            // else if (wifi.present && wifi.tx)
            // {
            //     wifi.sendStringMulti("Failure: sample value");
            //     delay(1);
            //     wifi.sendStringLast(" out of bounds");
            // }
        }
    }
    else
    {
        if (!_ads1299.streaming)
        {
            // printFailure();
            // printAll("invalid sample value");
            // sendEOT();
        }
        // else if (wifi.present && wifi.tx)
        // {
        //     wifi.sendStringLast("Failure: invalid sample value");
        //     delay(1);
        // }
    }
    endMultiCharCmdTimer();
}

/// @brief When a '`x' is found on the serial port it is a signal to insert a marker
///         of value x into the AUX1 stream (auxData[0]). This function sets the flag
///         to indicate that a new marker is available. The marker will be inserted
///         during the serial and sd write functions
/// @param c {char} - The character that will be inserted into the data stream
void WifiServer::processInsertMarker(char c)
{
    markerValue = c;
    newMarkerReceived = true;
    endMultiCharCmdTimer();
    // if (wifi.present && wifi.tx)
    // {
    //     wifi.sendStringLast("Marker recieved");
    // }
}

/// @brief Start the timer on multi char commands
/// @param cmd {char} the command received on the serial stream. See enum MULTI_CHAR_COMMAND
void WifiServer::startMultiCharCmdTimer(char cmd)
{
    // if (curDebugMode == DEBUG_MODE_ON)
    // {
    //     Serial1.printf("Start multi char: %c\n", cmd);
    // }
    isMultiCharCmd = true;
    multiCharCommand = cmd;
    multiCharCmdTimeout = millis() + MULTI_CHAR_COMMAND_TIMEOUT_MS;
}

/// @brief Using publically available state variables to drive packet type settings
/// @param
void WifiServer::setCurPacketType(void)
{
    if (curAccelMode == ACCEL_MODE_ON && curTimeSyncMode == TIME_SYNC_MODE_ON)
    {
        curPacketType = PACKET_TYPE_ACCEL_TIME_SET;
    }
    else if (curAccelMode == ACCEL_MODE_OFF && curTimeSyncMode == TIME_SYNC_MODE_ON)
    {
        curPacketType = PACKET_TYPE_RAW_AUX_TIME_SET;
    }
    else if (curAccelMode == ACCEL_MODE_OFF && curTimeSyncMode == TIME_SYNC_MODE_OFF)
    {
        curPacketType = PACKET_TYPE_RAW_AUX;
    }
    else
    { // default accel on mode
        // curAccelMode == ACCEL_MODE_ON && curTimeSyncMode == TIME_SYNC_MODE_OFF
        curPacketType = PACKET_TYPE_ACCEL;
    }
}

/// @brief End the timer on multi char commands
/// @param
void WifiServer::endMultiCharCmdTimer(void)
{
    isMultiCharCmd = false;
    multiCharCommand = MULTI_CHAR_CMD_NONE;
}

/// @brief Converts ascii character to byte value for channel setting bytes
/// @param asciiChar [char] - The ascii character to convert
/// @return [char] - Byte number value of acsii character, defaults to 0
char WifiServer::getChannelCommandForAsciiChar(char asciiChar)
{
    switch (asciiChar)
    {
    case OPENBCI_CHANNEL_CMD_CHANNEL_1:
        return 0x00;
    case OPENBCI_CHANNEL_CMD_CHANNEL_2:
        return 0x01;
    case OPENBCI_CHANNEL_CMD_CHANNEL_3:
        return 0x02;
    case OPENBCI_CHANNEL_CMD_CHANNEL_4:
        return 0x03;
    case OPENBCI_CHANNEL_CMD_CHANNEL_5:
        return 0x04;
    case OPENBCI_CHANNEL_CMD_CHANNEL_6:
        return 0x05;
    case OPENBCI_CHANNEL_CMD_CHANNEL_7:
        return 0x06;
    case OPENBCI_CHANNEL_CMD_CHANNEL_8:
        return 0x07;
    case OPENBCI_CHANNEL_CMD_CHANNEL_9:
        return 0x08;
    case OPENBCI_CHANNEL_CMD_CHANNEL_10:
        return 0x09;
    case OPENBCI_CHANNEL_CMD_CHANNEL_11:
        return 0x0A;
    case OPENBCI_CHANNEL_CMD_CHANNEL_12:
        return 0x0B;
    case OPENBCI_CHANNEL_CMD_CHANNEL_13:
        return 0x0C;
    case OPENBCI_CHANNEL_CMD_CHANNEL_14:
        return 0x0D;
    case OPENBCI_CHANNEL_CMD_CHANNEL_15:
        return 0x0E;
    case OPENBCI_CHANNEL_CMD_CHANNEL_16:
        return 0x0F;
    default:
        return 0x00;
    }
}

/// @brief Converts ascii character to get gain from channel settings
/// @param asciiChar [char] - The ascii character to convert
/// @return [char] - Byte number value of acsii character, defaults to 0
char WifiServer::getNumberForAsciiChar(char asciiChar)
{
    if (asciiChar < '0' || asciiChar > '9')
    {
        asciiChar = '0';
    }

    // Convert ascii char to number
    asciiChar -= '0';

    return asciiChar;
}

/// @brief Converts ascii character to get gain from channel settings
/// @param asciiChar [char] - The ascii character to convert
/// @return [char] - Byte number value of acsii character, defaults to 0
char WifiServer::getGainForAsciiChar(char asciiChar)
{
    char output = 0x00;

    if (asciiChar < '0' || asciiChar > '6')
    {
        asciiChar = '6'; // Default to 24
    }

    output = asciiChar - '0';

    return output << 4;
}

/// @brief Check for valid on multi char commands
/// @param
/// @return {boolean} true if a multi char commands is active and the timer is running, otherwise False
boolean WifiServer::checkMultiCharCmdTimer(void)
{
    if (isMultiCharCmd)
    {
        if (millis() < multiCharCmdTimeout)
            return true;
        else
        { // the timer has timed out - reset the multi char timeout
            endMultiCharCmdTimer();
            // printAll("Timeout processing multi byte");
            // printAll(" message - please send all");
            // printAll(" commands at once as of v2");
            // sendEOT();
        }
    }
    return false;
}

void WifiServer::initVariables(void)
{
    clientWaitingForResponse = false;
    clientWaitingForResponseFullfilled = false;
    jsonHasSampleNumbers = false;
    jsonHasTimeStamps = true;
    passthroughBufferLoaded = false;
    redundancy = false;
    tcpDelimiter = false;

    head = 0;
    lastSampleNumber = 0;
    lastTimeWasPolled = 0;
    mqttPort = DEFAULT_MQTT_PORT;
    passthroughPosition = 0;
    rawBufferHead = 0;
    rawBufferTail = 0;
    tail = 0;
    tcpPort = 80;
    timePassthroughBufferLoaded = 0;
    _counter = 0;
    _latency = DEFAULT_LATENCY;
    _ntpOffset = 0;
    currentChannelSetting = 0;

#ifdef MQTT
    mqttBrokerAddress = "";
    mqttUsername = "";
    mqttPassword = "";
#endif
    outputString = "";

    tcpAddress = IPAddress();

    // Enums
    curOutputMode = OUTPUT_MODE_RAW;
    curOutputProtocol = OUTPUT_PROTOCOL_NONE;
    curPacketType = PACKET_TYPE_ACCEL;
    curTimeSyncMode = TIME_SYNC_MODE_OFF;
    curAccelMode = ACCEL_MODE_OFF;
}

/// @brief Initalize arrays here
/// @param
void WifiServer::initArrays(void)
{
    gainReset();
}

/// @brief Initalize class objects here
/// @param
void WifiServer::initObjects(void)
{
    // setNumChannels(0);
#ifdef RAW_TO_JSON
    for (size_t i = 0; i < NUM_PACKETS_IN_RING_BUFFER_JSON; i++)
    {
        sampleReset(sampleBuffer + i);
    }
#endif
}

/// @brief Get a string version of the output mode
/// @param outputMode   {OUTPUT_MODE} The output mode is either 'raw' or 'json'
/// @return             {String} String version of the output mode
String WifiServer::getOutputModeString(OUTPUT_MODE outputMode)
{
    switch (outputMode)
    {
    case OUTPUT_MODE_JSON:
        return OUTPUT_JSON;
    case OUTPUT_MODE_RAW:
    default:
        return OUTPUT_RAW;
    }
}

/// @brief Get a string version of the output protocol
/// @param  outputProtocol  {OUTPUT_PROTOCOL} The output protocol is either 'tcp',
///                         'mqtt', 'serial', or 'none'
/// @return                 {String} String version of the output protoocol
String WifiServer::getOutputProtocolString(OUTPUT_PROTOCOL outputProtocol)
{
    switch (outputProtocol)
    {
    case OUTPUT_PROTOCOL_TCP:
        return OUTPUT_TCP;
    case OUTPUT_PROTOCOL_MQTT:
        return OUTPUT_MQTT;
    case OUTPUT_PROTOCOL_SERIAL:
        return OUTPUT_SERIAL;
    case OUTPUT_PROTOCOL_WEB_SOCKETS:
        return OUTPUT_WEB_SOCKETS;
    case OUTPUT_PROTOCOL_NONE:
    default:
        return OUTPUT_NONE;
    }
}

/// @brief Used to print out a long long number
/// @param  n    {int64_t} The signed number
/// @return
String WifiServer::getStringLLNumber(long long n)
{
    return getStringLLNumber(n, DEC);
}

/// @brief Used to print out a long long number
/// @param n    {int64_t} The signed number
/// @param base {uint8_t} The base you want to print in. DEC, HEX, BIN
/// @return
String WifiServer::getStringLLNumber(long long n, uint8_t base)
{
    unsigned char buf[16 * sizeof(long)]; // Assumes 8-bit chars.
    unsigned long long i = 0;

    if (n == 0)
    {
        return "0";
    }
    String output;
    while (n > 0)
    {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--)
    {
        output = output + String((char)(buf[i - 1] < 10
                                            ? '0' + buf[i - 1]
                                            : 'A' + buf[i - 1] - 10));
    }
    return output;
}

#ifdef RAW_TO_JSON
/// @brief Used to get a scale factor given a gain. Scale factors are hardcoded in
///         file `OpenBCI_Wifi_Definitions.h`
/// @param  gain {uint8_t} - A gain value from cyton, so either 1, 2, 4, 6, 8,
///              12, or 24
/// @return      {double} - The scale factor in volts
double WifiServer::getScaleFactorVoltsCyton(uint8_t gain)
{
    switch (gain)
    {
    case ADS_GAIN_1:
        return ADS_SCALE_FACTOR_VOLTS_1;
    case ADS_GAIN_2:
        return ADS_SCALE_FACTOR_VOLTS_2;
    case ADS_GAIN_4:
        return ADS_SCALE_FACTOR_VOLTS_4;
    case ADS_GAIN_6:
        return ADS_SCALE_FACTOR_VOLTS_6;
    case ADS_GAIN_8:
        return ADS_SCALE_FACTOR_VOLTS_8;
    case ADS_GAIN_12:
        return ADS_SCALE_FACTOR_VOLTS_12;
    case ADS_GAIN_24:
        return ADS_SCALE_FACTOR_VOLTS_24;
    default:
        return 1.0;
    }
}

double WifiServer::getScaleFactorVoltsGanglion(void)
{
    return MCP_SCALE_FACTOR_VOLTS;
}
#endif

/// @brief Returns the current of the buffer
/// @param
/// @return
uint8_t WifiServer::getTail(void)
{
    return tail;
}

/// @brief Safely get the time, defaults to micros() if ntp is not active.
/// @param
/// @return
unsigned long long WifiServer::getTime(void)
{
    if (ntpActive())
    {
        return ntpGetTime() + ntpGetPreciseAdjustment(_ntpOffset);
    }
    else
    {
        return micros();
    }
}

String WifiServer::getVersion()
{
    return SOFTWARE_VERSION;
}

/// @brief Convert an int24 into int32
/// @param arr  {uint8_t *} - 3-byte array of signed 24 bit number
/// @return     int32 - The converted number
int32_t WifiServer::int24To32(uint8_t *arr)
{
    uint32_t raw = 0;
    raw = arr[0] << 16 | arr[1] << 8 | arr[2];
    // carry through the sign
    if (bitRead(raw, 23) == 1)
    {
        raw |= 0xFF000000;
    }
    else
    {
        raw &= 0x00FFFFFF;
    }
    return (int32_t)raw;
}

/// @brief Test to see if a char follows the stream tail byte format
/// @param b
/// @return
boolean WifiServer::isAStreamByte(uint8_t b)
{
    return (b >> 4) == 0xC;
}

void WifiServer::loop(void)
{
}

/// @brief Check to see if SNTP is active
/// @param
/// @return
boolean WifiServer::ntpActive(void)
{
    return time(nullptr) > 1000;
}

/// @brief Calculate an adjusment based off an offset and the internal micros
/// @param ntpOffset {unsigned long} A time in micro seconds
/// @return          {unsigned long long} The adjested offset.
unsigned long long WifiServer::ntpGetPreciseAdjustment(unsigned long ntpOffset)
{
    unsigned long long boardTime_uS = micros() % MICROS_IN_SECONDS;
    unsigned long long adj = boardTime_uS - ntpOffset;
    if (boardTime_uS < ntpOffset)
    {
        boardTime_uS += MICROS_IN_SECONDS;
        adj = boardTime_uS - ntpOffset;
    }
    return adj;
}

/// @brief Get ntp time in microseconds
/// @param
/// @return [long] - The time in micro second
unsigned long long WifiServer::ntpGetTime(void)
{
    unsigned long long curTime = time(nullptr);
    return curTime * MICROS_IN_SECONDS;
}

/// @brief Use this to start the sntp time sync
/// @param
void WifiServer::ntpStart(void)
{
#ifdef DEBUG
    _serial.println("Setting time using SNTP");
#endif
    configTime(8 * 3600, 0, "ntp1.aliyun.com", "pool.ntp.org");
}

void WifiServer::passthroughBufferClear(void)
{
    for (uint8_t i = 0; i < BYTES_PER_SPI_PACKET; i++)
    {
        passthroughBuffer[i] = 0;
    }
    passthroughPosition = 0;
}

uint8_t WifiServer::passthroughCommands(String commands)
{
#ifdef DEBUG
    _serial.println("passthroughCommands:" + commands);
#endif
    uint8_t numCmds = uint8_t(commands.length());
    if (numCmds == 0)
    {
        return PASSTHROUGH_FAIL_NO_CHARS;
    }
    processChar(commands.c_str()[0]);
    // if (numCmds > BYTES_PER_SPI_PACKET - 1)
    // {
    //     return PASSTHROUGH_FAIL_TOO_MANY_CHARS;
    // }
    // else if (numCmds == 0)
    // {
    //     return PASSTHROUGH_FAIL_NO_CHARS;
    // }
    // // _serial.printf("got %d commands | passthroughPosition: %d\n", numCmds, passthroughPosition);
    // if (passthroughPosition > 0)
    // {
    //     if (numCmds > BYTES_PER_SPI_PACKET - passthroughPosition - 1)
    //     { // -1 because of numCmds as first byte
    //         return PASSTHROUGH_FAIL_QUEUE_FILLED;
    //     }
    //     passthroughBuffer[0] += numCmds;
    // }
    // else
    // {
    //     passthroughBuffer[passthroughPosition++] = numCmds;
    // }
    // for (int i = 0; i < numCmds; i++)
    // {
    //     // _serial.printf("cmd %c | passthroughPosition: %d\n", commands.charAt(i), passthroughPosition);
    //     passthroughBuffer[passthroughPosition++] = commands.charAt(i);
    // }
    // passthroughBufferLoaded = true;
    // clientWaitingForResponse = true;
    // timePassthroughBufferLoaded = millis();
    // // SPISlave.setData(passthroughBuffer, BYTES_PER_SPI_PACKET); // todo!
    return PASSTHROUGH_PASS;
}

String WifiServer::perfectPrintByteHex(uint8_t b)
{
    if (b <= 0x0F)
    {
        return "0" + String(b, HEX);
    }
    else
    {
        return String(b, HEX);
    }
}

/// @brief Used to convert the raw int32_t into a scaled double
/// @param raw          {int32_t} - The raw value
/// @param scaleFactor  {double} - The scale factor to multiply by
/// @return             {double} - The raw value scaled by `scaleFactor`
///                                 converted into nano volts.
double WifiServer::rawToScaled(int32_t raw, double scaleFactor)
{
    return scaleFactor * raw * NANO_VOLTS_IN_VOLTS;
}

void WifiServer::reset(void)
{
    initVariables();
    initArrays();
    initObjects();
}

#ifdef RAW_TO_JSON
/// @brief Resets all the samples assuming 16 channels
/// @param
void WifiServer::sampleReset(void)
{
    for (int i = 0; i < NUM_PACKETS_IN_RING_BUFFER_JSON; i++)
    {
        sampleReset(sampleBuffer + i);
    }
}

/// @brief Resets the sample assuming 16 channels
/// @param sample {Sample} - The sample struct
void WifiServer::sampleReset(Sample *sample)
{
    sampleReset(sample, NUM_CHANNELS_CYTON_DAISY);
}

/// @brief Resets the sample
/// @param sample       {Sample} - The sample struct
/// @param numChannels  {uint8_t} - The number of channels to clear to zero
void WifiServer::sampleReset(Sample *sample, uint8_t numChannels)
{
    for (size_t i = 0; i < numChannels; i++)
    {
        sample->channelData[i] = 0.0;
    }
    sample->sampleNumber = 0;
    sample->timestamp = 0;
}
#endif

void WifiServer::setGains(uint8_t *raw)
{
    // setGains(raw, _gains);
}

/// @brief Used to set the gain for the JSON conversion
/// @param raw [uint8_t *] - The array from Pic or Cyton descirbing channels
///             and gains.
/// @param gains
void WifiServer::setGains(uint8_t *raw, uint8_t *gains)
{
    uint8_t byteCounter = 2;
    uint8_t numChannels = raw[byteCounter++];

    if (numChannels < NUM_CHANNELS_GANGLION || numChannels > MAX_CHANNELS)
    {
        return;
    }
    for (uint8_t i = 0; i < numChannels; i++)
    {
        if (numChannels == NUM_CHANNELS_GANGLION)
        {
            // do gang related stuffs
            // _gains[i] = getGainGanglion();
        }
        else
        {
            // Save the gains for later sending in /all
            // _gains[i] = getGainCyton(raw[byteCounter]);
        }
    }
    setNumChannels(numChannels);
#ifdef DEBUG
    _serial.println("Gain set");
#endif
}
#ifdef MQTT
/// @brief Used to set the information required for a succesful MQTT communication
/// @param brokerAddress {String} - A string such as 'mock.getcloudbrain.com'
/// @param username      {String} - The username for the MQTT broker to user
/// @param password      {String} - The password for you to connect to
/// @param port
void WifiServer::setInfoMQTT(String brokerAddress, String username, String password, int port)
{
    mqttBrokerAddress = brokerAddress;
    mqttUsername = username;
    mqttPassword = password;
    mqttPort = port;
    setOutputProtocol(OUTPUT_PROTOCOL_MQTT);
}
#endif

/// @brief Used to configure the requried internal variables for UDP communication
/// @param address    {IPAddress} - The ip address in string form: "192.168.0.1"
/// @param port       {int} - The port number as an int
/// @param delimiter  {boolean} - Include the tcpDelimiter '\r\n'?
void WifiServer::setInfoUDP(String address, int port, boolean delimiter)
{
    tcpAddress.fromString(address);
    tcpDelimiter = delimiter;
    tcpPort = port;
    setOutputProtocol(OUTPUT_PROTOCOL_UDP);
}

/// @brief Used to configure the requried internal variables for TCP communication
/// @param address   {IPAddress} - The ip address in string form: "192.168.0.1"
/// @param port      {int} - The port number as an int
/// @param delimiter {boolean} - Include the tcpDelimiter '\r\n'?
void WifiServer::setInfoTCP(String address, int port, boolean delimiter)
{
    tcpAddress.fromString(address);
    tcpDelimiter = delimiter;
    tcpPort = port;
    setOutputProtocol(OUTPUT_PROTOCOL_UDP);
}

/// @brief Sets the latency
/// @param latency {unsigned long} - The latency of the system
void WifiServer::setLatency(unsigned long latency)
{
    _latency = latency;
}

/// @brief Gets the latency
/// @param
/// @return {unsigned long} - The latency of the system
unsigned long WifiServer::getLatency(void)
{
    return _latency;
}

/// @brief Used to get the last two bytes of the max addresses
/// @param
/// @return {String} The last two bytes, will always be four chars.
String WifiServer::getMacLastFourBytes(void)
{
    uint8_t mac[6];
    WiFi.softAPmacAddress(mac);
    String macID = perfectPrintByteHex(mac[6 - 2]) +
                   perfectPrintByteHex(mac[6 - 1]);
    macID.toUpperCase();
    return macID;
}

/// @brief Returns the full mac address
/// @param
/// @return {String} Mac address with bytes separated with colons
String WifiServer::getMac(void)
{
    uint8_t mac[6];
    WiFi.softAPmacAddress(mac);
    String fullMac;

    for (int i = 0; i < 6; ++i)
    {
        fullMac += perfectPrintByteHex(mac[i]);

        if (i < 6 - 1)
        {
            fullMac += ":";
        }
    }

    fullMac.toUpperCase();
    return fullMac;
}

/// @brief Model number has Push The World and their product number encoded with unqiue
///         last four bytes of mac address.
/// @param
/// @return {String} The model number
String WifiServer::getModelNumber(void)
{
    String AP_NameString = "PTW-0001-" + getMacLastFourBytes();

    // char AP_NameChar[AP_NameString.length() + 1];
    // memset(AP_NameChar, 0, AP_NameString.length() + 1);

    // for (int i = 0; i < AP_NameString.length(); i++)
    //     AP_NameChar[i] = AP_NameString.charAt(i);

    return AP_NameString;
}

/// @brief Each OpenBCI Wifi shield has a unique name
/// @param
/// @return {String} - The name of the device.
String WifiServer::getName(void)
{
    String AP_NameString = "OpenBCI-" + getMacLastFourBytes();

    // char AP_NameChar[AP_NameString.length() + 1];
    // memset(AP_NameChar, 0, AP_NameString.length() + 1);

    // for (int i = 0; i < AP_NameString.length(); i++)
    // {
    //     AP_NameChar[i] = AP_NameString.charAt(i);
    // }

    return AP_NameString;
}

uint8_t WifiServer::getNumChannels(void)
{
    return _ads1299.numChannels;
}

unsigned long WifiServer::getNTPOffset(void)
{
    return _ntpOffset;
}

/// @brief Used to set the current output protocl
/// @param  newOutputProtocol {OUTPUT_PROTOCOL} The output protocol you want to switch to
void WifiServer::setOutputProtocol(OUTPUT_PROTOCOL newOutputProtocol)
{
    curOutputProtocol = newOutputProtocol;
}

// todo!
boolean WifiServer::spiHasMaster(void)
{
    return boolean(true);
}

/// @brief Set the number of channels
/// @param  numChannels {uint8_t} - The number of channels to set the system to
void WifiServer::setNumChannels(uint8_t numChannels)
{
    _ads1299.numChannels = numChannels;
    // Used for JSON output mode
    _jsonBufferSize = 0;                                                              // Reset to 0
    _jsonBufferSize += JSON_OBJECT_SIZE(2);                                           // For {"chunk":[...], "count":0}
    _jsonBufferSize += JSON_ARRAY_SIZE(getJSONMaxPackets(numChannels));               // For the array of samples
    _jsonBufferSize += getJSONMaxPackets(numChannels) * JSON_OBJECT_SIZE(3);          // For each sample {"timestamp":0, "data":[...], "sampleNumber":0}
    _jsonBufferSize += getJSONMaxPackets(numChannels) * JSON_ARRAY_SIZE(numChannels); // For data array for each sample
    _jsonBufferSize += getJSONAdditionalBytes(numChannels);                           // The additional bytes needed for input duplication
}

/// @brief Set the ntp offset of the system
/// @param ntpOffset {unsigned long} - The micros mod with one million
void WifiServer::setNTPOffset(unsigned long ntpOffset)
{
    _ntpOffset = ntpOffset;
}

/// @brief Used to set the current output mode
/// @param newOutputMode {OUTPUT_MODE} The output mode you want to switch to
void WifiServer::setOutputMode(OUTPUT_MODE newOutputMode)
{
    curOutputMode = newOutputMode;
}
