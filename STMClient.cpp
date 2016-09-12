#include "STMClient.h"
#include "millis.h"

#define xstr(s) str(s)
#define str(s) #s

#define SLIP_END  0300        // indicates end of packet
#define SLIP_ESC  0333        // indicates byte stuffing
#define SLIP_ESC_END  0334    // ESC ESC_END means END data byte
#define SLIP_ESC_ESC  0335    // ESC ESC_ESC means ESC data byte

//===== Input

// Process a received SLIP message
STMClientPacket* STMClient::protoCompletedCb(void) {
  // the packet starts with a STMClientPacket
  STMClientPacket* packet = (STMClientPacket*)_proto.buf;
  if (_debugEn) {
   // _debug->printf("STMC: got %i @ %i:\n\r 0x%x\n\r 0x%x\n\r 0x%x\n\r", _proto.dataLen,(uint32_t)_proto.buf, packet->cmd, packet->value, packet->argc);

    for (uint16_t i=8; i<_proto.dataLen; i++) 
    {
    //  _debug->printf("%x", *(uint8_t*)(_proto.buf+i));
    }
    //_debug->printf("\n\r");
  }

  // verify CRC
  uint16_t crc = crc16Data(_proto.buf, _proto.dataLen-2, 0);
  //_debug->printf("CRC: %i\n\r",crc);
  //wait(0.5);
  uint16_t resp_crc = *(uint16_t*)(_proto.buf+_proto.dataLen-2);
  //_debug->printf("resp_crc: %i\n\r",resp_crc);
  if (crc != resp_crc) {
    DBG("STMC: Invalid CRC\n\r");
    wait(0.5);
    
    //return NULL; maybe the CRC isn't getting calculated correctly...
  }

  // dispatch based on command
  if (packet->cmd == CMD_RESP_V) {
    // value response
    _debug->printf("RESP_V: 0x%x \n\r",packet->value);

    return packet;
  } else if (packet->cmd == CMD_RESP_CB) {
    FP<void, void*> *fp;
    // callback reponse
    _debug->printf("RESP_CB: 0x%x 0x%x \n\r", packet->value, packet->argc);

    fp = (FP<void, void*>*)packet->value;
    if (fp->attached()) {
      STMClientResponse resp(packet);
      (*fp)(&resp);
    }
    return NULL;
  } else {
    // command (NOT IMPLEMENTED)
    _debug->printf("CMD 0x%x Value 0x%x ??\n\r", packet->cmd, packet->value);
    return NULL;
  }
}

// Read all characters available on the serial input and process any messages that arrive, but
// stop if a non-callback response comes in
STMClientPacket *STMClient::Process() {
  while (_serial->readable()) {
    //value =_serial->getc(rxBuffer, 5, eSerialCb, SERIAL_EVENT_RX_ALL, '$');
    char character=_serial->getc(); 
    //value = _serial->read();
    
    if ((int)character == SLIP_ESC) {
      _proto.isEsc = 1;
    } else if ((int)character == SLIP_END) {
      STMClientPacket *packet = _proto.dataLen >= 8 ? protoCompletedCb() : 0;
      _proto.dataLen = 0;
      _proto.isEsc = 0;
      if (packet != NULL) return packet;
    } else {
      if (_proto.isEsc) {
        if ((int)character == SLIP_ESC_END) character = SLIP_END;
        if ((int)character == SLIP_ESC_ESC) character = SLIP_ESC;
        _proto.isEsc = 0;
      }
      if (_proto.dataLen < _proto.bufSize) {
        _proto.buf[_proto.dataLen++] = (int)character;
      }
    }
  }
  return NULL;
}

//===== Output

// Write a byte to the output stream and perform SLIP escaping
void STMClient::write(uint8_t data) {
  switch (data) {
  case SLIP_END:
    _serial->putc(SLIP_ESC);
    _serial->putc(SLIP_ESC_END);
    break;
  case SLIP_ESC:
    _serial->putc(SLIP_ESC);
    _serial->putc(SLIP_ESC_ESC);
    break;
  default:
    _serial->putc(data);
  }
}

// Write some bytes to the output stream
void STMClient::write(void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;
  while (len--)
    write(*d++);
}

// Start a request. cmd=command, value=address of callback pointer or first arg,
// argc=additional argument count
void STMClient::Request(uint16_t cmd, uint32_t value, uint16_t argc) {
  //_debug->printf("Starting a request...\n\r");
  //wait(0.5);
  crc = 0;
  _serial->putc(SLIP_END);
    
  write(&cmd, 2);
  crc = crc16Data((unsigned const char*)&cmd, 2, crc);

  write(&argc, 2);
  crc = crc16Data((unsigned const char*)&argc, 2, crc);

  write(&value, 4);
  crc = crc16Data((unsigned const char*)&value, 4, crc);
}

// Append a block of data as an argument to the request
void STMClient::Request(const void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;

  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  for (uint16_t l=len; l>0; l--) {
    write(*d);
    crc = crc16Add(*d, crc);
    d++;
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

/* Commented this out for now...
// Append a block of data located in flash as an argument to the request
void STMClient::Request(const __FlashStringHSTMper* data, uint16_t len) {
  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  PGM_P p = reinterpret_cast<PGM_P>(data);
  for (uint16_t l=len; l>0; l--) {
    uint8_t c = pgm_read_byte(p++);
    write(c);
    crc = crc16Add(c, crc);
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}
*/

// Append the final CRC to the request and finish the request
void STMClient::Request(void) {
  write((uint8_t*)&crc, 2);
  _serial->putc(SLIP_END);
}

//===== Initialization

void STMClient::init() {
  _proto.buf = _protoBuf;
  _proto.bufSize = sizeof(_protoBuf);
  _proto.dataLen = 0;
  _proto.isEsc = 0;
}

STMClient::STMClient(Serial* serial) :
_serial(serial) {
  _debugEn = false;
  init();
}

STMClient::STMClient(Serial* serial, Serial* debug) :
_debug(debug), _serial(serial) {
  _debugEn = true;
  init();
}

void STMClient::DBG(const char* info) {
  if (_debugEn) _debug->printf(info);
}

//===== Responses

// Wait for a response for a given timeout
STMClientPacket *STMClient::WaitReturn(uint32_t timeout) {
  uint32_t wait = millis();
  while (millis() - wait < timeout) {
    STMClientPacket *packet = Process();
    if (packet != NULL) return packet;
  }
  return NULL;
}

//===== CRC hSTMper functions

uint16_t STMClient::crc16Add(unsigned char b, uint16_t acc)
{
  acc ^= b;
  acc = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}

uint16_t STMClient::crc16Data(const unsigned char *data, uint16_t len, uint16_t acc)
{
  for (uint16_t i=0; i<len; i++)
    acc = crc16Add(*data++, acc);
  return acc;
}

//===== Basic requests built into STMClient

bool STMClient::Sync(uint32_t timeout) {
  //_debug->printf("syncing...");
  wait(0.5);
  // send sync request
  Request(CMD_SYNC, (uint32_t)&wifiCb, 0);
  Request();
  
  // empty the response queue hoping to find the wifiCb address
  STMClientPacket *packet;
  while ((packet = WaitReturn(timeout)) != NULL) {
    if (packet->value == (uint32_t)&wifiCb) 
    { 
    //    _debug->printf("SYNC!");  
    //    wait(0.5); 
        return true; 
    }
    _debug->printf("BAD: %s /n/r", packet->value);
  }
  
  // doesn't look like we got a real response
  return false;
}

void STMClient::GetWifiStatus(void) {
  Request(CMD_WIFI_STATUS, 0, 0);
  Request();
}