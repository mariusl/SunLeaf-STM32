/**
 * @file    SensorControl.h
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

#include "SensorControl.h"
#include "mbed.h"

//  Use UART on pins PA0 and PA1 for Sensor UART, UART4
static Serial UART4_S(PA_0,PA_1);
// Use I2C on pins PB8 and PB9 for Sensor I2C, I2C1
I2C i2c1_S(PB_8,PB_9);

// analog sensor inputs on the SunLeaf
AnalogIn ana_s1(PA_7); // STM32 Port: PA7
AnalogIn ana_s2(PC_4); // STM32 Port: PC4
AnalogIn ana_s3(PC_5); // STM32 Port: PC5
AnalogIn ana_s4(PB_0); // STM32 Port: PB0

// Digital Ports for Multiplexer control
DigitalOut I2C_EN1(PB_4);
DigitalOut I2C_EN2(PB_8);
DigitalOut I2C_EN3(PB_9);
DigitalOut I2C_EN4(PB_5);

DigitalOut UART_EN(PA_4);
DigitalOut UART_S0(PA_5);
DigitalOut UART_S1(PA_6);

// Constants for I2C sensor addresses
const uint8_t addr_0 = 0x00;
const uint8_t addr_1 = 0x01;
const uint8_t addr_2 = 0x02;
const uint8_t addr_3 = 0x03;


SensorControl(){
  
}


// Configures all the GPIO pins for muxing, UIART peripheral, and I2C
void SensorControl::Sensor_IO_Setup()
{
    // Setup UART Peripheral
    UART4_S.baud(115200);
    UART4_S.format(8,SerialBase::None, 1);
    //UART4_S.set_flow_control(RTSCTS,CTS,RTS);

    // Setup I2C Peripheral
    i2c1_S.frequency(100);
}

// Sets up the ADC
void SensorControl::ADC_Setup()
{

}

// Used to Reinstantiate the ADC after a Serial Read, port checking
void SensorControl::ADC_Setup_Reset()
{

}

// Returns UART sensor value from the specifed channel
uint16_t SensorControl::Get_UART_S(int chan_num)
{
    uint16_t S_DAT=0;
    switch (chan_num) {
        case 0:
            UART_EN.write(1);
            UART_S0.write(0);
            UART_S1.write(0);
            break;
        case 1:
            UART_EN.write(1);
            UART_S0.write(0);
            UART_S1.write(1);
            break;
        case 2:
            UART_EN.write(1);
            UART_S0.write(1);
            UART_S1.write(0);
            break;
        case 3:
            UART_EN.write(1);
            UART_S0.write(1);
            UART_S1.write(1);
            break;
    }

    while(UART4_S.readable()) {
        S_DAT = UART4_S.getc();
    }
    return S_DAT;
}

// Returns I2C sensor value from the specifed channel
uint16_t SensorControl::Get_I2C_S(int chan_num, uint8_t s_cmd[2],uint8_t s_addr)
{
    uint16_t S_DAT=0;
    uint8_t cmd[2];
    uint8_t addr;
    cmd[2] = s_cmd[2];
    addr = s_addr;
    switch (chan_num) {
        case 0:
            I2C_EN1.write(1);
            I2C_EN2.write(1);
            I2C_EN3.write(1);
            I2C_EN4.write(1);
                    break;
        case 1:
            I2C_EN1.write(1);
            I2C_EN2.write(1);
            I2C_EN3.write(1);
            I2C_EN4.write(1);
                    break;
        case 2:
            I2C_EN1.write(1);
            I2C_EN2.write(1);
            I2C_EN3.write(1);
            I2C_EN4.write(1);
                    break;
        case 3:
            I2C_EN1.write(1);
            I2C_EN2.write(1);
            I2C_EN3.write(1);
            I2C_EN4.write(1);
                    break;
    }
    i2c1_S.write(addr);
    wait(0.5);
    cmd[0] = 0x00;
   // i2c1_S.write(a);
   // S_DAT = i2c1_S.read(cmd);

    return S_DAT;
}

// Returns Analog sensor value from the specifed channel
uint16_t SensorControl::Get_ANA_S(int chan_num)
{
    uint16_t S_DAT=0;
    switch (chan_num) {
        case 0:
            S_DAT = ana_s1.read_u16();
            break;
        case 1:
            S_DAT = ana_s2.read_u16();
            break;
        case 2:
            S_DAT = ana_s3.read_u16();
            break;
        case 3:
            S_DAT = ana_s4.read_u16();
            break;
    }
    return S_DAT;
}
