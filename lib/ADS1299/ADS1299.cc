#include "ADS1299.h"

volatile bool ADS1299::channelDataAvailable = false;

/// @brief ADS1299 回调函数
/// @return
void IRAM_ATTR ADS1299::ADS_DRDY_Service()
{
    channelDataAvailable = true;
}

/// @brief SPI communication method
/// @param byte data
/// @return byte data
byte ADS1299::xfer(byte data)
{
    // 发送数据并接收返回的数据
    return hspi->transfer(data);
}

void ADS1299::csLow(ChipSelect targetSS)
{
    // hspi->beginTransaction(SPISettings(ADS_SPI_SPEED, MSBFIRST, SPI_MODE1));
    // hspi->setFrequency(ADS_SPI_SPEED);
    switch (targetSS)
    {
    case BOARD_ADS:
        digitalWrite(PIN_ADS_CS1, LOW);
        break;
    case DAISY_ADS:
        digitalWrite(PIN_ADS_CS2, LOW);
        break;
    case BOTH_ADS:
        digitalWrite(PIN_ADS_CS1, LOW);
        digitalWrite(PIN_ADS_CS2, LOW);
        break;
    default:
        // 处理未知的枚举值
        break;
    }
}

void ADS1299::csHigh(ChipSelect targetSS)
{
    switch (targetSS)
    {
    case BOARD_ADS:
        digitalWrite(PIN_ADS_CS1, HIGH);
        break;
    case DAISY_ADS:
        digitalWrite(PIN_ADS_CS2, HIGH);
        break;
    case BOTH_ADS:
        digitalWrite(PIN_ADS_CS1, HIGH);
        digitalWrite(PIN_ADS_CS2, HIGH);
        break;
    default:
        // 处理未知的枚举值
        break;
    }
    // hspi->endTransaction();
}

/// @brief reset all the registers to default settings
/// @param targetSS
void ADS1299::RESET(ChipSelect targetSS)
{
    // reset all the registers to default settings
    csLow(targetSS);
    xfer(_RESET);
    delayMicroseconds(12); // must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    csHigh(targetSS);
}

/// @brief get out of read data continuous mode
/// @param targetSS
void ADS1299::SDATAC(ChipSelect targetSS)
{
    csLow(targetSS);
    xfer(_SDATAC);
    csHigh(targetSS);
    delayMicroseconds(10); // must wait at least 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}

/// @brief deactivate the given channel.
/// @param N
void ADS1299::deactivateChannel(byte N)
{
    byte setting, startChan, endChan;
    ADS1299::ChipSelect targetSS;

    if (N < 9)
    {
        targetSS = BOARD_ADS;
        startChan = 0;
        endChan = 8;
    }
    else
    {
        if (!daisyPresent)
        {
            return;
        }
        targetSS = DAISY_ADS;
        startChan = 8;
        endChan = 16;
    }
    SDATAC(targetSS);
    delay(1);                                     // exit Read Data Continuous mode to communicate with ADS
    N = constrain(N - 1, startChan, endChan - 1); // subtracts 1 so that we're counting from 0, not 1

    setting = RREG(CH1SET + (N - startChan), targetSS);
    delay(1);             // get the current channel settings
    bitSet(setting, 7);   // set bit7 to shut down channel
    bitClear(setting, 3); // clear bit3 to disclude from SRB2 if used
    WREG(CH1SET + (N - startChan), setting, targetSS);
    delay(1); // write the new value to disable the channel

    // remove the channel from the bias generation...
    setting = RREG(BIAS_SENSP, targetSS);
    delay(1);                         // get the current bias settings
    bitClear(setting, N - startChan); // clear this channel's bit to remove from bias generation
    WREG(BIAS_SENSP, setting, targetSS);
    delay(1); // send the modified byte back to the ADS

    setting = RREG(BIAS_SENSN, targetSS);
    delay(1);                         // get the current bias settings
    bitClear(setting, N - startChan); // clear this channel's bit to remove from bias generation
    WREG(BIAS_SENSN, setting, targetSS);
    delay(1); // send the modified byte back to the ADS

    leadOffSettings[N][0] = leadOffSettings[N][1] = NO;
    changeChannelLeadOffDetect(N + 1);
}

/// @brief read one ADS register
/// @param _address
/// @param targetSS
/// @return
byte ADS1299::RREG(byte _address, ChipSelect targetSS)
{                                   //  reads ONE register at _address
    byte opcode1 = _address + 0x20; //  RREG expects 001rrrrr where rrrrr = _address
    csLow(targetSS);                //  open SPI
    xfer(opcode1);                  //  opcode1
    xfer(0x00);                     //  opcode2
    regData[_address] = xfer(0x00); //  update mirror location with returned byte
    csHigh(targetSS);               //  close SPI
    return regData[_address];       // return requested register value
}

/// @brief write one ADS register
/// @param
/// @param
/// @param
void ADS1299::WREG(byte _address, byte _value, ChipSelect target_SS)
{                                   //  Write ONE register at _address
    byte opcode1 = _address + 0x40; //  WREG expects 010rrrrr where rrrrr = _address
    csLow(target_SS);               //  open SPI
    xfer(opcode1);                  //  Send WREG command & address
    xfer(0x00);                     //  Send number of registers to read -1
    xfer(_value);                   //  Write the value to the register
    csHigh(target_SS);              //  close SPI
    regData[_address] = _value;     //  update the mirror array
}

ADS1299::ADS1299()
    : lastSampleTime(0), sampleCounter(0), curSampleRate(SAMPLE_RATE_250)
{
    // ();
    // softReset();
}

/// @brief 初始化 ADS1299
void ADS1299::initialize()
{
    // 配置 引脚
    pinMode(PIN_ADS_CS1, OUTPUT); // BOARD_ADS ChipSelect Pin
    pinMode(PIN_ADS_CS2, OUTPUT); // DAISY_ADS ChipSelect Pin
    csHigh(BOTH_ADS);

    hspi->begin(PIN_SPI_SCLK, PIN_SPI_MISO, PIN_SPI_MOSI, -1);
    hspi->setFrequency(ADS_SPI_SPEED); // use 4MHz for ADS
    hspi->setDataMode(SPI_MODE3);

    initialize_ads();
}

void ADS1299::setSampleRate(uint8_t newSampleRateCode)
{
    curSampleRate = (SAMPLE_RATE)newSampleRateCode;
    initialize_ads();
}

void ADS1299::initialize_ads()
{
    // recommended power up sequence requiers >Tpor (~32mS)
    delay(50);
    pinMode(PIN_ADS_RESET, OUTPUT);
    digitalWrite(PIN_ADS_RESET, LOW);  // reset pin connected to both ADS ICs
    delayMicroseconds(4);              // toggle reset pin
    digitalWrite(PIN_ADS_RESET, HIGH); // this will reset the Daisy if it is present
    delayMicroseconds(20);             // recommended to wait 18 Tclk before using device (~8uS);
    // initalize the  data ready chip select and reset pins:
    pinMode(PIN_ADS_DRDY, INPUT); // we get DRDY asertion from the on-board ADS
    delay(40);
    resetADS(BOARD_ADS); // reset the on-board ADS registers, and stop DataContinuousMode
    delay(10);
    WREG(CONFIG1, (ADS1299_CONFIG1_DAISY | curSampleRate), BOARD_ADS); // tell on-board ADS to output its clk, set the data rate to 250SPS
    delay(40);
    resetADS(DAISY_ADS); // software reset daisy module if present
    delay(10);
    daisyPresent = smellDaisy(); // check to see if daisy module is present
    if (!daisyPresent)
    {
        WREG(CONFIG1, (ADS1299_CONFIG1_DAISY_NOT | curSampleRate), BOARD_ADS); // turn off clk output if no daisy present
        numChannels = 8;                                                       // expect up to 8 ADS channels
    }
    else
    {
        numChannels = 16;                                                      // expect up to 16 ADS channels
        WREG(CONFIG1, (ADS1299_CONFIG1_DAISY_NOT | curSampleRate), DAISY_ADS); // tell on-board ADS to output its clk, set the data rate to 250SPS
        delay(40);
    }

    // DEFAULT CHANNEL SETTINGS FOR ADS
    defaultChannelSettings[POWER_DOWN] = NO;                  // on = NO, off = YES
    defaultChannelSettings[GAIN_SET] = ADS_GAIN24;            // Gain setting
    defaultChannelSettings[INPUT_TYPE_SET] = ADSINPUT_NORMAL; // input muxer setting
    defaultChannelSettings[BIAS_SET] = YES;                   // add this channel to bias generation
    defaultChannelSettings[SRB2_SET] = YES;                   // connect this P side to SRB2
    defaultChannelSettings[SRB1_SET] = NO;                    // don't use SRB1

    for (int i = 0; i < numChannels; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            channelSettings[i][j] = defaultChannelSettings[j]; // assign default settings
        }
        useInBias[i] = true; // keeping track of Bias Generation
        useSRB2[i] = true;   // keeping track of SRB2 inclusion
    }
    boardUseSRB1 = daisyUseSRB1 = false;

    writeChannelSettings(); // write settings to the on-board and on-daisy ADS if present

    WREG(CONFIG3, 0b11101100, BOTH_ADS);
    delay(1); // enable internal reference drive and etc.
    for (int i = 0; i < numChannels; i++)
    { // turn off the impedance measure signal
        leadOffSettings[i][PCHAN] = OFF;
        leadOffSettings[i][NCHAN] = OFF;
    }
    verbosity = false; // when verbosity is true, there will be Serial feedback
    firstDataPacket = true;

    streaming = false;
}

/// @brief reset all the ADS1299's settings
/// @param targetSS
void ADS1299::resetADS(ChipSelect targetSS)
{
    int startChan, stopChan;
    if (targetSS == BOARD_ADS)
    {
        startChan = 1;
        stopChan = 8;
    }
    if (targetSS == DAISY_ADS)
    {
        startChan = 9;
        stopChan = 16;
    }
    RESET(targetSS);  // send RESET command to default all registers
    SDATAC(targetSS); // exit Read Data Continuous mode to communicate with ADS
    delay(100);
    // turn off all channels
    for (int chan = startChan; chan <= stopChan; chan++)
    {
        deactivateChannel(chan);
    }
}

/// @brief check if daisy present
/// @param
/// @return
boolean ADS1299::smellDaisy(void)
{
    boolean isDaisy = false;
    byte setting = RREG(ID_REG, DAISY_ADS); // try to read the daisy product ID
    if (setting == ADS_ID)
    {
        isDaisy = true;
    } // should read as 0x3E
    return isDaisy;
}

void ADS1299::writeChannelSettings(void)
{
    boolean use_SRB1 = false;
    byte setting, startChan, endChan;
    ChipSelect targetSS;

    for (int b = 0; b < 2; b++)
    {
        if (b == 0)
        {
            targetSS = BOARD_ADS;
            startChan = 0;
            endChan = 8;
        }
        if (b == 1)
        {
            if (!daisyPresent)
            {
                return;
            }
            targetSS = DAISY_ADS;
            startChan = 8;
            endChan = 16;
        }

        SDATAC(targetSS);
        delay(1); // exit Read Data Continuous mode to communicate with ADS

        for (byte i = startChan; i < endChan; i++)
        { // write 8 channel settings
            setting = 0x00;
            if (channelSettings[i][POWER_DOWN] == YES)
            {
                setting |= 0x80;
            }
            setting |= channelSettings[i][GAIN_SET];       // gain
            setting |= channelSettings[i][INPUT_TYPE_SET]; // input code
            if (channelSettings[i][SRB2_SET] == YES)
            {
                setting |= 0x08;   // close this SRB2 switch
                useSRB2[i] = true; // remember SRB2 state for this channel
            }
            else
            {
                useSRB2[i] = false; // rememver SRB2 state for this channel
            }
            WREG(CH1SET + (i - startChan), setting, targetSS); // write this channel's register settings

            // add or remove this channel from inclusion in BIAS generation
            setting = RREG(BIAS_SENSP, targetSS); // get the current P bias settings
            if (channelSettings[i][BIAS_SET] == YES)
            {
                bitSet(setting, i - startChan);
                useInBias[i] = true; // add this channel to the bias generation
            }
            else
            {
                bitClear(setting, i - startChan);
                useInBias[i] = false; // remove this channel from bias generation
            }
            WREG(BIAS_SENSP, setting, targetSS);
            delay(1); // send the modified byte back to the ADS

            setting = RREG(BIAS_SENSN, targetSS); // get the current N bias settings
            if (channelSettings[i][BIAS_SET] == YES)
            {
                bitSet(setting, i - startChan); // set this channel's bit to add it to the bias generation
            }
            else
            {
                bitClear(setting, i - startChan); // clear this channel's bit to remove from bias generation
            }
            WREG(BIAS_SENSN, setting, targetSS);
            delay(1); // send the modified byte back to the ADS

            if (channelSettings[i][SRB1_SET] == YES)
            {
                use_SRB1 = true; // if any of the channel setting closes SRB1, it is closed for all
            }
        } // end of CHnSET and BIAS settings
    }     // end of board select loop
    if (use_SRB1)
    {
        for (int i = startChan; i < endChan; i++)
        {
            channelSettings[i][SRB1_SET] = YES;
        }
        WREG(MISC1, 0x20, targetSS); // close SRB1 swtich
        if (targetSS == BOARD_ADS)
        {
            boardUseSRB1 = true;
        }
        if (targetSS == DAISY_ADS)
        {
            daisyUseSRB1 = true;
        }
    }
    else
    {
        for (int i = startChan; i < endChan; i++)
        {
            channelSettings[i][SRB1_SET] = NO;
        }
        WREG(MISC1, 0x00, targetSS); // open SRB1 switch
        if (targetSS == BOARD_ADS)
        {
            boardUseSRB1 = false;
        }
        if (targetSS == DAISY_ADS)
        {
            daisyUseSRB1 = false;
        }
    }
}

void ADS1299::writeChannelSettings(byte N)
{
    byte setting, startChan, endChan;
    ChipSelect targetSS;

    if (N < 9)
    { // channels 1-8 on board
        targetSS = BOARD_ADS;
        startChan = 0;
        endChan = 8;
    }
    else
    { // channels 9-16 on daisy module
        if (!daisyPresent)
        {
            return;
        }
        targetSS = DAISY_ADS;
        startChan = 8;
        endChan = 16;
    }
    // function accepts channel 1-16, must be 0 indexed to work with array
    N = constrain(N - 1, startChan, endChan - 1); // subtracts 1 so that we're counting from 0, not 1
    // first, disable any data collection
    SDATAC(targetSS);
    delay(1); // exit Read Data Continuous mode to communicate with ADS

    setting = 0x00;
    if (channelSettings[N][POWER_DOWN] == YES)
        setting |= 0x80;
    setting |= channelSettings[N][GAIN_SET];       // gain
    setting |= channelSettings[N][INPUT_TYPE_SET]; // input code
    if (channelSettings[N][SRB2_SET] == YES)
    {
        setting |= 0x08;   // close this SRB2 switch
        useSRB2[N] = true; // keep track of SRB2 usage
    }
    else
    {
        useSRB2[N] = false;
    }
    WREG(CH1SET + (N - startChan), setting, targetSS); // write this channel's register settings

    // add or remove from inclusion in BIAS generation
    setting = RREG(BIAS_SENSP, targetSS); // get the current P bias settings
    if (channelSettings[N][BIAS_SET] == YES)
    {
        useInBias[N] = true;
        bitSet(setting, N - startChan); // set this channel's bit to add it to the bias generation
    }
    else
    {
        useInBias[N] = false;
        bitClear(setting, N - startChan); // clear this channel's bit to remove from bias generation
    }
    WREG(BIAS_SENSP, setting, targetSS);
    delay(1);                             // send the modified byte back to the ADS
    setting = RREG(BIAS_SENSN, targetSS); // get the current N bias settings
    if (channelSettings[N][BIAS_SET] == YES)
    {
        bitSet(setting, N - startChan); // set this channel's bit to add it to the bias generation
    }
    else
    {
        bitClear(setting, N - startChan); // clear this channel's bit to remove from bias generation
    }
    WREG(BIAS_SENSN, setting, targetSS);
    delay(1); // send the modified byte back to the ADS

    // if SRB1 is closed or open for one channel, it will be the same for all channels
    if (channelSettings[N][SRB1_SET] == YES)
    {
        for (int i = startChan; i < endChan; i++)
        {
            channelSettings[i][SRB1_SET] = YES;
        }
        if (targetSS == BOARD_ADS)
            boardUseSRB1 = true;
        if (targetSS == DAISY_ADS)
            daisyUseSRB1 = true;
        setting = 0x20; // close SRB1 swtich
    }
    if (channelSettings[N][SRB1_SET] == NO)
    {
        for (int i = startChan; i < endChan; i++)
        {
            channelSettings[i][SRB1_SET] = NO;
        }
        if (targetSS == BOARD_ADS)
            boardUseSRB1 = false;
        if (targetSS == DAISY_ADS)
            daisyUseSRB1 = false;
        setting = 0x00; // open SRB1 switch
    }
    WREG(MISC1, setting, targetSS);
}

/// @brief change the lead off detect settings for all channels
void ADS1299::changeChannelLeadOffDetect()
{
    byte setting, startChan, endChan;
    ChipSelect targetSS;

    for (int b = 0; b < 2; b++)
    {
        if (b == 0)
        {
            targetSS = BOARD_ADS;
            startChan = 0;
            endChan = 8;
        }
        if (b == 1)
        {
            if (!daisyPresent)
            {
                return;
            }
            targetSS = DAISY_ADS;
            startChan = 8;
            endChan = 16;
        }

        SDATAC(targetSS);
        delay(1); // exit Read Data Continuous mode to communicate with ADS
        byte P_setting = RREG(LOFF_SENSP, targetSS);
        byte N_setting = RREG(LOFF_SENSN, targetSS);

        for (int i = startChan; i < endChan; i++)
        {
            if (leadOffSettings[i][PCHAN] == ON)
            {
                bitSet(P_setting, i - startChan);
            }
            else
            {
                bitClear(P_setting, i - startChan);
            }
            if (leadOffSettings[i][NCHAN] == ON)
            {
                bitSet(N_setting, i - startChan);
            }
            else
            {
                bitClear(N_setting, i - startChan);
            }
            WREG(LOFF_SENSP, P_setting, targetSS);
            WREG(LOFF_SENSN, N_setting, targetSS);
        }
    }
}

/// @brief change the lead off detect settings for specified channel
/// @param N
void ADS1299::changeChannelLeadOffDetect(byte N)
{
    byte setting, startChan, endChan;
    ChipSelect targetSS;

    if (N < 9)
    {
        targetSS = BOARD_ADS;
        startChan = 0;
        endChan = 8;
    }
    else
    {
        if (!daisyPresent)
        {
            return;
        }
        targetSS = DAISY_ADS;
        startChan = 8;
        endChan = 16;
    }

    N = constrain(N - 1, startChan, endChan - 1);
    SDATAC(targetSS);
    delay(1); // exit Read Data Continuous mode to communicate with ADS
    byte P_setting = RREG(LOFF_SENSP, targetSS);
    byte N_setting = RREG(LOFF_SENSN, targetSS);

    if (leadOffSettings[N][PCHAN] == ON)
    {
        bitSet(P_setting, N - startChan);
    }
    else
    {
        bitClear(P_setting, N - startChan);
    }
    if (leadOffSettings[N][NCHAN] == ON)
    {
        bitSet(N_setting, N - startChan);
    }
    else
    {
        bitClear(N_setting, N - startChan);
    }
    WREG(LOFF_SENSP, P_setting, targetSS);
    WREG(LOFF_SENSN, N_setting, targetSS);
}

/// @brief This is a function that can be called multiple times, this is
//         what we refer to as a `soft reset`. You will hear/see this
//         many times.
/// @param
void ADS1299::softReset(void)
{
    this->initialize();
    delay(500);
    configureLeadOffDetection(LOFF_MAG_6NA, LOFF_FREQ_31p2HZ);
    Serial0.println("OpenBCI V3 8-16 channel");
    Serial0.printf("On Board ADS1299 Device ID: 0x%X\n", ADS_getDeviceID(BOARD_ADS));
    if (daisyPresent)
    {
        Serial0.printf("On Daisy ADS1299 Device ID: 0x%X\n", ADS_getDeviceID(DAISY_ADS));
    }
    delay(5);
}

void ADS1299::configureLeadOffDetection(byte amplitudeCode, byte freqCode)
{
    amplitudeCode &= 0b00001100; // only these two bits should be used
    freqCode &= 0b00000011;      // only these two bits should be used

    byte setting;
    ChipSelect targetSS;
    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            targetSS = BOARD_ADS;
        }
        if (i == 1)
        {
            if (!daisyPresent)
            {
                return;
            }
            targetSS = DAISY_ADS;
        }
        setting = RREG(LOFF, targetSS); // get the current bias settings
        // reconfigure the byte to get what we want
        setting &= 0b11110000;    // clear out the last four bits
        setting |= amplitudeCode; // set the amplitude
        setting |= freqCode;      // set the frequency
        // send the config byte back to the hardware
        WREG(LOFF, setting, targetSS);
        delay(1); // send the modified byte back to the ADS
    }
}

byte ADS1299::ADS_getDeviceID(ChipSelect targetSS)
{
    // simple hello world com check
    byte data = RREG(ID_REG, targetSS);
    return data;
}

void ADS1299::boardBeginADSInterrupt(void)
{
    // 配置 DRDY 外部中断
    pinMode(PIN_ADS_DRDY, INPUT); // ADS data ready pin
    attachInterrupt(PIN_ADS_DRDY, ADS_DRDY_Service, RISING);
}

/// @brief CALLED WHEN DRDY PIN IS ASSERTED. NEW ADS DATA AVAILABLE!
/// @param
void ADS1299::updateChannelData(void)
{
    // this needs to be reset, or else it will constantly flag us
    channelDataAvailable = false;

    lastSampleTime = millis();

    boolean downsample = true;

    updateBoardData(downsample);
    if (daisyPresent)
    {
        updateDaisyData(downsample);
    }

    // switch (curBoardMode)
    // {
    // case BOARD_MODE_ANALOG:
    //     auxData[0] = analogRead(A5);
    //     auxData[1] = analogRead(A6);
    //     if (!wifi.present)
    //     {
    //         auxData[2] = analogRead(A7);
    //     }
    //     break;
    // case BOARD_MODE_DIGITAL:
    //     auxData[0] = digitalRead(11) << 8 | digitalRead(12);
    //     auxData[1] = (wifi.present ? 0 : digitalRead(13) << 8) | digitalRead(17);
    //     auxData[2] = wifi.present ? 0 : digitalRead(18);
    //     break;
    // case BOARD_MODE_MARKER:
    //     if (newMarkerReceived)
    //     {
    //         auxData[0] = (short)markerValue;
    //         newMarkerReceived = false;
    //     }
    //     break;
    // case BOARD_MODE_BLE:
    // case BOARD_MODE_DEBUG:
    // case BOARD_MODE_DEFAULT:
    //     break;
    // }
}

void ADS1299::updateBoardData(void)
{
    updateBoardData(true);
}

void ADS1299::updateBoardData(boolean downsample)
{
    byte inByte;
    int byteCounter = 0;

    if ((daisyPresent) && !firstDataPacket && downsample)
    {
        for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
        {                                                        // shift and average the byte arrays
            lastBoardChannelDataInt[i] = boardChannelDataInt[i]; // remember the last samples
        }
    }

    csLow(BOARD_ADS); //  open SPI
    for (int i = 0; i < 3; i++)
    {
        inByte = xfer(0x00); //  read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
        boardStat = (boardStat << 8) | inByte;
    }
    for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
    {
        for (int j = 0; j < OPENBCI_ADS_BYTES_PER_CHAN; j++)
        { //  read 24 bits of channel data in 8 3 byte chunks
            inByte = xfer(0x00);
            boardChannelDataRaw[byteCounter] = inByte; // raw data goes here
            byteCounter++;
            boardChannelDataInt[i] = (boardChannelDataInt[i] << 8) | inByte; // int data goes here
        }
    }
    csHigh(BOARD_ADS); // close SPI

    // need to convert 24bit to 32bit if using the filter
    for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
    { // convert 3 byte 2's compliment to 4 byte 2's compliment
        if (bitRead(boardChannelDataInt[i], 23) == 1)
        {
            boardChannelDataInt[i] |= 0xFF000000;
        }
        else
        {
            boardChannelDataInt[i] &= 0x00FFFFFF;
        }
    }
    if ((daisyPresent) && !firstDataPacket && downsample)
    {
        byteCounter = 0;
        for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
        { // take the average of this and the last sample
            meanBoardChannelDataInt[i] = (lastBoardChannelDataInt[i] + boardChannelDataInt[i]) / 2;
        }
        for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
        { // place the average values in the meanRaw array
            for (int b = 2; b >= 0; b--)
            {
                meanBoardDataRaw[byteCounter] = (meanBoardChannelDataInt[i] >> (b * 8)) & 0xFF;
                byteCounter++;
            }
        }
    }

    if (firstDataPacket == true)
    {
        firstDataPacket = false;
    }
}

/// @brief Read from the Daisy's ADS1299 chip and fill the core arrays with
//         new data. Defaults to downsampling if the daisy is present.
/// @param
void ADS1299::updateDaisyData(void)
{
    updateDaisyData(true);
}

/// @brief Read from the Daisy's ADS1299 chip and fill the core arrays with
//         new data.
/// @param  downsample {boolean} - Averages the last sample with the current to cut the sample rate in half.
void ADS1299::updateDaisyData(boolean downsample)
{
    byte inByte;
    int byteCounter = 0;

    if (daisyPresent && !firstDataPacket && downsample)
    {
        for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
        {                                                        // shift and average the byte arrays
            lastDaisyChannelDataInt[i] = daisyChannelDataInt[i]; // remember the last samples
        }
    }

    // Open SPI
    csLow(DAISY_ADS);
    // Read status register (1100 + LOFF_STATP + LOFF_STATN + GPIO[7:4])
    // TODO: Do we really need to read this status register ever time?
    for (int i = 0; i < 3; i++)
    {
        inByte = xfer(0x00);
        daisyStat = (daisyStat << 8) | inByte;
    }

    // Read 24 bits of channel data in 8 3 byte chunks
    for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
    {
        for (int j = 0; j < OPENBCI_ADS_BYTES_PER_CHAN; j++)
        {
            inByte = xfer(0x00);
            daisyChannelDataRaw[byteCounter] = inByte; // raw data goes here
            byteCounter++;
            daisyChannelDataInt[i] = (daisyChannelDataInt[i] << 8) | inByte; // int data goes here
        }
    }

    // Close SPI
    csHigh(DAISY_ADS);

    // Convert 24bit to 32bit
    for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
    {
        // Convert 3 byte 2's compliment to 4 byte 2's compliment
        if (bitRead(daisyChannelDataInt[i], 23) == 1)
        {
            daisyChannelDataInt[i] |= 0xFF000000;
        }
        else
        {
            daisyChannelDataInt[i] &= 0x00FFFFFF;
        }
    }

    if (daisyPresent && !firstDataPacket && downsample)
    {
        byteCounter = 0;
        // Average this sample with the last sample
        for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
        {
            meanDaisyChannelDataInt[i] = (lastDaisyChannelDataInt[i] + daisyChannelDataInt[i]) / 2;
        }
        // Place the average values in the meanRaw array
        for (int i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; i++)
        {
            for (int b = 2; b >= 0; b--)
            {
                meanDaisyDataRaw[byteCounter] = (meanDaisyChannelDataInt[i] >> (b * 8)) & 0xFF;
                byteCounter++;
            }
        }
    }

    if (firstDataPacket == true)
    {
        firstDataPacket = false;
    }
}

/// @brief Called from the .ino file as the main sender. Driven by board mode,
//         sample number, and ultimately the current packer type.
/// @param
void ADS1299::sendChannelData(void)
{
    // sendChannelDataWifi();
    sendChannelDataSerial();
    sampleCounter++;
}

void ADS1299::sendChannelDataSerial()
{
    Serial0.println("Channel data sent");
}

ADS1299::~ADS1299()
{
}
