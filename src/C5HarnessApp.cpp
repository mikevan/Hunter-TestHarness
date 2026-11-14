#if defined(LAYERTIME_HARNESS_APP)
#include <Arduino.h>
#include <esp_random.h>
#include <SD.h>
#include "C5Link.h"
#include "C5Display.h"
#include "C5Led.h"
#include "TDongleC5Pins.h"
#include "BringUpLogic.h"
#include "HarnessScenario.h"
using namespace layertime::tdongle_c5;
namespace {
C5Link linkServer; C5Display display; C5Led led; ButtonDebouncer button;uint32_t lastDisplayTransfers=0;
uint32_t runId=0;
File sourceLog;bool sdMounted=false,sdError=false;uint32_t logRows=0;String logName;String serialCommand;
void startLog(uint32_t id) {
    if(!sdMounted)return;
    char path[40];snprintf(path,sizeof(path),"/harness_%08lx.csv",static_cast<unsigned long>(id));logName=path;
    if(SD.exists(path)){sdError=true;return;}
    sourceLog=SD.open(path,FILE_WRITE);sdError=!sourceLog;
    if(sourceLog){sourceLog.println("run_id,session,sequence,source_elapsed_ms,stage,notify_accepted,frame_hex");sourceLog.flush();}
}
void recordSource(const uint8_t *b,uint16_t session,uint16_t sequence,uint32_t elapsed) {
    if(!sourceLog || sdError)return;
    char hex[41];for(int i=0;i<20;i++)snprintf(hex+2*i,3,"%02X",b[i]);
    char row[180];const int n=snprintf(row,sizeof(row),"%lu,%u,%u,%lu,%u,1,%s\n",
        static_cast<unsigned long>(runId),session,sequence,static_cast<unsigned long>(elapsed),b[2],hex);
    if(n<=0 || sourceLog.write(reinterpret_cast<const uint8_t*>(row),n)!=static_cast<size_t>(n)){sdError=true;sourceLog.close();return;}
    sourceLog.flush();if(sourceLog.getWriteError()){sdError=true;sourceLog.close();return;}++logRows;
}
bool running=false,wasConnected=false; uint32_t started=0,lastSent=0;uint16_t sequence=0;
const char* name(uint8_t s) {switch(s){case 1:return "Baseline";case 2:return "WiFi/BLE drop";case 3:return "Outage verified";case 4:return "Drone signature";default:return "Data complete";}}
void line(const char* state) {display.setLine(0,"Hunter-Test");display.setLine(1,state);display.setLine(3,sdError?"SD ERROR":(sdMounted?"SD ready":"No SD card"));display.setLine(4,running?"Running":"Press BOOT");}
}
void setup() {
    Serial.begin(115200);Serial.setTxTimeoutMs(2);
    pinMode(pins::kSdCs,OUTPUT);digitalWrite(pins::kSdCs,HIGH);
    pinMode(pins::kBootButton,INPUT);display.begin();
    sdMounted=SD.begin(pins::kSdCs,SPI,4000000,"/sd",4,false);led.begin();led.off();
    const bool ok=linkServer.begin(harness::kCapability|layertime::link::kCapDisplay|layertime::link::kCapButton);
    line(ok?"Connect Vixen":"BLE failed");
    Serial.printf("Hunter-TestHarness name=%s session=%u\n",linkServer.advertisedName(),linkServer.sessionId());
}
void loop() {
    const uint32_t now=millis();linkServer.service(now);
    if(button.update(digitalRead(pins::kBootButton)==LOW,now) && !running) {
        if(linkServer.connected()) {runId=esp_random();if(!runId)runId=1;started=now;lastSent=now-1000;sequence=0;logRows=0;sdError=false;startLog(runId);running=true;line("Starting");}
        else line("Connect Vixen");
    }
    if(wasConnected && !linkServer.connected()) {running=false;if(sourceLog){sourceLog.flush();sourceLog.close();}line("Link lost");}
    wasConnected=linkServer.connected();
    if(running && now-lastSent>=1000) {
        if(sequence==0)started=now; // Clock starts only when the first frame can be delivered.
        const uint32_t elapsed=now-started;uint8_t bytes[20];harness::frame(bytes,runId,elapsed,sequence+1);
        if(sourceLog && !sdError)bytes[3]|=4;if(sdError)bytes[3]|=8;
        if(linkServer.sendHarness(bytes,20)) {
            ++sequence;recordSource(bytes,linkServer.sessionId(),sequence,elapsed);lastSent=now;line(name(bytes[2]));
            char clock[24];snprintf(clock,sizeof(clock),"t+%lus",static_cast<unsigned long>(elapsed/1000));display.setLine(2,clock);
            Serial.printf("HARNESS run=%lu seq=%u stage=%u elapsed=%lu\n",static_cast<unsigned long>(runId),sequence,bytes[2],static_cast<unsigned long>(elapsed));
            if(elapsed>=61000){running=false;if(sourceLog){sourceLog.flush();sourceLog.close();}line("Ready again");}
        }
    }
    while(Serial.available()) {
        char c=Serial.read();if(c=='\n') {
            serialCommand.trim();
            if(serialCommand=="INFO")Serial.printf("HARNESS_INFO mounted=%u sd_error=%u rows=%lu last=%s\n",sdMounted,sdError,static_cast<unsigned long>(logRows),logName.c_str());
            else if(serialCommand=="READ-LAST" && !running && sdMounted && logName.length()) {
                File file=SD.open(logName);Serial.println("SD_BEGIN");
                while(file.available()){uint8_t b[128];size_t n=file.read(b,sizeof(b));Serial.write(b,n);}file.close();led.off();Serial.println("SD_END");
            }
            serialCommand="";
        } else if(serialCommand.length()<80)serialCommand+=c;
    }
    display.service();if(display.transfers()!=lastDisplayTransfers){led.off();lastDisplayTransfers=display.transfers();}delay(5);
}
#endif
