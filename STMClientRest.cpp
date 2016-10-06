#include "STMClientRest.h"
#include "millis.h"

typedef enum {
  HEADER_GENERIC = 0,
  HEADER_CONTENT_TYPE,
  HEADER_USER_AGENT
} HEADER_TYPE;

STMClientRest::STMClientRest(STMClient *e)
{
  _elc = e;
  remote_instance = -1;
}

void STMClientRest::restCallback(void *res)
{
  if (!res) return;

  STMClientResponse *resp = (STMClientResponse *)res;

  resp->popArg(&_status, sizeof(_status));
  _elc->_debug->printf("REST code %i \n\r",_status);

  _len = resp->popArgPtr(&_data);
}

int STMClientRest::begin(const char* host, uint16_t port, bool security)
{
  uint8_t sec = !!security;
  restCb.attach(this, &STMClientRest::restCallback);

  _elc->Request(CMD_REST_SETUP, (uint32_t)&restCb, 3);
  _elc->Request(host, strlen(host));
  _elc->Request(&port, 2);
  _elc->Request(&sec, 1);
  _elc->Request();

  STMClientPacket *pkt = _elc->WaitReturn();
  if (pkt && (int32_t)pkt->value >= 0) {
    remote_instance = pkt->value;
    return 0;
  }
  return (int)pkt->value;
}

void STMClientRest::request(const char* path, const char* method, const char* data, int len)
{
  _status = 0;
  if (remote_instance < 0) return;
  if (data != 0 && len > 0) _elc->Request(CMD_REST_REQUEST, remote_instance, 3);
  else                      _elc->Request(CMD_REST_REQUEST, remote_instance, 2);
  _elc->Request(method, strlen(method));
  _elc->Request(path, strlen(path));
  if (data != NULL && len > 0) {
    _elc->Request(data, len);
  }

  _elc->Request();
}

void STMClientRest::request(const char* path, const char* method, const char* data)
{
  request(path, method, data, strlen(data));
}

void STMClientRest::get(const char* path, const char* data) { request(path, "GET", data); }
void STMClientRest::post(const char* path, const char* data) { request(path, "POST", data); }
void STMClientRest::put(const char* path, const char* data) { request(path, "PUT", data); }
void STMClientRest::del(const char* path) { request(path, "DELETE", 0); }

void STMClientRest::setHeader(const char* value)
{
  uint8_t header_index = HEADER_GENERIC;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

void STMClientRest::setContentType(const char* value)
{
  uint8_t header_index = HEADER_CONTENT_TYPE;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

void STMClientRest::setUserAgent(const char* value)
{
  uint8_t header_index = HEADER_USER_AGENT;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

uint16_t STMClientRest::getResponse(char* data, uint16_t maxLen)
{
  if (_status == 0) return 0;
  memcpy(data, _data, _len>maxLen?maxLen:_len);
  int16_t s = _status;
  _status = 0;
  return s;
}

uint16_t STMClientRest::waitResponse(char* data, uint16_t maxLen, uint32_t timeout)
{
  uint32_t wait = millis();
  while (_status == 0 && (millis() - wait < timeout)) {
    _elc->Process();
  }
  return getResponse(data, maxLen);
}