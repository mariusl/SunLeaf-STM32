#include "STMClientResponse.h"

STMClientResponse::STMClientResponse(STMClientPacket* packet) {
  _cmd = packet;
  _arg_ptr = _cmd->args;
  _arg_num = 0;
}

STMClientResponse::STMClientResponse(void* packet) {
  _cmd = (STMClientPacket *)packet;
  _arg_ptr = _cmd->args;
  _arg_num = 0;
}

int16_t STMClientResponse::popArgPtr(void **data) {
  if (_arg_num >= _cmd->argc) return -1;

  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  *data = _arg_ptr;
  _arg_ptr += len + pad;
  return len;
}

int16_t STMClientResponse::popArg(void* d, uint16_t maxLen) {
  if (_arg_num >= _cmd->argc) return -1;

  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  uint8_t *data = (uint8_t *)d;
  uint16_t l = len > maxLen ? maxLen : len;
  uint8_t *p = _arg_ptr;
  while (l--)
    *data++ = *p++;

  _arg_ptr += len + pad;
  return len;
}

void STMClientResponse::popChar(char* buffer) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  uint8_t i;
  for (i = 0; i < len; i++) {
    buffer[i] = (char)*_arg_ptr++;
  }
  buffer[i] = '\0';

  _arg_ptr += pad;
}

string STMClientResponse::popString() {
  string ret;
  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  while (len--)
    ret += (char)*_arg_ptr++;

  _arg_ptr += pad;
  return ret;
}

void STMClientResponse::popString(string* data) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  while (len--)
    //string.append((char)*_arg_ptr++);
    //strcat (data,(char)*_arg_ptr++);
    data += (char)*_arg_ptr++;

  _arg_ptr += pad;
}
