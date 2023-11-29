#include <WiFi.h>
#include "WifiServer.h"

WifiServer::WifiServer()
    : ledState(false), startWifiManager(false), tryConnectToAP(false), underSelfTest(false),
      wifiReset(false), lastHeadMove(0), lastSendToClient(0), ledFlashes(0), ledInterval(300),
      ledLastFlash(millis()), wifiConnectTimeout(millis()), jsonStr(""), bufferPosition(0),
      buffer{}, _serial(Serial0), _ads1299(ads1299), _server(server), _WiFi(WiFi)
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
    return getBoardTypeString(curNumChannels);
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
    for (uint8_t i = 0; i < getNumChannels(); i++)
    {
        gainsArr.add(_gains[i]);
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
    // 配置Web服务器
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", "Hello, configure your Wi-Fi here!"); });
    _server.begin();
}

boolean WifiServer::connectToWiFi(const char *ssid, const char *password)
{
    _serial.println("Connecting to WiFi...");
    _WiFi.begin(ssid, password);
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
    sendHeadersForCORS(request);
    AsyncWebServerResponse *response = request->beginResponse(code, "text/plain", s + "\r\n");
    request->send(response);
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
    debugPrintGet();
#endif

    // 设置 CORS 头部
    sendHeadersForCORS(request);

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
    root.printTo(_serial);
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
    _serial.print("Current uri: ");
    _serial.println(server.uri());
    _serial.print("Starting socket to host: ");
    _serial.print(tcpAddress.toString());
    _serial.print(" on port: ");
    _serial.println(tcpPort);
#endif

    setOutputProtocol(OUTPUT_PROTOCOL_TCP);

    // const size_t bufferSize = JSON_OBJECT_SIZE(6) + 40*6;
    // DynamicJsonBuffer jsonBuffer(bufferSize);
    // JsonObject& rootOut = jsonBuffer.createObject();
    sendHeadersForCORS(request);
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
    }
}

void WifiServer::udpSetup(AsyncWebServerRequest *request)
{
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
    _serial.print("Current uri: ");
    _serial.println(server.uri());
    _serial.print("Ready to write to: ");
    _serial.print(tcpAddress.toString());
    _serial.print(" on port: ");
    _serial.println(tcpPort);
#endif

    sendHeadersForCORS(request);
    jsonStr = "";
    return request->send(200, "text/json", jsonStr.c_str());
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

void WifiServer::initVariables(void)
{
    clientWaitingForResponse = false;
    clientWaitingForResponseFullfilled = false;
    jsonHasSampleNumbers = false;
    jsonHasTimeStamps = true;
    passthroughBufferLoaded = false;
    redundancy = false;
    tcpDelimiter = false;

    curNumChannels = 0;
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

#ifdef MQTT
    mqttBrokerAddress = "";
    mqttUsername = "";
    mqttPassword = "";
#endif
    outputString = "";

    tcpAddress = IPAddress();

    curOutputMode = OUTPUT_MODE_RAW;
    curOutputProtocol = OUTPUT_PROTOCOL_NONE;
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
    setNumChannels(0);
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
    uint8_t numCmds = uint8_t(commands.length());
    if (numCmds > BYTES_PER_SPI_PACKET - 1)
    {
        return PASSTHROUGH_FAIL_TOO_MANY_CHARS;
    }
    else if (numCmds == 0)
    {
        return PASSTHROUGH_FAIL_NO_CHARS;
    }
    // _serial.printf("got %d commands | passthroughPosition: %d\n", numCmds, passthroughPosition);
    if (passthroughPosition > 0)
    {
        if (numCmds > BYTES_PER_SPI_PACKET - passthroughPosition - 1)
        { // -1 because of numCmds as first byte
            return PASSTHROUGH_FAIL_QUEUE_FILLED;
        }
        passthroughBuffer[0] += numCmds;
    }
    else
    {
        passthroughBuffer[passthroughPosition++] = numCmds;
    }
    for (int i = 0; i < numCmds; i++)
    {
        // _serial.printf("cmd %c | passthroughPosition: %d\n", commands.charAt(i), passthroughPosition);
        passthroughBuffer[passthroughPosition++] = commands.charAt(i);
    }
    passthroughBufferLoaded = true;
    clientWaitingForResponse = true;
    timePassthroughBufferLoaded = millis();
    // SPISlave.setData(passthroughBuffer, BYTES_PER_SPI_PACKET); // todo!
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
    setGains(raw, _gains);
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
            _gains[i] = getGainGanglion();
        }
        else
        {
            // Save the gains for later sending in /all
            _gains[i] = getGainCyton(raw[byteCounter]);
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

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i = 0; i < AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);

    return AP_NameString;
}

/// @brief Each OpenBCI Wifi shield has a unique name
/// @param
/// @return {String} - The name of the device.
String WifiServer::getName(void)
{
    String AP_NameString = "OpenBCI-" + getMacLastFourBytes();

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i = 0; i < AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);

    return AP_NameString;
}

uint8_t WifiServer::getNumChannels(void)
{
    return curNumChannels;
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
    return boolean();
}

/// @brief Set the number of channels
/// @param  numChannels {uint8_t} - The number of channels to set the system to
void WifiServer::setNumChannels(uint8_t numChannels)
{
    curNumChannels = numChannels;
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
