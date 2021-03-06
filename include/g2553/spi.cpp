// Copyright (C) [2012, 2013] [AB2 Technologies] [Austin Beam, Alan Bullick]
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

// MSP430G2553 SPI Library
// Simple SPI library for providing maximum re-use and ease of integration.

#include "spi.h"

#define SPI_RXIE(usci)  (1<<(usci))
#define SPI_TXIE(usci)  (1<<(usci+1))
#define SPI_RXIFG(usci) (1<<(usci))
#define SPI_TXIFG(usci) (1<<(usci+1))

//                                     A0     B0
bool spi::is_init[NUM_SPI_USCIs] = {false, false};


const msp_pin_t spi::spi_pins[NUM_SPI_USCIs][NUM_SPI_PINS] = {
  {
    // USCI_A0
    // CLK , SOMI, SIMO
       p1_4, p1_1, p1_2
  },
  {
    // USCI_B0
    // CLK , SOMI, SIMO
       p1_5, p1_6, p1_7
  }
};

// Configure SPI for LSB-first transfers
void spi::cfgLSB(void)
{
  enterReset();
  // Turn the MSB bit off
  off(UC_CTL1(spi_base_addr), UCMSB);
  exitReset();
}

// Configure SPI for MSB-first transfers
void spi::cfgMSB(void)
{
  enterReset();
  // Turn the MSB bit on
  on (UC_CTL1(spi_base_addr), UCMSB);
  exitReset();
}

// Disable the SOMI pin if it's needed for something else
void spi::disableSOMI(void)
{
  pinSelOff(spi_pins[spi_usci][SPI_USCI_SOMI]);
  pinSel2Off(spi_pins[spi_usci][SPI_USCI_SOMI]);
}

// Configure SPI for falling edge
void spi::fallingEdge(void)
{
  enterReset();
  off(UC_CTL0(spi_base_addr), UCCKPH);
  exitReset();
}

// Get the current SPI prescaler
uint16_t spi::getPrescaler(void)
{
  return (UC_BRW(spi_base_addr));
}

// Initialize SPI
void spi::init(void)
{
  // Enable spi using the SEL register for each pin
  pinSelOn(spi_pins[spi_usci][SPI_USCI_CLK]);
  pinSel2On(spi_pins[spi_usci][SPI_USCI_CLK]);
  pinSelOn(spi_pins[spi_usci][SPI_USCI_SIMO]);
  pinSel2On(spi_pins[spi_usci][SPI_USCI_SIMO]);

  // Put state machine in reset
  enterReset();

  // 3-pin, 8-bit SPI master
  // Clock polarity low, rising edge, MSB
  on(UC_CTL0(spi_base_addr), (UCMST | UCSYNC | UCCKPH | UCMSB));

  // SMCLK
  on(UC_CTL1(spi_base_addr), UCSSEL_2);

  // Set the prescaler to min initially
  setMinPrescaler();

  // Initialize USCI state machine
  // Not necessary -- done in setMinPrescaler
  // We'll do it again anyway
  exitReset();
}

// Pulse the SPI CLK pin
void spi::pulseClk(uint8_t times)
{
  uint8_t i;

  // Need to first disable the SPI functionality of the pin
  pinSelOff(spi_pins[spi_usci][SPI_USCI_CLK]);
  pinSel2Off(spi_pins[spi_usci][SPI_USCI_CLK]);
  // Now make sure it's an output
  pinOutput(spi_pins[spi_usci][SPI_USCI_CLK]);
  for (i=0; i<times; i++)
  {
    // Now pulse the pin
    pinPulse(spi_pins[spi_usci][SPI_USCI_CLK]);
  }
  // Now turn the SPI functionality back on
  pinSelOn(spi_pins[spi_usci][SPI_USCI_CLK]);
  pinSel2On(spi_pins[spi_usci][SPI_USCI_CLK]);
}

// Configure SPI for rising edge
void spi::risingEdge(void)
{
  enterReset();
  on (UC_CTL0(spi_base_addr), UCCKPH);
  exitReset();
}

// Read a series of bytes from a SPI slave
void spi::rxFrame(uint8_t *buf, uint16_t size)
{
  uint16_t i = 0;
  for (i=0; i<size; i++)
  {
    // Write the dummy char to the TXBUF
    set(UC_TXBUF(spi_base_addr), dummy_char);
    // Wait for the transaction to complete
    while (read(UC_IFG(spi_base_addr), SPI_RXIFG(spi_usci)) == 0);
    // Store the received value in the buffer
    set(buf[i], UC_RXBUF(spi_base_addr));
  }
}

// Set the SPI clock prescaler value
void spi::setPrescaler(uint16_t prescaler)
{
  enterReset();
  set(UC_BRW(spi_base_addr), prescaler);
  exitReset();
}

// Write a byte to SPI -- read return byte as well
uint8_t spi::tx(uint8_t byte)
{
  // Write a byte to TXBUF
  set(UC_TXBUF(spi_base_addr), byte);
  // Wait for the transaction to complete
  //while (read(UC_IFG(spi_base_addr), SPI_RXIFG(spi_usci)) == 0);
  while (read(UC_STAT(spi_base_addr), UCBUSY));

  // Return any data that might have been returned by the slave
  return (UC_RXBUF(spi_base_addr));
}

// Write a series of bytes to SPI
void spi::txFrame(uint8_t *buf, uint16_t size)
{
  int16_t i = 0;
  volatile uint8_t tmpVar;

  // Send the buffer one byte at a time
  for (i=size-1; i>=0; --i)
  {
    // Write the TX buffer with the i-th byte
    set(UC_TXBUF(spi_base_addr), buf[i]);

    // Wait for the TX buffer to be ready
    while (read(UC_IFG(spi_base_addr), SPI_TXIFG(spi_usci)) == 0);
  }

  // Wait for the transaction to complete
  while (read(UC_STAT(spi_base_addr), UCBUSY));
  // Dummy read to clear the RX IFG flag
  set(tmpVar, UC_RXBUF(spi_base_addr));
}

