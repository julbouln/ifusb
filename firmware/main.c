/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Copyright (C) 2012 Karl Palsson <karlp@tweak.net.au>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>

#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/syscfg.h>

#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/pwr.h>

#include "usb.h"
#include "board.h"
#include "ifusb.h"

volatile uint32_t system_millis;

/* Called when systick fires */
void sys_tick_handler(void)
{
	system_millis++;
}

/* sleep for delay milliseconds */
static void delay(uint32_t delay)
{
	uint32_t wake = system_millis + delay;
	while (wake > system_millis);
}

/* Set up a timer to create 1mS ticks. */
static void systick_setup(void)
{
	/* clock rate / 1000 to get 1mS interrupt rate */
	systick_set_reload(48000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	/* this done last */
	systick_interrupt_enable();
}

static void gpio_setup(void)
{

//	gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

#define __WFI() __asm__("wfi")
#define __WFE() __asm__("wfe")

void sleep_mode(int deepSleepFlag)
{
	// Clear PDDS and LPDS bits
	PWR_CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

	// Set PDDS and LPDS bits for standby mode, and set Clear WUF flag (required per datasheet):
	PWR_CR |= PWR_CR_CWUF;
	PWR_CR |= PWR_CR_PDDS;
	// Enable wakeup pin bit.
	PWR_CR |=  PWR_CSR_EWUP;

	// Low-power deepsleep bit.
	// PWR_CR |= PWR_CR_LPDS;

	// System Control Register Bits. See...
	// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0497a/Cihhjgdh.html
	if (deepSleepFlag)
	{	// Experimental
		// Set Power down deepsleep bit.
		PWR_CR |= PWR_CR_PDDS;
		// Unset Low-power deepsleep.
		PWR_CR &= ~PWR_CR_LPDS;
		// Set sleepdeep in the system control register - if set, we deepsleep and coldstart from RTC or pin interrupts.
//    SCB_SCR |= SCB_SCR_SLEEPDEEP;
	} else {
		//  Unset Power down deepsleep bit.
		PWR_CR &= ~PWR_CR_PDDS;
		// set Low-power deepsleep.
		PWR_CR |= PWR_CR_LPDS;
		/*
		 * PWR_CR_PDDS
		    Power down deepsleep.
		   PWR_CR_LPDS
		    Low-power deepsleep.
		 */
		// Unset sleepdeep in the system control register - if not set then we only sleep and can wake from RTC or pin interrupts.
//    SCB_SCR |= SCB_SCR_SLEEPDEEP;
		// Low-power deepsleep bit.
	}

	// Set end Event on Pending bit: - enabled events and all interrupts, including disabled interrupts, can wakeup the processor.
	SCB_SCR |= SCB_SCR_SEVEONPEND;


	// Set SLEEPONEXIT -Indicates sleep-on-exit when returning from Handler mode to Thread mode -
	// if enabled, we will effectively sleep from the end of  one interrupt till the start of the next.
	SCB_SCR |= SCB_SCR_SLEEPONEXIT;


//	SCB_SCR &= ~SCB_SCR_SLEEPDEEP;

	// Now go into stop mode, wake up on interrupt
	// asm("    wfi");
//	__WFI();

}

void pow_init() {
	SCB_SCR &= ~SCB_SCR_SLEEPDEEP;
	SCB_SCR |= SCB_SCR_SLEEPONEXIT;
}

void rcc_init() {
#ifdef IFUSB_BOARD
	// crystal-less ?
	rcc_clock_setup_in_hsi48_out_48mhz();
	rcc_periph_clock_enable(RCC_SYSCFG_COMP);
	SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
	crs_autotrim_usb_enable();
	rcc_set_usbclk_source(RCC_HSI48);
#else
	rcc_clock_setup_in_hsi_out_48mhz();
#endif

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOF);

	rcc_periph_clock_enable(RCC_DMA);

	rcc_periph_clock_enable(RCC_USARTN);
	rcc_periph_clock_enable(RCC_SPI1);
	rcc_periph_clock_enable(RCC_I2C1);

}

void rcc_disable() {
	rcc_periph_clock_disable(RCC_GPIOA);
	rcc_periph_clock_disable(RCC_GPIOB);
	rcc_periph_clock_disable(RCC_GPIOF);

	rcc_periph_clock_disable(RCC_DMA);

	rcc_periph_clock_disable(RCC_USARTN);
	rcc_periph_clock_disable(RCC_SPI1);
	rcc_periph_clock_disable(RCC_I2C1);
}

void sys_init() {

	rcc_init();
//	gpio_setup();
//	systick_setup();


	usart_setup();
	spi_setup();
	i2c_setup();

//	gpio_set(LED_PORT, LED_PIN);

//	sleep_mode(0);

	usb_setup();
	ifusb_init();
}

int main(void)
{
	sys_init();
//	delay(100);

#ifdef USB_USE_INT
	pow_init();
#endif

	while (1) {
#ifdef USB_USE_INT
		__WFI();

#else
		usb_poll();
#endif
	}

	return 0;
}

