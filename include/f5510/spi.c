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

// MSP430F5510 SPI Library
// Simple SPI library for providing maximum re-use and ease of integration

// Limitations: Only master out SPI is supported. Read should be trivial, with
// the only qualification being the configuration of an RX ISR

#include "spi.h"

static bool_t is_init[NUM_SPI_USCIs] = {FALSE, FALSE};

static const msp_pin_t spi_pins[NUM_SPI_USCIs][NUM_SPI_PINS] = {
  {
    // USCI_A1
    // CLK , SOMI, SIMO
       p4_0, p4_5, p4_4
  },
  {
    // USCI_B1
    // CLK , SOMI, SIMO
       p4_3, p4_2, p4_1
  }
};

spi_ret_t spiInit(spi_usci_t usci)
{
  // Only initialize this USCI if it hasn't already been initialized (useful if
  // multiple libraries initialize the same SPI USCI)
  if (!is_init[usci])
    is_init[usci] = TRUE;
  else
    return SPI_REINIT;

  // Which USCI are we initializing?
  switch (usci)
  {
    case SPI_A1:
      // Enable spi using the A1 Port SEL register
      pinSelOn(spi_pins[SPI_A1][SPI_USCI_CLK]);
      pinSelOn(spi_pins[SPI_A1][SPI_USCI_SIMO]);

      // Put state machine in reset
      on(UCA1CTL1, UCSWRST);

      // 3-pin, 8-bit SPI master
      // Clock polarity high, MSB
      on(UCA1CTL0, (UCMST | UCSYNC | UCCKPH | UCMSB));

      // SMCLK
      on(UCA1CTL1, UCSSEL_2);

      // Set the prescaler to max (2) initially
      spiSetMaxPrescaler(usci);

      // Initialize USCI state machine
      off(UCA1CTL1, UCSWRST);

    case SPI_B1:
      // Enable spi using the B1 Port SEL register
      pinSelOn(spi_pins[SPI_B1][SPI_USCI_CLK]);
      pinSelOn(spi_pins[SPI_B1][SPI_USCI_SIMO]);

      // Put state machine in reset
      on(UCB1CTL1, UCSWRST);

      // 3-pin, 8-bit SPI master
      // Clock polarity high, MSB
      on(UCB1CTL0, (UCMST | UCSYNC | UCCKPH | UCMSB));

      // SMCLK
      on(UCB1CTL1, UCSSEL_2);

      // Set the prescaler to max (2) initially
      spiSetMaxPrescaler(usci);

      // Initialize USCI state machine
      off(UCB1CTL1, UCSWRST);

    default:
      return SPI_USCI_DNE;
  }
  return SPI_NO_ERR;
}

uint16_t spiGetPrescaler(spi_usci_t usci)
{
  switch (usci)
  {
    case SPI_A1:
      return ((UCA1BR1 << BYTE_SIZE) | UCA1BR0);
    case SPI_B1:
      return ((UCB1BR1 << BYTE_SIZE) | UCB1BR0);
    default:
      return 0;
  }
}

spi_ret_t spiPulseClk(spi_usci_t usci)
{
  switch (usci)
  {
    case SPI_A1:
      // Need to first disable the SPI functionality of the pin
      pinSelOff(spi_pins[SPI_A1][SPI_USCI_CLK]);
      // Now make sure it's an output
      pinOutput(spi_pins[SPI_A1][SPI_USCI_CLK]);
      // Now pulse the pin
      pinPulse(spi_pins[SPI_A1][SPI_USCI_CLK]);
      // Now turn the SPI functionality back on
      pinSelOn(spi_pins[SPI_A1][SPI_USCI_CLK]);
      break;
    case SPI_B1:
      // Need to first disable the SPI functionality of the pin
      pinSelOff(spi_pins[SPI_B1][SPI_USCI_CLK]);
      // Now make sure it's an output
      pinOutput(spi_pins[SPI_B1][SPI_USCI_CLK]);
      // Now pulse the pin
      pinPulse(spi_pins[SPI_B1][SPI_USCI_CLK]);
      // Now turn the SPI functionality back on
      pinSelOn(spi_pins[SPI_B1][SPI_USCI_CLK]);
      break;
    default:
      return SPI_USCI_DNE;
  }
  return SPI_NO_ERR;
}

spi_ret_t spiFallingEdge(spi_usci_t usci)
{
  switch (usci)
  {
    case SPI_A1:
      off(UCA1CTL0, UCCKPH);
      break;
    case SPI_B1:
      off(UCB1CTL0, UCCKPH);
      break;
    default:
      return SPI_USCI_DNE;
  }
  return SPI_NO_ERR;
}

spi_ret_t spiRisingEdge(spi_usci_t usci)
{
  switch (usci)
  {
    case SPI_A1:
      on(UCA1CTL0, UCCKPH);
      break;
    case SPI_B1:
      on(UCB1CTL0, UCCKPH);
      break;
    default:
      return SPI_USCI_DNE;
  }
  return SPI_NO_ERR;
}

spi_ret_t spiSetMaxPrescaler(spi_usci_t usci)
{
  switch (usci)
  {
    case SPI_A1:
      // Set the prescaler to 2 (max)
      UCA1BR0 = 0x02;
      UCA1BR1 = 0;
      break;
    case SPI_B1:
      // Set the prescaler to 2 (max)
      UCB1BR0 = 0x02;
      UCB1BR1 = 0;
      break;
    default:
      return SPI_USCI_DNE;
   }
  return SPI_NO_ERR;
}

spi_ret_t spiSetPrescaler(spi_usci_t usci, uint16_t prescaler)
{
  switch (usci)
  {
    case SPI_A1:
      // Set the prescaler based on the prescaler input
      UCA1BR0 = (prescaler & 0x00FF);
      UCA1BR1 = ((prescaler >> BYTE_SIZE) & 0x00FF);
      break;
    case SPI_B1:
      // Set the prescaler based on the prescaler input
      UCB1BR0 = (prescaler & 0x00FF);
      UCB1BR1 = ((prescaler >> BYTE_SIZE) & 0x00FF);
      break;
    default:
      return SPI_USCI_DNE;
  }
  return SPI_NO_ERR;
}

spi_ret_t spiWrite(spi_usci_t usci, uint8_t byte)
{
  // Which USCI to write to?
  switch (usci)
  {
    case SPI_A1:
      UCA1TXBUF = byte;
      while ((UCA1STAT&UCBUSY));
      break;
    case SPI_B1:
      UCB1TXBUF = byte;
      while ((UCB1STAT&UCBUSY));
      break;
    default:
      return SPI_USCI_DNE;
  }
  return SPI_NO_ERR;
}

