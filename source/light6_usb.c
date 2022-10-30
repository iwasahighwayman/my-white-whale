/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*
 *
 * MIT License
 * 
 * Copyright <YEAR> <COPYRIGHT HOLDER>
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"


//See: https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2

uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t f, int d) {
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / f / 4096 + (clock % (f * 4096) != 0);
    if (divider16 / 16 == 0) {
        divider16 = 16;
    }
    uint32_t wrap = clock * 16 / divider16 / f - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16/16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * d / 100);
    return wrap;
}

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    const uint MWW_HOURS_ON_5 = 1; // (60 * 60 * 5);
    const uint MWW_HOURS_OFF_19 = 2 - 1; // (60 * 60 * 19) - 1;
    uint mwwhourson = 0;
    uint mwwhoursoff = 0;
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    const uint MWW_IN_PIN_DARK = 6;
    uint mwwdark = 0;
    gpio_init(MWW_IN_PIN_DARK);
    gpio_set_dir(MWW_IN_PIN_DARK, GPIO_IN);
    gpio_set_pulls(MWW_IN_PIN_DARK, true, false);  //pin, pull-up, pull-down
    const uint MWW_IN_PIN_PWR = 7;
    uint mwwpwr = 0;
    gpio_init(MWW_IN_PIN_PWR);
    gpio_set_dir(MWW_IN_PIN_PWR, GPIO_IN);
    gpio_set_pulls(MWW_IN_PIN_PWR, true, false);  //pin, pull-up, pull-down
    const uint MWW_OUT_PIN = 0;
    gpio_init(MWW_OUT_PIN);
    gpio_set_dir(MWW_OUT_PIN, GPIO_OUT);
    stdio_init_all();
    uint mwwpwrctr = 0;
    uint mwwpwron = 0;
    uint mwwledon = 0;

    const uint MWW_PWM_PIN = 1;
    printf("light6_usb pwm pin %d\n", MWW_PWM_PIN);
    gpio_set_function(MWW_PWM_PIN, GPIO_FUNC_PWM);
    uint mwwslicenum = pwm_gpio_to_slice_num(MWW_PWM_PIN);
    printf("light6_usb mwwslicenum %d\n", mwwslicenum);
    uint mwwchan = pwm_gpio_to_channel(MWW_PWM_PIN);
    printf("light6_usb mwwchan %d\n", mwwchan);
    //pwm_set_freq_duty(mwwslicenum, mwwchan, 50, 75);
    pwm_set_freq_duty(mwwslicenum, mwwchan, 100, 50);
    pwm_set_enabled(mwwslicenum, false);

    while (true) {
        if (mwwpwron) {
            gpio_put(LED_PIN, 1);
            printf("light6_usb power ON pin %d\n", LED_PIN);
        }
        else {
            gpio_put(LED_PIN, 0);
            printf("light6_usb power OFF pin %d\n", LED_PIN);
        }
        if (mwwpwron) {
            if (mwwhourson > 0) {
                mwwledon = 1;
                mwwhourson--;
            }
            else {
                if (mwwhoursoff > 0) {
                    mwwledon = 0;
                    mwwhoursoff--;
                }
                else {
                    mwwhourson = MWW_HOURS_ON_5;
                    mwwhoursoff = MWW_HOURS_OFF_19;
                }
            }
            printf("light6_usb mwwhourson is %d\n", mwwhourson);
            printf("light6_usb mwwhoursoff is %d\n", mwwhoursoff);
        }
        mwwpwr = !gpio_get(MWW_IN_PIN_PWR);
        printf("light6_usb mwwpwr pin %d is %d\n", MWW_IN_PIN_PWR, mwwpwr);
        if (mwwpwr) {
            if (mwwpwrctr < 3) {
                printf("light6_usb mwwpwrctr is %d so less than 3 so incrementing\n", mwwpwrctr);
                mwwpwrctr++;
            }
            else if (mwwpwrctr == 3) {
                printf("light6_usb mwwpwrctr is %d so flipping power state\n", mwwpwrctr);
                if (mwwpwron == 0) {
                    mwwpwron = 1;
                    mwwhourson = MWW_HOURS_ON_5;
                    mwwhoursoff = MWW_HOURS_OFF_19;
                }
                else {
                    mwwpwron = 0;
                    mwwledon = 0;
                    mwwhourson = 0;
                    mwwhoursoff = 0;
                }
                printf("light6_usb set mwwpwron to %d\n", mwwpwron);
                mwwpwrctr++;
            }
        }
        else {
            mwwpwrctr = 0;
        }
        // This is the "testing" line
        // Uncomment the following to test during the day
        //  and comment-out the "real" line below
        //mwwdark = gpio_get(MWW_IN_PIN_DARK);
        // This is the "real" line with "!" NOT operater
        // The 555 dark=high light=low
        // But 2n7000 FET plus built-in pull-up resistor
        //  reverses to be dark=low light=high
        mwwdark = !gpio_get(MWW_IN_PIN_DARK);
        printf("light6_usb mwwdark pin %d is %d\n", MWW_IN_PIN_DARK, mwwdark);
        if (mwwdark) {
            if (mwwpwron) {
                if (mwwhourson) {
                    mwwledon = 1;
                }
                else {
                    mwwledon = 0;
                }
            }
        }
        else {
            mwwledon = 0;
        }
        if (mwwledon) {
            gpio_put(MWW_OUT_PIN, 1);
            printf("light6_usb led ON pin %d\n", MWW_OUT_PIN);
            pwm_set_enabled(mwwslicenum, true);
            printf("light6_usb pwm led ON pin %d\n", MWW_PWM_PIN);
        }
        else {
            gpio_put(MWW_OUT_PIN, 0);
            printf("light6_usb led OFF pin %d\n", MWW_OUT_PIN);
            pwm_set_enabled(mwwslicenum, false);
            printf("light6_usb pwm led OFF pin %d\n", MWW_PWM_PIN);
            // N.B.: pwm pin NOT guaranteed to be low 0V when stop pwm - might remain high 3.45V
        }
        sleep_ms(1000);
    }
#endif
    return 0;
}
