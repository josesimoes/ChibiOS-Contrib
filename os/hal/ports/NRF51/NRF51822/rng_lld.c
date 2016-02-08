/*
    RNG for ChibiOS - Copyright (C) 2016 Stephane D'Alu

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    NRF51/RNGv1/rng_lld.c
 * @brief   NRF51 RNG subsystem low level driver source.
 *
 * @addtogroup RNG
 * @{
 */

#include "hal.h"

#if (HAL_USE_RNG == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   RNG default configuration.
 */
static const RNGConfig default_config = {
    .digital_error_correction = 1,
    .power_on_write           = 1,
};

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief RNG1 driver identifier.*/
#if NRF51_RNG_USE_RNG1 || defined(__DOXYGEN__)
RNGDriver RNGD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level RNG driver initialization.
 *
 * @notapi
 */
void rng_lld_init(void) {
  rngObjectInit(&RNGD1);
  RNGD1.rng    = NRF_RNG;
}

/**
 * @brief   Configures and activates the RNG peripheral.
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @notapi
 */
void rng_lld_start(RNGDriver *rngp) {
  if (rngp->config == NULL)
    rngp->config = &default_config;

  rngp->rng->POWER = 1;

  if (rngp->config->digital_error_correction) 
    rngp->rng->CONFIG |=  RNG_CONFIG_DERCEN_Msk;
  else
    rngp->rng->CONFIG &= ~RNG_CONFIG_DERCEN_Msk;

  rngp->rng->INTENSET    = RNG_INTENSET_VALRDY_Msk;
  rngp->rng->TASKS_START = 1;
}


/**
 * @brief   Deactivates the RNG peripheral.
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @notapi
 */
void rng_lld_stop(RNGDriver *rngp) {
  rngp->rng->TASKS_STOP = 1;
  rngp->rng->POWER = 0;
}


/**
 * @brief   Write random bytes;
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 * @param[in] n         size of buf in bytes
 * @param[in] buf       @p buffer location
 *
 * @notapi
 */
msg_t rng_lld_write(RNGDriver *rngp, uint8_t *buf, size_t n,
		    systime_t timeout) {
  size_t i;
  if (n == 0)
    return MSG_OK;

  if (n == 1)
	rngp->rng->SHORTS |= RNG_SHORTS_VALRDY_STOP_Msk;

  

  NRF_RNG->EVENTS_VALRDY = 0;
    
  for (i = 0 ; i < n ; i++) {
    /* sleep until number is generated */
    while (NRF_RNG->EVENTS_VALRDY == 0) {
      /* enable wake up on events for __WFE CPU sleep */
      SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
      /* sleep until next event */
      __SEV();
      __WFE();
      __WFE();
    }

    buf[i] = (char)NRF_RNG->VALUE;

    NRF_RNG->EVENTS_VALRDY = 0;
    nvicClearPending(RNG_IRQn);
  }
  return MSG_OK;
}

#endif /* HAL_USE_RNG */

/** @} */
