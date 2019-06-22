/**
 * \file IfxCcu6_TPwm.c
 * \brief CCU6 TPWM details
 *
 * \version iLLD_1_0_1_8_0
 * \copyright Copyright (c) 2018 Infineon Technologies AG. All rights reserved.
 *
 *
 *                                 IMPORTANT NOTICE
 *
 *
 * Infineon Technologies AG (Infineon) is supplying this file for use
 * exclusively with Infineon's microcontroller products. This file can be freely
 * distributed within development tools that are supporting such microcontroller
 * products.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include "IfxCcu6_TPwm.h"

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

void IfxCcu6_TPwm_initModule(IfxCcu6_TPwm *tPwm, const IfxCcu6_TPwm_Config *config)
{
    Ifx_CCU6 *ccu6SFR = config->ccu6; // pointer to CCU6 registers
    tPwm->ccu6 = ccu6SFR;             // adding register pointer to module handler

    /* -- hardware module initialisation -- */

    // enable module if it hasn't been enabled by any other interface //
    if (IfxCcu6_isModuleEnabled(ccu6SFR) == FALSE)
    {
        IfxCcu6_enableModule(ccu6SFR);
    }

    /* -- timer initialisation -- */

    // Timer 13 initialisation //

    if (config->timer == IfxCcu6_TimerId_t13)
    {
        // enable Timer13 if it hasn't been enabled by any other interface //
        if (IfxCcu6_getTimerAvailabilityStatus(ccu6SFR, IfxCcu6_TimerId_t13) == FALSE)
        {
            IfxCcu6_enableTimer(ccu6SFR, IfxCcu6_TimerId_t13);
        }

        // clock initialisation //

        if (config->clock.t13ExtClockEnabled)   // if external source is selected as reference
        {
            // input signal selection
            if (config->clock.t13ExtClockInput != NULL_PTR)
            {
                IfxCcu6_setT13InputSignal(config->ccu6, config->clock.t13ExtClockInput);
            }

            // counting input mode selection
            IfxCcu6_setCountingInputMode(ccu6SFR, IfxCcu6_TimerId_t13, config->clock.t13countingInputMode);

            // period setting
            IfxCcu6_setT13PeriodValue(ccu6SFR, (uint16)config->base.period);
        }
        else    // if internal clock is selected as reference
        {
            IfxCcu6_setT13Frequency(ccu6SFR, config->base.frequency, config->base.period);
        }

        // duty cycle initialisation //

        IfxCcu6_setT13CounterValue(ccu6SFR, config->timer13.counterValue);

        IfxCcu6_setT13CompareValue(ccu6SFR, config->timer13.compareValue);

        // if Timer 13 start is in sync with Timer 12 //
        if (config->trigger.t13InSyncWithT12)
        {
            IfxCcu6_setT13TriggerEventMode(ccu6SFR, config->timer13.t12SyncEvent);
            IfxCcu6_setT13TriggerEventDirection(ccu6SFR, config->timer13.t12SyncDirection);
        }
    }
    else
    {}

    // Timer 12 initialisation //

    if ((config->timer == IfxCcu6_TimerId_t12) || (config->trigger.t13InSyncWithT12))
    {
        Ifx_TimerValue period       = config->base.period;
        Ifx_TimerValue compareValue = config->timer12.compareValue;

        // enable Timer12 if it hasn't been enabled by any other interface //
        if (IfxCcu6_getTimerAvailabilityStatus(ccu6SFR, IfxCcu6_TimerId_t12) == FALSE)
        {
            IfxCcu6_enableTimer(ccu6SFR, IfxCcu6_TimerId_t12);
        }

        // if Timer 13 start is in sync with Timer 12 //
        if ((config->trigger.t13InSyncWithT12) && (config->base.waitingTime != 0))
        {
            // if in sync with T12 period match
            if (config->timer13.t12SyncEvent == IfxCcu6_T13TriggerEvent_onT12Period)
            {
                period = config->base.waitingTime;         // waiting time as period
            }
            else if (config->timer13.t12SyncEvent != IfxCcu6_T13TriggerEvent_noAction)
            {
                compareValue = config->base.waitingTime;           // waiting time as compare value
                period       = config->base.activeCount;
            }
        }
        else
        {
            // configuration error
        }

        // clock initialisation //

        if (config->clock.t12ExtClockEnabled)   // if external source is selected as reference
        {
            // input signal selection
            if (config->clock.t12ExtClockInput != NULL_PTR)
            {
                IfxCcu6_setT12InputSignal(config->ccu6, config->clock.t12ExtClockInput);
            }

            // counting input mode selection
            IfxCcu6_setCountingInputMode(ccu6SFR, IfxCcu6_TimerId_t12, config->clock.t12countingInputMode);

            // countining mode selection
            IfxCcu6_setT12CountMode(ccu6SFR, config->timer12.countMode);

            // period selection for center aligned mode
            if (config->timer12.countMode == IfxCcu6_T12CountMode_centerAligned)
            {
                period = (period / 2) - 1;
            }

            // period setting
            IfxCcu6_setT12PeriodValue(ccu6SFR, (uint16)period);
        }
        else    // if internal clock is selected as reference
        {
            // in case of in sync with T13 T12 runs at same frequency as T13
            IfxCcu6_setT12Frequency(ccu6SFR, config->base.frequency, period, config->timer12.countMode);
        }

        // duty cycle initialisation //

        IfxCcu6_setT12CounterValue(ccu6SFR, config->timer12.counterValue);

        IfxCcu6_setT12ChannelMode(ccu6SFR, config->timer12.channelId, config->timer12.channelMode);

        IfxCcu6_setT12CompareValue(ccu6SFR, config->timer12.channelId, (uint16)compareValue);

        // dead time injection if selected
        if (config->timer12.deadTimeValue)
        {
            IfxCcu6_setDeadTimeValue(ccu6SFR, config->timer12.deadTimeValue);
            IfxCcu6_enableDeadTime(ccu6SFR, config->timer12.channelId);
        }
    }
    else
    {}

    /* -- output path initialisation -- */

    //enable modulation output path //
    IfxCcu6_enableModulationOutput(ccu6SFR, config->timer, config->channelOut);

    // output passive logic configuration //
    //TODO check correct polarity:
    IfxCcu6_setOutputPassiveState(ccu6SFR, config->channelOut, config->base.activeState);

    /* -- Pin mapping -- */

    const IfxCcu6_TPwm_Pins *pins = config->pins;

    if (pins != NULL_PTR)
    {
        IfxCcu6_Cc60_Out *cc60Out = pins->cc60Out;

        if (cc60Out != NULL_PTR)
        {
            IfxCcu6_initCc60OutPin(cc60Out, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_Cc61_Out *cc61Out = pins->cc61Out;

        if (cc61Out != NULL_PTR)
        {
            IfxCcu6_initCc61OutPin(cc61Out, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_Cc62_Out *cc62Out = pins->cc62Out;

        if (cc62Out != NULL_PTR)
        {
            IfxCcu6_initCc62OutPin(cc62Out, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_Cout60_Out *cout60 = pins->cout60;

        if (cout60 != NULL_PTR)
        {
            IfxCcu6_initCout60Pin(cout60, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_Cout61_Out *cout61 = pins->cout61;

        if (cout61 != NULL_PTR)
        {
            IfxCcu6_initCout61Pin(cout61, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_Cout62_Out *cout62 = pins->cout62;

        if (cout62 != NULL_PTR)
        {
            IfxCcu6_initCout62Pin(cout62, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_Cout63_Out *cout63 = pins->cout63;

        if (cout63 != NULL_PTR)
        {
            IfxCcu6_initCout63Pin(cout63, pins->outputMode, pins->pinDriver);
        }

        IfxCcu6_T12hr_In *t12hr = pins->t12hr;

        if (t12hr != NULL_PTR)
        {
            IfxCcu6_initT12hrPin(t12hr, pins->t1xhrInputMode);
        }

        IfxCcu6_T13hr_In *t13hr = pins->t13hr;

        if (t13hr != NULL_PTR)
        {
            IfxCcu6_initT13hrPin(t13hr, pins->t1xhrInputMode);
        }
    }

    /* -- interrupt initialisation -- */

    if (config->interrupt1.priority > 0)
    {
        IfxCcu6_enableInterrupt(config->ccu6, config->interrupt1.source);
        IfxCcu6_routeInterruptNode(config->ccu6, config->interrupt1.source, config->interrupt1.serviceRequest);

        volatile Ifx_SRC_SRCR *src;
        src = IfxCcu6_getSrcAddress(config->ccu6, config->interrupt1.serviceRequest);
        IfxSrc_init(src, config->interrupt1.typeOfService, config->interrupt1.priority);
        IfxSrc_enable(src);
    }

    if (config->interrupt2.priority > 0)
    {
        IfxCcu6_enableInterrupt(config->ccu6, config->interrupt2.source);
        IfxCcu6_routeInterruptNode(config->ccu6, config->interrupt2.source, config->interrupt2.serviceRequest);

        volatile Ifx_SRC_SRCR *src;
        src = IfxCcu6_getSrcAddress(config->ccu6, config->interrupt2.serviceRequest);
        IfxSrc_init(src, config->interrupt2.typeOfService, config->interrupt2.priority);
        IfxSrc_enable(src);
    }

    if (config->interrupt3.priority > 0)
    {
        IfxCcu6_enableInterrupt(config->ccu6, config->interrupt3.source);
        IfxCcu6_routeInterruptNode(config->ccu6, config->interrupt3.source, config->interrupt3.serviceRequest);

        volatile Ifx_SRC_SRCR *src;
        src = IfxCcu6_getSrcAddress(config->ccu6, config->interrupt3.serviceRequest);
        IfxSrc_init(src, config->interrupt3.typeOfService, config->interrupt3.priority);
        IfxSrc_enable(src);
    }

    if (config->interrupt4.priority > 0)
    {
        IfxCcu6_enableInterrupt(config->ccu6, config->interrupt4.source);
        IfxCcu6_routeInterruptNode(config->ccu6, config->interrupt4.source, config->interrupt4.serviceRequest);

        volatile Ifx_SRC_SRCR *src;
        src = IfxCcu6_getSrcAddress(config->ccu6, config->interrupt4.serviceRequest);
        IfxSrc_init(src, config->interrupt4.typeOfService, config->interrupt4.priority);
        IfxSrc_enable(src);
    }

    /* -- output trigger initialisation --*/

    if (config->trigger.outputTriggerEnabled)
    {
        IfxCcu6_connectTrigger(ccu6SFR, config->trigger.outputLine, config->trigger.outputTrigger);
    }

    tPwm->timer      = config->timer;
    tPwm->channelOut = config->channelOut;
    tPwm->trigger    = config->trigger;

#if IFX_CFG_USE_STANDARD_INTERFACE
    IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, (uint32)tPwm == ((uint32)&tPwm->base));
    tPwm->base.functions.start  = (TPwm_Start) & IfxCcu6_TPwm_start;
    tPwm->base.functions.pause  = (TPwm_Pause) & IfxCcu6_TPwm_pause;
    tPwm->base.functions.resume = (TPwm_Resume) & IfxCcu6_TPwm_resume;
    tPwm->base.functions.stop   = (TPwm_Stop) & IfxCcu6_TPwm_stop;
#endif
}


void IfxCcu6_TPwm_initModuleConfig(IfxCcu6_TPwm_Config *config, Ifx_CCU6 *ccu6)
{
    const IfxCcu6_TPwm_Config defaultConfig = {
        .ccu6             = NULL_PTR, // will be initialized below

        .base.frequency   = 400000,
        .base.period      = 100,
        .base.waitingTime = 20,
        .base.activeState = Ifx_ActiveState_high,

        .timer            = IfxCcu6_TimerId_t13,

        .clock            = {
            .t12ExtClockEnabled   = FALSE,
            .t12ExtClockInput     = NULL_PTR,
            .t12countingInputMode = IfxCcu6_CountingInputMode_internal,

            .t13ExtClockEnabled   = FALSE,
            .t13ExtClockInput     = NULL_PTR,
            .t13countingInputMode = IfxCcu6_CountingInputMode_internal,
        },

        .timer12                  = {
            .channelId     = IfxCcu6_T12Channel_0,
            .channelMode   = IfxCcu6_T12ChannelMode_compareMode,
            .countMode     = IfxCcu6_T12CountMode_edgeAligned,
            .counterValue  = 0,
            .deadTimeValue = 0,
            .compareValue  = 0,
        },

        .timer13                  = {
            .counterValue     = 0,
            .compareValue     = 0,
            .t12SyncEvent     = IfxCcu6_T13TriggerEvent_onCC60RCompare,
            .t12SyncDirection = IfxCcu6_T13TriggerDirection_onT12CountingUp,
        },

        .channelOut = IfxCcu6_ChannelOut_cout3,

        .pins       = NULL_PTR,

        .interrupt1 = {
            .source         = IfxCcu6_InterruptSource_t12PeriodMatch,
            .serviceRequest = IfxCcu6_ServiceRequest_0,
            .priority       = 0,                // interrupt priority 0
            .typeOfService  = IfxSrc_Tos_cpu0,  // type of service CPU0
        },

        .interrupt2               = {
            .source         = IfxCcu6_InterruptSource_t13PeriodMatch,
            .serviceRequest = IfxCcu6_ServiceRequest_1,
            .priority       = 0,                // interrupt priority 0
            .typeOfService  = IfxSrc_Tos_cpu0,  // type of service CPU0
        },

        .interrupt3               = {
            .source         = IfxCcu6_InterruptSource_t12OneMatch,
            .serviceRequest = IfxCcu6_ServiceRequest_2,
            .priority       = 0,                // interrupt priority 0
            .typeOfService  = IfxSrc_Tos_cpu0,  // type of service CPU0
        },

        .interrupt4               = {
            .source         = IfxCcu6_InterruptSource_trap,
            .serviceRequest = IfxCcu6_ServiceRequest_3,
            .priority       = 0,                // interrupt priority 0
            .typeOfService  = IfxSrc_Tos_cpu0,  // type of service CPU0
        },

        .trigger                  = {
            .t12ExtInputTrigger   = NULL_PTR,
            .t13ExtInputTrigger   = NULL_PTR,
            .extInputTriggerMode  = IfxCcu6_ExternalTriggerMode_risingEdge,
            .t13InSyncWithT12     = TRUE,

            .outputTriggerEnabled = TRUE,
            .outputLine           = IfxCcu6_TrigOut_0,
            .outputTrigger        = IfxCcu6_TrigSel_cout63,
        },
    };

    /* Default Configuration */
    *config = defaultConfig;

    /* take over module pointer */
    config->ccu6 = ccu6;
}


void IfxCcu6_TPwm_pause(IfxCcu6_TPwm *tPwm)
{
    IfxCcu6_disableModulationOutput(tPwm->ccu6, tPwm->timer, tPwm->channelOut);
}


void IfxCcu6_TPwm_resume(IfxCcu6_TPwm *tPwm)
{
    IfxCcu6_enableModulationOutput(tPwm->ccu6, tPwm->timer, tPwm->channelOut);
}


void IfxCcu6_TPwm_start(IfxCcu6_TPwm *tPwm)
{
    // Timer 13 modulation //

    if (tPwm->timer == IfxCcu6_TimerId_t13)
    {
        // enable shadow transfer
        IfxCcu6_enableShadowTransfer(tPwm->ccu6, FALSE, TRUE);

        // start Timer 13 when not in sync with Timer 12
        if (!(tPwm->trigger.t13InSyncWithT12))
        {
            if (tPwm->trigger.t13ExtInputTrigger != NULL_PTR)
            {
                // external start
                IfxCcu6_setExternalRunMode(tPwm->ccu6, IfxCcu6_TimerId_t13, tPwm->trigger.extInputTriggerMode);
                IfxCcu6_setT13InputSignal(tPwm->ccu6, tPwm->trigger.t13ExtInputTrigger);
            }
            else
            {
                // internal start
                IfxCcu6_startTimer(tPwm->ccu6, FALSE, TRUE);
            }
        }
        else
        {}
    }
    else
    {}

    // Timer 12 alone for modulation or when in sync with Timer 13 //

    if ((tPwm->timer == IfxCcu6_TimerId_t12) || (tPwm->trigger.t13InSyncWithT12))
    {
        // enable shadow transfer
        IfxCcu6_enableShadowTransfer(tPwm->ccu6, TRUE, FALSE);

        //start Timer 12
        if (tPwm->trigger.t12ExtInputTrigger != NULL_PTR)
        {
            // external start
            IfxCcu6_setExternalRunMode(tPwm->ccu6, IfxCcu6_TimerId_t12, tPwm->trigger.extInputTriggerMode);
            IfxCcu6_setT12InputSignal(tPwm->ccu6, tPwm->trigger.t12ExtInputTrigger);
        }
        else
        {
            // internal start
            IfxCcu6_startTimer(tPwm->ccu6, TRUE, FALSE);
        }
    }
    else
    {}
}


void IfxCcu6_TPwm_stop(IfxCcu6_TPwm *tPwm)
{
    // Timer 13 modulation //

    if ((tPwm->timer == IfxCcu6_TimerId_t13) || (tPwm->trigger.t13InSyncWithT12))
    {
        // disable shadow transfer
        IfxCcu6_disableShadowTransfer(tPwm->ccu6, FALSE, TRUE);

        // remove external input trigger if any
        if (tPwm->trigger.t13ExtInputTrigger != NULL_PTR)
        {
            IfxCcu6_setExternalRunMode(tPwm->ccu6, IfxCcu6_TimerId_t13, IfxCcu6_ExternalTriggerMode_disable);
        }

        // remove the sync with Timer 12
        if (tPwm->trigger.t13InSyncWithT12)
        {
            IfxCcu6_setT13TriggerEventMode(tPwm->ccu6, IfxCcu6_T13TriggerEvent_noAction);
            IfxCcu6_setT13TriggerEventDirection(tPwm->ccu6, IfxCcu6_T13TriggerDirection_noAction);
        }

        // stop Timer 13
        IfxCcu6_stopTimer(tPwm->ccu6, FALSE, TRUE);
    }

    // Timer 12 modulation //

    else
    {
        // disable shadow transfer
        IfxCcu6_disableShadowTransfer(tPwm->ccu6, TRUE, FALSE);

        // remove external input trigger if any
        if (tPwm->trigger.t12ExtInputTrigger != NULL_PTR)
        {
            IfxCcu6_setExternalRunMode(tPwm->ccu6, IfxCcu6_TimerId_t12, IfxCcu6_ExternalTriggerMode_disable);
        }

        // stop Timer 12
        IfxCcu6_stopTimer(tPwm->ccu6, TRUE, FALSE);
    }

    // disable the modulation
    IfxCcu6_disableModulationOutput(tPwm->ccu6, tPwm->timer, tPwm->channelOut);
}
