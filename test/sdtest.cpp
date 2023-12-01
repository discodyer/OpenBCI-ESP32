// 引用相关库
// #include "FS.h"
#include "SD_MMC.h"

// 接口连接如下：
// SD卡 - ESP32
// ------------
// DAT2 - IO12
// DAT3 - IO13
// CMD  - IO15
// CLK  - IO14
// DAT0 - IO2
// DAT1 - IO4

void setup()
{
    Serial0.begin(115200);
    Serial0.println();

    SD_MMC.setPins(9, 16, 8, 18, 7, 15);

    // 挂载文件系统
    if (!SD_MMC.begin())
    {
        Serial0.println("存储卡挂载失败");
        return;
    }
    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial0.println("未连接存储卡");
        return;
    }
    else if (cardType == CARD_MMC)
    {
        Serial0.println("挂载了MMC卡");
    }
    else if (cardType == CARD_SD)
    {
        Serial0.println("挂载了SDSC卡");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial0.println("挂载了SDHC卡");
    }
    else
    {
        Serial0.println("挂载了未知存储卡");
    }

    // 打开/建立 并写入数据
    File file = SD_MMC.open("/test.txt", FILE_WRITE);
    if (file)
    {
        Serial0.println("打开/建立 根目录下 test.txt 文件！");
    }

    char data[] = "hello world\r\n";
    file.write((uint8_t *)data, strlen(data));
    file.close();

    // 重命名文件
    if (SD_MMC.rename("/test.txt", "/retest.txt"))
    {
        Serial0.println("test.txt 重命名为 retest.txt ！");
    }

    // 读取文件数据
    file = SD_MMC.open("/retest.txt", FILE_READ);
    if (file)
    {
        Serial0.print("文件内容是：");
        while (file.available())
        {
            Serial0.print((char)file.read());
        }
    }

    // 打印存储卡信息
    Serial0.printf("存储卡总大小是： %lluMB \n", SD_MMC.cardSize() / (1024 * 1024)); // "/ (1024 * 1024)"可以换成">> 20"
    Serial0.printf("文件系统总大小是： %lluB \n", SD_MMC.totalBytes());
    Serial0.printf("文件系统已用大小是： %lluB \n", SD_MMC.usedBytes());
}

void loop()
{
}