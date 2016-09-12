// MiscSTMlaneous commands

#ifndef _STM_CLIENT_CMD_H_
#define _STM_CLIENT_CMD_H_

#include "STMClient.h"
#include "FP.h"

class STMClientCmd {
  public:
    // Constructor
    STMClientCmd(STMClient* elc);
    // Get the current time in seconds since the epoch, 0 if the time is unknown
    uint32_t GetTime();

  private:
    STMClient* _elc;
};
#endif