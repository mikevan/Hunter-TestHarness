#if defined(LAYERTIME_SD_MAINTENANCE)
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <esp_mac.h>
namespace {
char deviceId[13]={0};String confirmation;
bool correct=false,mounted=false;String command;uint32_t erasedFiles=0,erasedDirs=0;
void ledOff() {
    uint8_t off[12]={0,0,0,0,0xe0,0,0,0,0,0,0,0};
    digitalWrite(10,HIGH);digitalWrite(23,HIGH);
    SPI.beginTransaction(SPISettings(1000000,MSBFIRST,SPI_MODE0));SPI.writeBytes(off,sizeof(off));SPI.endTransaction();
}
void info() {Serial.printf("SD_MAINT device=%s matched=%u mounted=%u bytes=%llu used=%llu\n",deviceId,correct,mounted,
    mounted?SD.cardSize():0,mounted?SD.usedBytes():0);}
bool clearDirectory(const String &path,uint8_t depth) {
    if(depth>12 || !path.startsWith("/") || path.indexOf("..")>=0)return false;
    File dir=SD.open(path);if(!dir || !dir.isDirectory())return false;
    for(;;) {
        File entry=dir.openNextFile();if(!entry)break;
        String child=entry.path();bool directory=entry.isDirectory();entry.close();
        if(!child.startsWith("/") || child=="/" || child.indexOf("..")>=0){dir.close();return false;}
        bool ok=directory?(clearDirectory(child,depth+1)&&SD.rmdir(child)):SD.remove(child);
        if(!ok){dir.close();Serial.printf("REMOVE_FAILED %s\n",child.c_str());return false;}
        if(directory)++erasedDirs;else ++erasedFiles;
    }
    dir.close();return true;
}
void clearCard() {
    if(!correct || !mounted){Serial.println("CLEAR_REFUSED wrong device or no card");return;}
    erasedFiles=erasedDirs=0;
    bool ok=clearDirectory("/",0);
    File root=SD.open("/");File entry=root.openNextFile();bool empty=!entry;entry.close();root.close();
    if(ok && empty) {
        File probe=SD.open("/write-check.tmp",FILE_WRITE);
        bool writable=probe && probe.print("card writable")==13;probe.close();
        bool removed=SD.remove("/write-check.tmp");
        File check=SD.open("/");File remaining=check.openNextFile();empty=!remaining;remaining.close();check.close();
        Serial.printf("CLEAR_RESULT success=%u empty=%u writable=%u files=%lu directories=%lu bytes=%llu used=%llu\n",
            writable&&removed&&empty,empty,writable,static_cast<unsigned long>(erasedFiles),static_cast<unsigned long>(erasedDirs),SD.cardSize(),SD.usedBytes());
    } else Serial.printf("CLEAR_RESULT success=0 empty=%u files=%lu directories=%lu\n",empty,
        static_cast<unsigned long>(erasedFiles),static_cast<unsigned long>(erasedDirs));
}
}
void setup() {
    Serial.begin(115200);Serial.setTxTimeoutMs(10);
    uint8_t mac[6];correct=esp_read_mac(mac,ESP_MAC_BASE)==ESP_OK;
    if(correct){for(int i=0;i<6;i++)snprintf(deviceId+2*i,3,"%02X",mac[i]);confirmation=String("CLEAR-SD-")+deviceId;}
    pinMode(10,OUTPUT);digitalWrite(10,HIGH);pinMode(23,OUTPUT);digitalWrite(23,HIGH);
    SPI.begin(6,7,2,-1);ledOff();
    if(correct)mounted=SD.begin(23,SPI,4000000,"/sd",16,false);
    ledOff();delay(500);info();Serial.printf("READY INFO or %s\n",confirmation.c_str());
}
void loop() {
    while(Serial.available()) {
        const char c=Serial.read();if(c=='\n') {
            command.trim();if(command=="INFO")info();else if(correct && command==confirmation)clearCard();else Serial.println("UNKNOWN_COMMAND");command="";
        } else if(command.length()<80)command+=c;
    }
    static uint32_t lastOff=0;if(millis()-lastOff>=100){ledOff();lastOff=millis();}
    delay(5);
}
#endif
