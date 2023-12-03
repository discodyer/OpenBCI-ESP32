#pragma once
#include "Config.h"
#include <Arduino.h>
#include "ADS1299_Definitions.h"

class ADS1299
{
public:
    // 定义枚举类型
    enum ChipSelect
    {
        BOARD_ADS,
        DAISY_ADS,
        BOTH_ADS
    };
    enum SAMPLE_RATE
    {
        SAMPLE_RATE_16000,
        SAMPLE_RATE_8000,
        SAMPLE_RATE_4000,
        SAMPLE_RATE_2000,
        SAMPLE_RATE_1000,
        SAMPLE_RATE_500,
        SAMPLE_RATE_250
    };

    // Variables
    boolean boardUseSRB1; // used to keep track of if we are using SRB1
    boolean daisyPresent;
    boolean daisyUseSRB1;
    boolean streaming;
    boolean useInBias[OPENBCI_NUMBER_OF_CHANNELS_DAISY]; // used to remember if we were included in Bias before channel power down
    boolean useSRB2[OPENBCI_NUMBER_OF_CHANNELS_DAISY];
    boolean verbosity; // turn on/off Serial verbosity

    byte boardChannelDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];                              // array to hold raw channel data
    byte channelSettings[OPENBCI_NUMBER_OF_CHANNELS_DAISY][OPENBCI_NUMBER_OF_CHANNEL_SETTINGS]; // array to hold current channel settings
    byte daisyChannelDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
    byte defaultChannelSettings[OPENBCI_NUMBER_OF_CHANNEL_SETTINGS]; // default channel settings
    byte lastBoardDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
    byte lastDaisyDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
    byte leadOffSettings[OPENBCI_NUMBER_OF_CHANNELS_DAISY][OPENBCI_NUMBER_OF_LEAD_OFF_SETTINGS]; // used to control on/off of impedance measure for P and N side of each channel
    byte meanBoardDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
    byte meanDaisyDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
    // byte sampleCounter;
    // byte sampleCounterBLE;

    int boardChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE]; // array used when reading channel data as ints
    int daisyChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE]; // array used when reading channel data as ints
    int lastBoardChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
    int lastDaisyChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
    int meanBoardChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
    int meanDaisyChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
    int numChannels;

    short auxData[3]; // This is user faceing
    short axisData[3];

    unsigned long lastSampleTime;

    static volatile bool channelDataAvailable;

    // ENUMS
    // ACCEL_MODE curAccelMode;
    // BOARD_MODE curBoardMode;
    // DEBUG_MODE curDebugMode;
    // PACKET_TYPE curPacketType;
    SAMPLE_RATE curSampleRate;
    // TIME_SYNC_MODE curTimeSyncMode;

    ADS1299();
    void initialize();
    void attachDaisy(void);
    void setSampleRate(uint8_t newSampleRateCode);
    void initialize_ads();
    void resetADS(ChipSelect targetSS);
    boolean smellDaisy(void);
    void writeChannelSettings(void);
    void writeChannelSettings(byte);
    void changeChannelLeadOffDetect();
    void changeChannelLeadOffDetect(byte N);
    void softReset(void);
    void configureLeadOffDetection(byte amplitudeCode, byte freqCode);
    byte ADS_getDeviceID(ChipSelect);
    void boardBeginADSInterrupt(void);
    void updateChannelData(void);
    void updateBoardData(void);
    void updateBoardData(boolean);
    void updateDaisyData(void);
    void updateDaisyData(boolean);
    void sendChannelData(void);
    // void sendChannelData(PACKET_TYPE);
    void sendChannelDataSerial();
    void startHSPI(void);
    void start();
    void streamSafeChannelDeactivate(byte channelNumber);
    void streamStop();
    void stopADS();
    void streamStart();
    void startADS();
    void streamSafeChannelActivate(byte channelNumber);
    void activateChannel(byte);
    void activateAllChannelsToTestCondition(byte testInputCode, byte amplitudeCode, byte freqCode);
    void configureInternalTestSignal(byte amplitudeCode, byte freqCode);
    void changeInputType(byte inputCode);
    void streamSafeSetAllChannelsToDefault(void);
    void setChannelsToDefault(void);
    void removeDaisy(void);
    void streamSafeChannelSettingsForChannel(byte channelNumber, byte powerDown, byte gain, byte inputType, byte bias, byte srb2, byte srb1);
    void streamSafeChannelSettingsForChannel(byte channelNumber);
    void streamSafeLeadOffSetForChannel(byte channelNumber, byte pInput, byte nInput);
    void streamSafeLeadOffSetForChannel(byte channelNumber);
    void streamSafeSetSampleRate(SAMPLE_RATE sr);
    char getDefaultChannelSettingForSettingAscii(byte setting);
    byte getDefaultChannelSettingForSetting(byte setting);
    // void printfWifi(const char *format, ...);
    // void printlnWifi(const char *msg);
    // void printWifi(const char *msg);

    ~ADS1299();

private:
    static void IRAM_ATTR ADS_DRDY_Service();
    byte xfer(byte _data);
    void csLow(ChipSelect targetSS);
    void csHigh(ChipSelect targetSS);
    void RESET(ChipSelect targetSS);
    void SDATAC(ChipSelect targetSS);
    void deactivateChannel(byte N);
    byte RREG(byte, ChipSelect targetSS);
    void WREG(byte, byte, ChipSelect); // write one ADS register
    void STOP(ChipSelect targetSS);
    void RDATAC(ChipSelect targetSS);
    void START(ChipSelect targetSS);
    void STANDBY(ChipSelect targetSS);

    // Variables
    boolean firstDataPacket;
    byte regData[24]; // array is used to mirror register data
    int boardStat;    // used to hold the status register
    int daisyStat;
    boolean isRunning;


    // void printRegisterName(byte);
    // void printAll(char);
    // void printAll(const char *);
};
