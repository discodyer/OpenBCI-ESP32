#include <Arduino.h>
#include "ADS1299.h"
#include "Config.h"

SPIClass * hspi = NULL;

ADS1299 ads1299;
void setup()
{
    hspi = new SPIClass(HSPI);
    Serial0.begin(115200);
    ads1299.boardBeginADSInterrupt();
    ads1299.softReset();
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_IMU_CS, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_IMU_CS, HIGH);
}

void loop()
{
    if(ads1299.channelDataAvailable)
    {
        ads1299.updateBoardData();
        ads1299.sendChannelData();
    }
}
