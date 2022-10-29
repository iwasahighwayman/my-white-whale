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
    const uint TJ_HOURS_ON_5 = 1; // (60 * 60 * 5);
    const uint TJ_HOURS_OFF_19 = 2 - 1; // (60 * 60 * 19) - 1;
    uint tjhourson = 0;
    uint tjhoursoff = 0;
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    const uint TJ_IN_PIN_DARK = 6;
    uint tjdark = 0;
    gpio_init(TJ_IN_PIN_DARK);
    gpio_set_dir(TJ_IN_PIN_DARK, GPIO_IN);
    gpio_set_pulls(TJ_IN_PIN_DARK, true, false);  //pin, pull-up, pull-down
    const uint TJ_IN_PIN_PWR = 7;
    uint tjpwr = 0;
    gpio_init(TJ_IN_PIN_PWR);
    gpio_set_dir(TJ_IN_PIN_PWR, GPIO_IN);
    gpio_set_pulls(TJ_IN_PIN_PWR, true, false);  //pin, pull-up, pull-down
    const uint TJ_OUT_PIN = 0;
    gpio_init(TJ_OUT_PIN);
    gpio_set_dir(TJ_OUT_PIN, GPIO_OUT);
    stdio_init_all();
    uint tjpwrctr = 0;
    uint tjpwron = 0;
    uint tjledon = 0;

    const uint TJ_PWM_PIN = 1;
    printf("light6_usb pwm pin %d\n", TJ_PWM_PIN);
    gpio_set_function(TJ_PWM_PIN, GPIO_FUNC_PWM);
    uint tjslicenum = pwm_gpio_to_slice_num(TJ_PWM_PIN);
    printf("light6_usb tjslicenum %d\n", tjslicenum);
    uint tjchan = pwm_gpio_to_channel(TJ_PWM_PIN);
    printf("light6_usb tjchan %d\n", tjchan);
    //pwm_set_freq_duty(tjslicenum, tjchan, 50, 75);
    pwm_set_freq_duty(tjslicenum, tjchan, 100, 50);
    pwm_set_enabled(tjslicenum, false);

    while (true) {
        if (tjpwron) {
            gpio_put(LED_PIN, 1);
            printf("light6_usb power ON pin %d\n", LED_PIN);
        }
        else {
            gpio_put(LED_PIN, 0);
            printf("light6_usb power OFF pin %d\n", LED_PIN);
        }
        if (tjpwron) {
            if (tjhourson > 0) {
                tjledon = 1;
                tjhourson--;
            }
            else {
                if (tjhoursoff > 0) {
                    tjledon = 0;
                    tjhoursoff--;
                }
                else {
                    tjhourson = TJ_HOURS_ON_5;
                    tjhoursoff = TJ_HOURS_OFF_19;
                }
            }
            printf("light6_usb tjhourson is %d\n", tjhourson);
            printf("light6_usb tjhoursoff is %d\n", tjhoursoff);
        }
        tjpwr = !gpio_get(TJ_IN_PIN_PWR);
        printf("light6_usb tjpwr pin %d is %d\n", TJ_IN_PIN_PWR, tjpwr);
        if (tjpwr) {
            if (tjpwrctr < 3) {
                printf("light6_usb tjpwrctr is %d so less than 3 so incrementing\n", tjpwrctr);
                tjpwrctr++;
            }
            else if (tjpwrctr == 3) {
                printf("light6_usb tjpwrctr is %d so flipping power state\n", tjpwrctr);
                if (tjpwron == 0) {
                    tjpwron = 1;
                    tjhourson = TJ_HOURS_ON_5;
                    tjhoursoff = TJ_HOURS_OFF_19;
                }
                else {
                    tjpwron = 0;
                    tjledon = 0;
                    tjhourson = 0;
                    tjhoursoff = 0;
                }
                printf("light6_usb set tjpwron to %d\n", tjpwron);
                tjpwrctr++;
            }
        }
        else {
            tjpwrctr = 0;
        }
        // This is the "testing" line
        // Uncomment the following to test during the day
        //  and comment-out the "real" line below
        //tjdark = gpio_get(TJ_IN_PIN_DARK);
        // This is the "real" line with "!" NOT operater
        // The 555 dark=high light=low
        // But 2n7000 FET plus built-in pull-up resistor
        //  reverses to be dark=low light=high
        tjdark = !gpio_get(TJ_IN_PIN_DARK);
        printf("light6_usb tjdark pin %d is %d\n", TJ_IN_PIN_DARK, tjdark);
        if (tjdark) {
            if (tjpwron) {
                if (tjhourson) {
                    tjledon = 1;
                }
                else {
                    tjledon = 0;
                }
            }
        }
        else {
            tjledon = 0;
        }
        if (tjledon) {
            gpio_put(TJ_OUT_PIN, 1);
            printf("light6_usb led ON pin %d\n", TJ_OUT_PIN);
            pwm_set_enabled(tjslicenum, true);
            printf("light6_usb pwm led ON pin %d\n", TJ_PWM_PIN);
        }
        else {
            gpio_put(TJ_OUT_PIN, 0);
            printf("light6_usb led OFF pin %d\n", TJ_OUT_PIN);
            pwm_set_enabled(tjslicenum, false);
            printf("light6_usb pwm led OFF pin %d\n", TJ_PWM_PIN);
            // N.B.: pwm pin NOT guaranteed to be low 0V when stop pwm - might remain high 3.45V
        }
        sleep_ms(1000);
    }
#endif
    return 0;
}
