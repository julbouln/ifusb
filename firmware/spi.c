#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>

#include "board.h"

void spi_setup(void)
{
  /* Setup GPIO pins for AF5 for SPI1 signals. */
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
                  GPIO5 | GPIO6 | GPIO7);
  gpio_set_af(GPIOA, GPIO_AF0, GPIO5 | GPIO6 | GPIO7);

  spi_set_master_mode(SPI1);
  spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_64); // 750khz
  spi_set_clock_polarity_0(SPI1);
  spi_set_clock_phase_0(SPI1);
  spi_set_full_duplex_mode(SPI1);
  spi_set_unidirectional_mode(SPI1); /* bidirectional but in 3-wire */
  spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
  spi_enable_software_slave_management(SPI1);
  spi_send_msb_first(SPI1);
  spi_set_nss_high(SPI1);
  spi_fifo_reception_threshold_8bit(SPI1);
  spi_i2s_mode_spi_mode(SPI1);
  spi_enable(SPI1);

//  nvic_set_priority(NVIC_DMA1_CHANNEL2_3_IRQ, 0);
//  nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_IRQ);
}

void spi_dma_xfer(char *ibuf, char *obuf, uint16_t len) {
  dma_channel_reset(DMA1, SPI_DMA_RX_CH);
  dma_channel_reset(DMA1, SPI_DMA_TX_CH);

  /* Reset SPI data and status registers.
   * Here we assume that the SPI peripheral is NOT
   * busy any longer, i.e. the last activity was verified
   * complete elsewhere in the program.
   */
  volatile uint8_t temp_data __attribute__ ((unused));
  while (SPI_SR(SPI1) & (SPI_SR_RXNE | SPI_SR_OVR)) {
    temp_data = SPI_DR8(SPI1);
  }

  dma_set_peripheral_address(DMA1, SPI_DMA_RX_CH, (uint32_t)&SPI1_DR);
  dma_set_memory_address(DMA1, SPI_DMA_RX_CH, (uint32_t)obuf);
  dma_set_number_of_data(DMA1, SPI_DMA_RX_CH, len);
  dma_set_read_from_peripheral(DMA1, SPI_DMA_RX_CH);
  dma_enable_memory_increment_mode(DMA1, SPI_DMA_RX_CH);
  dma_set_peripheral_size(DMA1, SPI_DMA_RX_CH, DMA_CCR_PSIZE_8BIT);
  dma_set_memory_size(DMA1, SPI_DMA_RX_CH, DMA_CCR_MSIZE_8BIT);
  dma_set_priority(DMA1, SPI_DMA_RX_CH, DMA_CCR_PL_VERY_HIGH);


  dma_set_peripheral_address(DMA1, SPI_DMA_TX_CH, (uint32_t)&SPI1_DR);
  dma_set_memory_address(DMA1, SPI_DMA_TX_CH, (uint32_t)ibuf);
  dma_set_number_of_data(DMA1, SPI_DMA_TX_CH, len);
  dma_set_read_from_memory(DMA1, SPI_DMA_TX_CH);
  dma_enable_memory_increment_mode(DMA1, SPI_DMA_TX_CH);
  dma_set_peripheral_size(DMA1, SPI_DMA_TX_CH, DMA_CCR_PSIZE_8BIT);
  dma_set_memory_size(DMA1, SPI_DMA_TX_CH, DMA_CCR_MSIZE_8BIT);
  dma_set_priority(DMA1, SPI_DMA_TX_CH, DMA_CCR_PL_HIGH);

  dma_enable_transfer_complete_interrupt(DMA1, SPI_DMA_RX_CH);
  dma_enable_transfer_complete_interrupt(DMA1, SPI_DMA_TX_CH);

  dma_enable_channel(DMA1, SPI_DMA_RX_CH);
  dma_enable_channel(DMA1, SPI_DMA_TX_CH);

  spi_enable_rx_dma(SPI1);
  spi_enable_tx_dma(SPI1);

  while (!dma_get_interrupt_flag(DMA1, SPI_DMA_RX_CH, DMA_TCIF)); // blocking

  spi_disable_rx_dma(SPI1);
  spi_disable_tx_dma(SPI1);
  dma_clear_interrupt_flags(DMA1, SPI_DMA_RX_CH, DMA_TCIF);
  dma_clear_interrupt_flags(DMA1, SPI_DMA_TX_CH, DMA_TCIF);
  dma_disable_channel(DMA1, SPI_DMA_RX_CH);
  dma_disable_channel(DMA1, SPI_DMA_TX_CH);

}

// DMA_CH2 and DMA_CH3 int
void dma1_channel2_3_isr(void) {
  spi_disable_tx_dma(SPI1);
  spi_disable_rx_dma(SPI1);
  dma_clear_interrupt_flags(DMA1, SPI_DMA_TX_CH, DMA_TCIF);
  dma_clear_interrupt_flags(DMA1, SPI_DMA_RX_CH, DMA_TCIF);
  dma_disable_channel(DMA1, SPI_DMA_TX_CH);
  dma_disable_channel(DMA1, SPI_DMA_RX_CH);
}