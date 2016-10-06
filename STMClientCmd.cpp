
#include "STMClientCmd.h"

STMClientCmd::STMClientCmd(STMClient* elc) :_elc(elc) {}

uint32_t STMClientCmd::GetTime() {
  _elc->Request(CMD_GET_TIME, 0, 0);
  _elc->Request();

  STMClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}