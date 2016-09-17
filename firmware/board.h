#ifndef BOARD_H
#define BOARD_H

#define IFUSB_BOARD 1

#define LED_PORT GPIOB
#define LED_PIN GPIO3

#define USB_PORT GPIOA
#define USB_DM_PIN GPIO11
#define USB_DP_PIN GPIO12

// USART
#define USARTN USART2
#define USART_GPIO_AF GPIO_AF1
#define RCC_USARTN RCC_USART2
#define NVIC_USARTN_IRQ NVIC_USART2_IRQ
#define USARTN_TDR USART2_TDR
#define USART_PORT GPIOA
#define USART_TX_PIN GPIO2
#define USART_RX_PIN GPIO3

// DMA
#define USART_DMA_TX_CH DMA_CHANNEL4 // fixed mapping
#define USART_DMA_RX_CH DMA_CHANNEL5 // fixed mapping

#define SPI_DMA_RX_CH DMA_CHANNEL2 // fixed mapping
#define SPI_DMA_TX_CH DMA_CHANNEL3 // fixed mapping

static uint32_t gpio_map[6][2] = {
	{GPIOA,GPIO0},
	{GPIOA,GPIO1},
	{GPIOA,GPIO4},
#ifdef IFUSB_BOARD
	{GPIOB,GPIO1},
	{GPIOA,GPIO13},
	{GPIOA,GPIO14}
#else
	{GPIOB,GPIO0},
	{GPIOB,GPIO1},
	{GPIOA,GPIO8}
#endif
};

// other config
#define SPI_USE_DMA 1
#define USART_USE_DMA 1
#define USB_USE_INT 1

#endif