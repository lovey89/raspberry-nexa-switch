#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "gpio.h"
#include "../constants.h"

#define GROUP_CODE_OFFSET    6
#define GROUP_CODE_MASK      0x03FFFFFF

#define GROUP_COMMAND_OFFSET 5
#define GROUP_COMMAND_MASK   0x00000001

#define ACTIVATE_OFFSET      4
#define ACTIVATE_MASK        0x00000001

#define DEVICE_CODE_OFFSET   0
#define DEVICE_CODE_MASK     0x0000000F

struct timespec bitSleepTime;
struct timespec resendSleepTime;
struct timespec remTime;

/* PRIVATE FUNCTION DECLARATIONS */
void Nexa__sendStartBits();
void Nexa__sendStopBits();
void Nexa__sendLow(char count);
void Nexa__sendHigh(char count);
void Nexa__sendOne();
void Nexa__sendZero();
int32_t Nexa__nexaCode(int32_t groupCode, bool groupCommand, bool activate, char deviceCode);
char boolToBit(bool boolVar);

void Nexa__setup()
{
  // Set up gpi pointer for direct register access
  Gpio__setup();

  // Switch the chosen data pin (DATA_PIN) to output mode
  INP_GPIO(DATA_PIN); // must use INP_GPIO before we can use OUT_GPIO
  OUT_GPIO(DATA_PIN);

  resendSleepTime.tv_sec = 0;
  resendSleepTime.tv_nsec = 10000000; //10ms

  bitSleepTime.tv_sec = 0;
  bitSleepTime.tv_nsec = SLEEP_NS; // value obtained by trial and error to match transmitter
}

void Nexa__sendCode(
  int32_t groupCode, bool groupCommand, bool activate, char deviceCode)
{
  int32_t nexaCodeOrig =
    Nexa__nexaCode(groupCode, groupCommand, activate, deviceCode);

  int32_t mask = 0x80000000;

  // Resend the code 10 times
  for (int i = 0 ; i < 10 ; i++)
  {
    int32_t nexaCode = nexaCodeOrig;
    Nexa__sendStartBits();
    for (int bit = 0; bit < 32; bit++)
    {
      int32_t currentBit = nexaCode & mask;
      if (currentBit) // Current bit is set
      {
        Nexa__sendOne();
      }
      else
      {
        Nexa__sendZero();
      }
      nexaCode <<= 1;
    }
    Nexa__sendStopBits();

    nanosleep(&resendSleepTime, &remTime);
  }
}

void Nexa__sendStartBits()
{
  Nexa__sendLow(4);
  Nexa__sendHigh(1);
  Nexa__sendLow(7);
}

void Nexa__sendStopBits()
{
  Nexa__sendHigh(1);
  Nexa__sendLow(4);
}

void Nexa__sendOne()
{
  Nexa__sendHigh(1);
  Nexa__sendLow(4);
  Nexa__sendHigh(1);
  Nexa__sendLow(1);
}

void Nexa__sendZero()
{
  Nexa__sendHigh(1);
  Nexa__sendLow(1);
  Nexa__sendHigh(1);
  Nexa__sendLow(4);
}

void Nexa__sendHigh(char count)
{
  for (char i = 0; i < count; i++)
  {
    GPIO_SET = 1 << DATA_PIN;
    nanosleep(&bitSleepTime, &remTime);
  }
}

void Nexa__sendLow(char count)
{
  for (char i = 0; i < count; i++)
  {
    GPIO_CLR = 1 << DATA_PIN;
    nanosleep(&bitSleepTime, &remTime);
  }
}

int32_t Nexa__nexaCode(int32_t groupCode, bool groupCommand, bool activate, char deviceCode)
{
  int32_t nexaCode = 0;

  nexaCode |= (groupCode & GROUP_CODE_MASK) << GROUP_CODE_OFFSET;
  nexaCode |= (boolToBit(groupCommand) & GROUP_COMMAND_MASK) << GROUP_COMMAND_OFFSET;
  nexaCode |= (boolToBit(activate) & ACTIVATE_MASK) << ACTIVATE_OFFSET;
  nexaCode |= (deviceCode & DEVICE_CODE_MASK) << DEVICE_CODE_OFFSET;

  return nexaCode;
}

char boolToBit(bool boolVar)
{
  if (boolVar)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
