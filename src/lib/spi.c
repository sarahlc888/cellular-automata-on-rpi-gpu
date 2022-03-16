/*
 * This file was taken directly from $CS107E/extras/hw_rng
 *
 * Additional comments and page references added by Avi Udash on 03/13/2022
 *
 * Library for SPI protocol to interact with hardware devices.
 *
 * Much of the data for the implementation can be found in the main Broadcom2835
 * datasheet, specifically pages 148-159
 */
#include "spi.h"
#include "gpio.h"

/*
 * Setup the SPI register map at address 0x20204000 on the pi.
 *
 * page 152 of the manual.
 */
struct spi {
  unsigned cs;   /* SPI controller control and status */
  unsigned fifo; // TX and RX FIFO
  unsigned clk;  // clock divider
  unsigned dlen; // data length
  unsigned ltoh;
  unsigned dc; // DMA DREQ controls
};
volatile struct spi *spi = (struct spi *)0x20204000;

/* Defines common control and status bits, used in the CS register
 *
 * page 153 of the manual.
 */
#define SPI0_CLEAR_TX (1 << 4)
#define SPI0_CLEAR_RX (1 << 5)
#define SPI0_TRANSFER_ACTIVE (1 << 7)
#define SPI0_TRANSFER_DONE (1 << 16)
#define SPI0_RX_HAS_DATA (1 << 17)
#define SPI0_TX_CAN_ACCEPT (1 << 18)

/*
 * Function: spi_init
 * --------------------------
 *  @param unsigned chip_select: either SPI_CE0 or SPI_CE1
 *  @param unsigned clock_divider: sets clock_divider rate for CS, 0 for
 *         65536(pg 156)
 * --------------------------
 * The function initializes the SPI library, setting correct functions for
 * gpio_pins and CS and CLK settings
 */
void spi_init(unsigned chip_select, unsigned clock_divider) {
  gpio_set_function(GPIO_PIN7, GPIO_FUNC_ALT0);  // SPI0_CE1_N
  gpio_set_function(GPIO_PIN8, GPIO_FUNC_ALT0);  // SPI0_CE0_N
  gpio_set_function(GPIO_PIN9, GPIO_FUNC_ALT0);  // SPI0_MISO
  gpio_set_function(GPIO_PIN10, GPIO_FUNC_ALT0); // SPI0_MOSI
  gpio_set_function(GPIO_PIN11, GPIO_FUNC_ALT0); // SPI0_SCLK

  spi->cs = 0;
  spi->cs |= SPI0_CLEAR_TX | SPI0_CLEAR_RX;

  if (chip_select == SPI_CE0) {
    spi->cs |= 0b00;
  } else if (chip_select == SPI_CE1) {
    spi->cs |= 0b01;
  }

  spi->clk = clock_divider;
}

/*
 * Function: spi_transfer
 * --------------------------
 *  @param unsigned char* tx: an array of 8-bit chars to SEND to the peripheral
 *         device
 *  @param unsigned char* rx: an array of 8-bit chars to READ from the
 *         peripheral
 *  @param unsigned len: number of 8-bit chars to send to / read from the
 *         peripheral. each array above should have the same size
 * --------------------------
 *  The function transfers to and read from the peripheral, 8-bits at a time. It
 *  sends and reads data at the same time.
 *
 *  It continuously sends 8-bits from 'tx[]' to the peripheral, and then reads
 *  8-bits into 'rx[]' from the peripheral, until it sends 'len' number of chars
 *
 */
void spi_transfer(unsigned char *tx, unsigned char *rx, unsigned len) {
  spi->cs |= SPI0_CLEAR_TX | SPI0_CLEAR_RX | SPI0_TRANSFER_ACTIVE;

  for (int i = 0; i < len; i++) {
    // wait until TX can accept data
    while (!(spi->cs & SPI0_TX_CAN_ACCEPT))
      ;
    spi->fifo = tx[i];

    // wait until transfer is finished
    while (!(spi->cs & SPI0_TRANSFER_DONE))
      ;
    // then wait until RX has data
    while (!(spi->cs & SPI0_RX_HAS_DATA))
      ;

    // read in the data
    rx[i] = (unsigned char)spi->fifo;
  }

  spi->cs &= ~SPI0_TRANSFER_ACTIVE;
}
