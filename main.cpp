/**
 * Simple example to demo the STM-Client REST calls
 */
#include "mbed.h"
#include <STMClient.h>
#include <STMClientRest.h>

/*-------- Check if platform compatible ----------*/
#if DEVICE_SERIAL_ASYNCH
Serial debugSerial(SERIAL_TX, SERIAL_RX);
Serial espSerial(PA_0, PA_1);
#else
    #warning "Platform not compatible with Low Power APIs for Serial"
    Serial debugSerial(SERIAL_TX, SERIAL_RX);
    Serial espSerial(PA_0, PA_1);
#endif


DigitalOut led1(LED1);             

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
STMClient esp(&espSerial, &debugSerial);


// Initialize a REST client on the connection to esp-link
STMClientRest rest(&esp);

bool wifiConnected = false;

// Callback made from esp-link to notify of wifi status changes
// Here we print something out and set a global flag
void wifiCb(void *response) {
  debugSerial.printf("waiting for wifi status...\n\r"); //debug
  STMClientResponse *res = (STMClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);
    debugSerial.printf("waiting for wifi status...\n\r");
    if(status == STATION_GOT_IP) {
      debugSerial.printf("WIFI CONNECTED");
      wifiConnected = true;
    } else {
      debugSerial.printf("WIFI NOT READY: %i",status);
      //Serial.printf(status);
      wifiConnected = false;
    }
  }
}

#define BUFLEN 266

int loop() {
  //debugSerial.printf("begin main loop \n\r");
  // process any callbacks coming from esp_link
  esp.Process();
  debugSerial.printf("Wifi Connected: %i \n\r",wifiConnected);
  // if we're connected make an HTTP request
  while(wifiConnected) {
    // Request /utc/now from the previously set-up server
    rest.get("/utc/now");

    char response[BUFLEN];
    memset(response, 0, BUFLEN);
    uint16_t code = rest.waitResponse(response, BUFLEN);
    if(code == HTTP_STATUS_OK){
      debugSerial.printf("STM: GET successful: %s\n\r", response);
    } else {
      debugSerial.printf("STM: GET failed: %i\n\r",code);
      return code;
    }
    wait(1);
  }
  return 0; 
  
}

int main() {
  led1=0;
  debugSerial.baud(115200);   // the baud rate here needs to match the esp-link config
  espSerial.baud(115200);
  
  debugSerial.printf("STM-Client starting!\n\r");
  wait(0.5);
  
  espSerial.printf("STM-Client starting!\n\r");
  wait(0.5);

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
      //debugSerial.printf("main syncing..\n\r");
      wait(0.5);
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok){ 
        debugSerial.printf("STM-Client sync failed!\n\r");
        wait(0.5);
        }
  } while(!ok);
  
  debugSerial.printf("STM-Client synced!\n\r");

  // Get immediate wifi status info for demo purposes. This is not normally used because the
  // wifi status callback registered above gets called immediately. 
  esp.GetWifiStatus();
  STMClientPacket *packet;
  if ((packet=esp.WaitReturn()) != NULL) 
  {
    //debugSerial.printf("Wifi status: %i\n\r", packet->value);
    //debugSerial.printf("waiting for wifi status...\n\r");
    if(packet->value == 2) { ///ideally this would coincide with STATION_GOT_IP... 
      debugSerial.printf("WIFI CONNECTED\n\r");
      wifiConnected = true;
    } else {
      debugSerial.printf("WIFI NOT READY: %i\n\r",packet->value);
      //Serial.printf(status);
      wifiConnected = false;
    }
  }
  

  // Set up the REST client to talk to www.timeapi.org, this doesn't connect to that server,
  // it just sets-up stuff on the esp-link side
  int err = rest.begin("www.timeapi.org");
  if (err != 0) {
    debugSerial.printf("REST begin failed: %i\n\r",err);
    while(1) ;
  }
  debugSerial.printf("STM-REST ready\n\r");
  int loopStat = 0;
  while(loopStat == 0){
      debugSerial.printf("status: %i\n\r",loopStat);
      loopStat = loop();
      wait(1);
    } 
}