# Final Project: Commanding Robot through USART



actual tested HC-05 UART baud = 9600
do not trust the uploaded datasheet for this module variant

You run bind when you need to create the /dev/rfcomm1 mapping
after that, opening /dev/rfcomm1 should trigger the connection
you only need to run bind again if the binding is gone, released, or lost after reboot/service restart

sudo rfcomm bind 1 00:14:03:05:09:9E 1

// Fixing character device
sudo rfcomm release 1 2>/dev/null
sudo rm -f /dev/rfcomm1
sudo rfcomm bind 1 00:14:03:05:09:9E 1
ls -l /dev/rfcomm1


// Code for viewing clk frequency
#include "stm32f0xx.h"
extern uint32_t SystemCoreClock;
extern void SystemCoreClockUpdate(void);
void get_clock_info() {
    SystemCoreClockUpdate();

    volatile uint32_t hclk = SystemCoreClock;   // core clock

    volatile uint32_t sysclk = hclk;
    volatile uint32_t pclk   = hclk;

    uint32_t hpre = (RCC->CFGR >> 4) & 0xF;     // AHB prescaler
    uint32_t ppre = (RCC->CFGR >> 8) & 0x7;     // APB prescaler

    // Recover SYSCLK from HCLK
    if (hpre >= 8) {                            // 1000:/2 ... 1111:/512
        uint32_t ahb_shift = hpre - 7;
        sysclk = hclk << ahb_shift;
    }


    // Get PCLK from HCLK
    if (ppre >= 4) {                            // 100:/2, 101:/4, 110:/8, 111:/16
        pclk = hclk >> (ppre - 3);
    }

    DBG_PRINTF("pclk: %lu\r\n", pclk);
    DBG_PRINTF("hclk: %lu\r\n", hclk);
    DBG_PRINTF("sysclk: %lu\r\n", sysclk);
}


// Joystick Forward-Backward mapping
F99R00 is the value produced when moving the joystick all the way forward 
B99R00 is the value produced when moving the joystick all the way backward
F00R00 is the value when produced when I am not touching the joystick at all
Difference between F99R00 - F00R00 = 99, so 99 is the magnitude

// Joystick Left-Right mapping
FOOR60 is the value produced when moving the right joystick all the way right
FOOL60 is the value produced when moving the right joystick all the way right
F00R00 is the value when produced when I am not touching the joystick at all

// pin layout

// motor 1
PA9 -> STBY
PC7 -> AIN1
PB6 -> AIN2
PB5 -> BIN1
PB3 -> BIN2
3.3V -> VCC
GND -> GND
battery + -> VM
battery - -> GND

// motor 2
PB0 -> BIN2
PC1 -> BIN1
PC0 -> STBY
PC3 -> AIN2
PC2 -> AIN1

PA6 -> PWMB
PA7 -> PWMA

Bluetooth Module:
- HC-05 VCC -> STM32 3.3V or board VCC input depending on breakout
- HC-05 GND -> STM32 GND
- HC-05 TX -> STM32 UART RX
- HC-05 RX -> STM32 UART TX

// Motor Definition
B channel as the left motor
A channel as the right motor

https://www.microcenter.com/product/456573/adafruit-industries-snap-action-3-wire-block-connector-12-28-awg-3-pack?utm_source=chatgpt.com
https://www.microcenter.com/product/618895/inland-dc-dc-voltage-step-down-module