// Copyright (C) [2013] [AB2 Technologies] [Austin Beam, Alan Bullick]
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

#include "timerA_fw.h"

const uint16_t msp_timerA_base[MSP_TIMERA_SIZE] = {
  (uint16_t) &TA0CTL, (uint16_t) &TA0CTL, (uint16_t) &TA0CTL, \
    (uint16_t) &TA0CTL, (uint16_t) &TA0CTL,
  (uint16_t) &TA1CTL, (uint16_t) &TA1CTL, (uint16_t) &TA1CTL,
  (uint16_t) &TA2CTL, (uint16_t) &TA2CTL, (uint16_t) &TA2CTL
};
const uint16_t msp_timerA_cctl[MSP_TIMERA_SIZE] = {
  (uint16_t) &TA0CCTL0, (uint16_t) &TA0CCTL1, (uint16_t) &TA0CCTL2, \
    (uint16_t) &TA0CCTL3, (uint16_t) &TA0CCTL4,
  (uint16_t) &TA1CCTL0, (uint16_t) &TA1CCTL1, (uint16_t) &TA1CCTL2,
  (uint16_t) &TA2CCTL0, (uint16_t) &TA2CCTL1, (uint16_t) &TA2CCTL2
};
const uint16_t msp_timerA_ccr[MSP_TIMERA_SIZE] = {
  (uint16_t) &TA0CCR0, (uint16_t) &TA0CCR1, (uint16_t) &TA0CCR2, \
    (uint16_t) &TA0CCR3, (uint16_t) &TA0CCR4,
  (uint16_t) &TA1CCR0, (uint16_t) &TA1CCR1, (uint16_t) &TA1CCR2,
  (uint16_t) &TA2CCR0, (uint16_t) &TA2CCR1, (uint16_t) &TA2CCR2
};
