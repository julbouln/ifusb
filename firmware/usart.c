#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>

#include "board.h"
#include "ifusb.h"


void usart_setup(void)
{

	/* Setup GPIO pins for USART TX/RX pin */
	gpio_mode_setup(USART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART_TX_PIN | USART_RX_PIN);

	/* Setup USART TX/RX pin as alternate function. */
	gpio_set_af(USART_PORT, USART_GPIO_AF, USART_TX_PIN | USART_RX_PIN);

	/* Setup USART parameters. */
	usart_set_baudrate(USARTN, 115200);
//	usart_set_baudrate(USARTN, 921600);
//	usart_set_baudrate(USARTN, 2000000);

	usart_set_databits(USARTN, 8);
	usart_set_parity(USARTN, USART_PARITY_NONE);
	usart_set_stopbits(USARTN, USART_CR2_STOP_1_0BIT);
	usart_set_mode(USARTN, USART_MODE_TX_RX);
	usart_set_flow_control(USARTN, USART_FLOWCONTROL_NONE);

	usart_enable_rx_interrupt(USARTN);
	usart_disable_tx_interrupt(USARTN);

	/* Finally enable the USART. */
	usart_enable(USARTN);

	dma_enable_channel(DMA1, USART_DMA_TX_CH);
	dma_enable_channel(DMA1, USART_DMA_RX_CH);
//nvic_enable_irq(NVIC_DMA1_CHANNEL4_5_IRQ);

	/* Enable the USART interrupt. */
	nvic_enable_irq(NVIC_USARTN_IRQ);

}

void usart_dma_write(char *data, int size)
{
	dma_channel_reset(DMA1, USART_DMA_TX_CH);

	dma_set_peripheral_address(DMA1, USART_DMA_TX_CH, (uint32_t)&USARTN_TDR);
	dma_set_memory_address(DMA1, USART_DMA_TX_CH, (uint32_t)data);
	dma_set_number_of_data(DMA1, USART_DMA_TX_CH, size);
	dma_set_read_from_memory(DMA1, USART_DMA_TX_CH);
	dma_enable_memory_increment_mode(DMA1, USART_DMA_TX_CH);
	dma_set_peripheral_size(DMA1, USART_DMA_TX_CH, DMA_CCR_PSIZE_8BIT);
	dma_set_memory_size(DMA1, USART_DMA_TX_CH, DMA_CCR_MSIZE_8BIT);
	dma_set_priority(DMA1, USART_DMA_TX_CH, DMA_CCR_PL_VERY_HIGH);

	dma_enable_transfer_complete_interrupt(DMA1, USART_DMA_TX_CH);

	dma_enable_channel(DMA1, USART_DMA_TX_CH);

	usart_enable_tx_dma(USARTN);

	while (!dma_get_interrupt_flag(DMA1, USART_DMA_TX_CH, DMA_TCIF)); // blocking
}


void usart_dma_read(char *data, int size)
{
	dma_channel_reset(DMA1, USART_DMA_RX_CH);

	dma_set_peripheral_address(DMA1, USART_DMA_RX_CH, (uint32_t)&USARTN_TDR);
	dma_set_memory_address(DMA1, USART_DMA_RX_CH, (uint32_t)data);
	dma_set_number_of_data(DMA1, USART_DMA_RX_CH, size);
	dma_set_read_from_peripheral(DMA1, USART_DMA_RX_CH);
	dma_enable_memory_increment_mode(DMA1, USART_DMA_RX_CH);
	dma_set_peripheral_size(DMA1, USART_DMA_RX_CH, DMA_CCR_PSIZE_8BIT);
	dma_set_memory_size(DMA1, USART_DMA_RX_CH, DMA_CCR_MSIZE_8BIT);
	dma_set_priority(DMA1, USART_DMA_RX_CH, DMA_CCR_PL_HIGH);

	dma_enable_transfer_complete_interrupt(DMA1, USART_DMA_RX_CH);

	dma_enable_channel(DMA1, USART_DMA_RX_CH);

	usart_enable_rx_dma(USART2);

	while (!dma_get_interrupt_flag(DMA1, USART_DMA_RX_CH, DMA_TCIF)); // blocking
}

void usart_write(uint8_t *data, uint8_t len)
{
	uint8_t b;
	for (b = 0; b < len; b++)
		usart_send_blocking(USARTN, data[b]);
}

uint8_t usart_read() {
	uint8_t recv = usart_recv_blocking(USARTN);
	return recv;
}

// DMA_CH4 and DMA_CH5 int
void dma1_channel4_5_isr(void) {
	usart_disable_tx_dma(USARTN);
	usart_disable_rx_dma(USARTN);
	dma_clear_interrupt_flags(DMA1, USART_DMA_TX_CH, DMA_TCIF);
	dma_clear_interrupt_flags(DMA1, USART_DMA_RX_CH, DMA_TCIF);

//	dma_disable_transfer_complete_interrupt(DMA1, USART_DMA_RX_CH);

	dma_disable_channel(DMA1, USART_DMA_TX_CH);
	dma_disable_channel(DMA1, USART_DMA_RX_CH);
}

void usart2_isr(void)
{
/*	
	uint8_t data[64];

	data[0] = IFUSB_UART_RECV;
	usart_dma_read(data+1, 63);
	ifusb_packet_write(data, 64);
*/
	
	uint8_t data;
	uint8_t buf[2];
	data = usart_recv(USARTN);
	buf[0] = IFUSB_UART_RECV;
	buf[1] = data;
	ifusb_packet_write(buf, 2);

//	nvic_clear_pending_irq(NVIC_USARTN_IRQ);
}
