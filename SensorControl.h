/**
 * @file    SENSOR_cntrl.h
 * @brief   Control of all Sensor Inputs and associated Hardware Peripherals onboard SunLeaf hardware Module
 * @author  Adam Vadala-Roth (Adamjvr)
 * @version 1.0
 * @see
 *
 * Copyright (c) 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Pin map note
/* I2C GPIO Pins
*  EN1 - enable 1 - STM32 Port: PB4
*  EN2 - enable 2 - STM32 Port: PB8
*  EN3 - enable 3 - STM32 Port: PB9
*  EN4 - enable 4 - STM32 Port: PB5
*/

/* UART Multiplexer/Demultiplexer GPIO Pins
*  EN - enable - STM32 Port: PA4
*  S0 - select 0 - STM32 Port: PA5
*  S1 - select 1 - STM32 Port: PA6
*/

#ifndef _SENSOR_CONTROL_H
#define _SENSOR_CONTROL_H

#include "mbed.h"

// UART Sensor Bank Data stuct
typedef struct {
    uint16_t UART_chan1;
    uint16_t UART_chan2;
    uint16_t UART_chan3;
    uint16_t UART_chan4;
} UART_DAT;

// I2C Sensor Bank Data struct
typedef struct {
    uint16_t I2C_chan1;
    uint16_t I2C_chan2;
    uint16_t I2C_chan3;
    uint16_t I2C_chan4;
} I2C_DAT;

// Analog Sensor Bank Data struct
typedef struct {
    uint16_t ana_chan1;
    uint16_t ana_chan2;
    uint16_t ana_chan3;
    uint16_t ana_chan4;
} ANA_DAT;

class SensorControl
{
public:
    SensorControl();

// Configures all the GPIO pins for muxing, UIART peripheral, and I2C
    void Sensor_IO_Setup();
// Sets up ADC
    void ADC_Setup();
// Reinstantiates the ADC after a serial read
    void ADC_Setup_Reset();
// Returns UART sensor value from the specifed channel
    uint16_t Get_UART_S(int chan_num);
// Returns I2C sensor value from the specifed channel
    uint16_t Get_I2C_S(int chan_num, uint8_t s_cmd[2],uint8_t s_addr);
// Returns Analog sensor value from the specifed channel
    uint16_t Get_ANA_S(int chan_num);
};
#endif
