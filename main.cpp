/**
 * Simple example to demo the STM-Client REST calls
 */
//#include "ST_L152_32MHZ.h"
#include "mbed.h"
#include "DHT.h"

#include <STMClient.h>
#include <STMClientRest.h>
//#include <RCC_config.h>

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
DigitalOut led2(LED2);       
DigitalOut LCD_D7(D7);

//Initialize the sensors
DHT DHTsensor(D4, DHT11);
AnalogIn Lsensor(A2);
// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
STMClient esp(&espSerial, &debugSerial);

// Initialize a REST client on the connection to esp-link
STMClientRest rest(&esp);
bool wifiConnected = false;

Ticker post_data;
bool posted = false;
bool ESPisAsleep = false;

//in the future pass the wait time and the number of flashes per command
void toggleLEDSuccess(){
        
    for(int ii =0; ii > 3; ii++){    
        led2 = 1;
        wait(0.25);        
        led2 = 0;
        wait(0.25);
    }
}
        
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

//resync after sleep mode
void syncESP(){
  
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
    if(packet->value >= 1) { ///ideally this would coincide with STATION_GOT_IP... 
      debugSerial.printf("WIFI CONNECTED\n\r");
      wifiConnected = true;
    } else {
      debugSerial.printf("WIFI NOT READY: %i\n\r",packet->value);
      //Serial.printf(status);
      wifiConnected = false;
    }
  }  
}
//Start/Restart REST...can pass some fancy arguments to this in the future.
void beginREST(){
  
  // Set up the REST client to talk to www.timeapi.org, this doesn't connect to that server,
  // it just sets-up stuff on the esp-link side
  //int err = rest.begin("www.timeapi.org"); //for basic example of get
    const char* host = "api.thingspeak.com";
  //const char* host = "posttestserver.com";  
  //const char* host = "vivaplanetbusservicedev.servicebus.windows.net";
  uint16_t port = 80;
  bool security = true;
  int err = rest.begin(host,port,security); 
  if (err != 0) {
    debugSerial.printf("REST begin failed: %i\n\r",err);
    while(1) ;
  }
  debugSerial.printf("STM-REST ready\n\r");    
}

//generate some random numbers
int random_number(int min_num, int max_num){
    int result=0,low_num=0,hi_num=0;
    if(min_num<max_num)
    {
        low_num=min_num;
        hi_num=max_num+1; // this is done to include max_num in output.
    }else{
        low_num=max_num+1;// this is done to include max_num in output.
        hi_num=min_num;
    }
    srand(time(NULL));
    result = (rand()%(hi_num-low_num))+low_num;
    return result;
 }

#define BUFLEN 100
char response[BUFLEN];

char *loop_get() {
  //debugSerial.printf("begin main loop \n\r");
  // process any callbacks coming from esp_link
  esp.Process();
  debugSerial.printf("Wifi Connected: %i \n\r",wifiConnected);
  // if we're connected make an HTTP request
  while(wifiConnected) {
    // Request /utc/now from the previously set-up server
    rest.get("/utc/now");

    //char response[BUFLEN];
    memset(response, 0, BUFLEN);
    uint16_t code = rest.waitResponse(response, BUFLEN);
    if(code == HTTP_STATUS_OK){
      debugSerial.printf("STM: GET successful: %s\n\r", response);
      wait(60);
      return response;
    } else {
      debugSerial.printf("STM: GET failed: %i\n\r",code);
      wait(60);
      return (char *)code;
    }    
  }
  return "0";   
}
//initialize light variables We don't want these to get reset each time...
float lnow = 0.0f, lmax = 50.00, lmin = 20.00;
int loop_post() {
  posted = false;

  // process any callbacks coming from esp_link
  esp.Process();
  
  debugSerial.printf("Wifi Connected: %i \n\r",wifiConnected);
  // if we're connected make an HTTP request
  if(wifiConnected) {
    //this is where the calls to the sensor drivers should go.
    //ideally they will be blocking so they don't return a value and allow the program to move on until each
    //function is complete    
    
    int error = 0;
    float l = 0.0f, h = 0.0f, c = 0.0f, f = 0.0f, k = 0.0f, dp = 0.0f, dpf = 0.0f;

    wait(2.0f);
    error = DHTsensor.readData();
    if (0 == error || 6 == error) {
        //c   = sensor.ReadTemperature(CELCIUS);
        f   = DHTsensor.ReadTemperature(FARENHEIT);
        //k   = sensor.ReadTemperature(KELVIN);
        h   = DHTsensor.ReadHumidity();
        lnow = Lsensor.read()*100;
        
        if(lnow > lmax)
        {
            lmax = lnow;
        }
        if(lnow < lmin){
            lmin = lnow;                
        }
        
        l = ((lnow - lmin)/(lmax-lmin))*100;
        
        //dp  = sensor.CalcdewPoint(c, h);
        //dpf = sensor.CalcdewPointFast(c, h);
        debugSerial.printf("Temperature in Farenheit %2.2f\n\r", f);
        debugSerial.printf("Humidity is %2.2f\n\r", h);
    } else {
        debugSerial.printf("Error: %d\n", error);
    }
    
    
    debugSerial.printf("geting measurements...\n\r");
    //int Tint = random_number(65, 99);
    
    //int Hint = random_number(20, 25);

    char T[5];
    sprintf(T, "%2.2f", f);
    char L[5];
    sprintf(L, "%2.2f", l);
    char H[5];
    sprintf(H, "%2.2f", h);
    debugSerial.printf("T: %s \n\r",T); //check to make sure the value is the same
    debugSerial.printf("L: %s \n\r",L); //check to make sure the value is the same
    debugSerial.printf("H: %s \n\r",H); //check to make sure the value is the same
            
    /**make sure that the size of this array is consistent with the input string**/ 
    const char* body = "";    
    char output [62];
    sprintf(output, "/update?api_key=3FF5CTKAJIU2IH0M&field1=%s&field2=%s&field3=%s", T,L,H);
    //debugSerial.printf("output: %s \n\r", output);
    //debugSerial.printf("size: %i \n\r", strlen(output));    
    debugSerial.printf("sending data...\n\r");
    rest.post(output, body); //basic post test
      
    char response[BUFLEN];
    memset(response, 0, BUFLEN);
    uint16_t code = rest.waitResponse(response, BUFLEN);
    if(code == HTTP_STATUS_OK){
      debugSerial.printf("STM: POST successful: %s\n\r", response);
    }else {
      debugSerial.printf("STM: POST failed: %i\n\r",code);
      return 1;
    }
  }else{
    debugSerial.printf("STM: wifi NOT connected: %i\n\r",wifiConnected);
    return 1;
  } 
  //debugSerial.printf("Exiting...\n\r");
  return 0;
}

void post() {
    led2 = 1;
    if(ESPisAsleep){      
        debugSerial.printf("syncing...\n\r");
        LCD_D7 = 0;
        wait(0.5);
        LCD_D7 = 1;
        wait(20);
        syncESP();
        ESPisAsleep = false;
        debugSerial.printf("restarting REST...\n\r");        
        beginREST();
    }
    debugSerial.printf("posting...\n\r");
    int postVal = loop_post();
    if(!postVal)
    {
        toggleLEDSuccess(); //flashes the LED to show the post was successful
        posted = true;
    }else{
        debugSerial.printf("error...%i\n\r",postVal);
        led2 = 0; 
        posted = false;
    }
}

int main() {
  //setup
  led1=0;
  LCD_D7 = 1;
  debugSerial.baud(115200);   // the baud rate here needs to match the esp-link config
  espSerial.baud(115200);
  espSerial.printf("*********SunLeaf starting!**************\n\r"); 
  debugSerial.printf("*********SunLeaf starting!**************\n\r");
  
  syncESP(); //sync the ESP
  beginREST(); //resync the REST
    
  post_data.attach(&post, 60.0); // the address of the function to be attached (flip) and the interval (2 seconds)
 
  while(true){      
      if(posted)
      {
        debugSerial.printf("sleeping...\n\r");
        posted = false;
        esp.Sleep();  //put the esp to sleep
        ESPisAsleep = true;
        led2 = 0; //turn off all other power and sensors
        
        sleep();       //then sleep
      }else{
        LCD_D7 = 1;
        led2 = !led2;     
        wait(0.5); // give everything a second to wake up
      }
  } 

}

