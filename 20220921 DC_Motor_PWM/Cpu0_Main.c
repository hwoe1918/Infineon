/**********************************************************************************************************************
 * \file Cpu0_Main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 * 
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of 
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 * 
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and 
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all 
 * derivative works of the Software, unless such copies or derivative works are solely in the form of 
 * machine-executable object code generated by a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "ADC_Single_Channel.h"
#include "Motor_control_pwm.h"
#include "Bsp.h"
#include "GPT12_PWM_Generation.h"

#define WAIT_TIME   100
#define a           0.5
IfxCpu_syncEvent g_cpuSyncEvent = 0;

int adc = 0;
float pre_esti = 0.0;
int pwm = 0;
int r_pwm = 0;
int H_L = 0;

float Low_pass_fil(float pre_esti, float X);

int core0_main(void)
{
    IfxCpu_enableInterrupts();
    
    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
    
    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);
    
    /* Initialize a time variable */
//    Ifx_TickTime ticksFor100ms = IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, WAIT_TIME);

    initializeLEDs();

    vadcBackgroundScanInit();
    vadcBackgroundScanRun();

    L298_Pin_init();

    initGpt12PWM();
    runGpt12PWM();

    while(1)
    {
        adc = indicateConversionValue();
        pre_esti = Low_pass_fil(pre_esti, adc);

        pwm = (pre_esti*100)/4095;     //pwm control
        H_L = pre_esti*(255.0/4095);

        if(pwm < 50)
        {
            r_pwm = 100 - pwm;
            Set_GptPWM_Duty(r_pwm);
        }
        else
        {
            Set_GptPWM_Duty(pwm);
        }

        motor_control(H_L);

    }
    return (1);
}

float Low_pass_fil(float pre_esti, float X)
{
    float new_esti = 0.0;

    //��ͽ� avg_x(k) = avg_x(k-1) + x(k)/n - x(k-n)/n
    new_esti = a * pre_esti + (1 - a) * X;

    return new_esti;
}
