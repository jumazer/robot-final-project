# Final Project: Commanding Robot through USART


actual tested HC-05 UART baud = 9600
do not trust the uploaded datasheet for this module variant

You run bind when you need to create the /dev/rfcomm1 mapping
after that, opening /dev/rfcomm1 should trigger the connection
you only need to run bind again if the binding is gone, released, or lost after reboot/service restart

sudo rfcomm bind 1 00:14:03:05:09:9E 1

Fixing character device
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