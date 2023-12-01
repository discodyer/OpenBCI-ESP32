#pragma once

// Pin Definations
#define PIN_SPI_MISO 47
#define PIN_SPI_MOSI 21
#define PIN_SPI_SCLK 14
#define PIN_ADS_RESET 11 // ADS reset pin
#define PIN_ADS_DRDY 48  // ADS data ready pin
#define PIN_ADS_CS1 13   // ADS chip select
#define PIN_ADS_CS2 12   // ADS Daisy chip select

// ADS1299 SPI Command Definition Byte Assignments
#define _WAKEUP 0x02  // Wake-up from standby mode
#define _STANDBY 0x04 // Enter Standby mode
#define _RESET 0x06   // Reset the device registers to default
#define _START 0x08   // Start and restart (synchronize) conversions
#define _STOP 0x0A    // Stop conversion
#define _RDATAC 0x10  // Enable Read Data Continuous mode (default mode at power-up)
#define _SDATAC 0x11  // Stop Read Data Continuous mode
#define _RDATA 0x12   // Read data by command supports multiple read back

// ASD1299 Register Addresses
#define ADS_ID 0x3E // product ID for ADS1299
#define ID_REG 0x00 // this register contains ADS_ID
#define CONFIG1 0x01
#define CONFIG2 0x02
#define CONFIG3 0x03
#define LOFF 0x04
#define CH1SET 0x05
#define CH2SET 0x06
#define CH3SET 0x07
#define CH4SET 0x08
#define CH5SET 0x09
#define CH6SET 0x0A
#define CH7SET 0x0B
#define CH8SET 0x0C
#define BIAS_SENSP 0x0D
#define BIAS_SENSN 0x0E
#define LOFF_SENSP 0x0F
#define LOFF_SENSN 0x10
#define LOFF_FLIP 0x11
#define LOFF_STATP 0x12
#define LOFF_STATN 0x13
#define GPIO 0x14
#define MISC1 0x15
#define MISC2 0x16
#define CONFIG4 0x17

#define OUTPUT_NOTHING (0) // quiet
#define OUTPUT_8_CHAN (1)  // not using Daisy module
#define OUTPUT_16_CHAN (2) // using Daisy module
#define ON_BOARD (8)       // slave address for on board ADS
#define ON_DAISY (3)       // slave address for daisy ADS
// CHANNEL SETTINGS
#define POWER_DOWN (0)
#define GAIN_SET (1)
#define INPUT_TYPE_SET (2)
#define BIAS_SET (3)
#define SRB2_SET (4)
#define SRB1_SET (5)
#define YES (0x01)
#define NO (0x00)

// gainCode choices
#define ADS_GAIN01 (0b00000000) // 0x00
#define ADS_GAIN02 (0b00010000) // 0x10
#define ADS_GAIN04 (0b00100000) // 0x20
#define ADS_GAIN06 (0b00110000) // 0x30
#define ADS_GAIN08 (0b01000000) // 0x40
#define ADS_GAIN12 (0b01010000) // 0x50
#define ADS_GAIN24 (0b01100000) // 0x60

// inputType choices
#define ADSINPUT_NORMAL (0b00000000)
#define ADSINPUT_SHORTED (0b00000001)
#define ADSINPUT_BIAS_MEAS (0b00000010)
#define ADSINPUT_MVDD (0b00000011)
#define ADSINPUT_TEMP (0b00000100)
#define ADSINPUT_TESTSIG (0b00000101)
#define ADSINPUT_BIAS_DRP (0b00000110)
#define ADSINPUT_BIAL_DRN (0b00000111)

// test signal choices...ADS1299 datasheet page 41
#define ADSTESTSIG_AMP_1X (0b00000000)
#define ADSTESTSIG_AMP_2X (0b00000100)
#define ADSTESTSIG_PULSE_SLOW (0b00000000)
#define ADSTESTSIG_PULSE_FAST (0b00000001)
#define ADSTESTSIG_DCSIG (0b00000011)
#define ADSTESTSIG_NOCHANGE (0b11111111)

#define ADS1299_CONFIG1_DAISY (0b10110000)
#define ADS1299_CONFIG1_DAISY_NOT (0b10010000)

// Lead-off signal choices
#define LOFF_MAG_6NA (0b00000000)
#define LOFF_MAG_24NA (0b00000100)
#define LOFF_MAG_6UA (0b00001000)
#define LOFF_MAG_24UA (0b00001100)
#define LOFF_FREQ_DC (0b00000000)
#define LOFF_FREQ_7p8HZ (0b00000001)
#define LOFF_FREQ_31p2HZ (0b00000010)
#define LOFF_FREQ_FS_4 (0b00000011)
#define PCHAN (0)
#define NCHAN (1)
#define OFF (0)
#define ON (1)

// used for channel settings
#define ACTIVATE_SHORTED (2)
#define ACTIVATE (1)
#define DEACTIVATE (0)

#define PCKT_START 0xA0 // prefix for data packet error checking
#define PCKT_END 0xC0   // postfix for data packet error checking

#define ADS_SPI_SPEED (4000000)

/** Possible number of channels */
#define OPENBCI_NUMBER_OF_CHANNELS_DAISY 16
#define OPENBCI_NUMBER_OF_CHANNELS_DEFAULT 8

#define OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE 24
#define OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE 24

/** Helpful numbers */
#define OPENBCI_NUMBER_OF_BOARD_SETTINGS 1
#define OPENBCI_NUMBER_OF_CHANNEL_SETTINGS 6
#define OPENBCI_NUMBER_OF_LEAD_OFF_SETTINGS 2

#define OPENBCI_ADS_BYTES_PER_CHAN 3
#define OPENBCI_ADS_CHANS_PER_BOARD 8