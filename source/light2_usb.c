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
    while (true) {
        if (mwwpwron) {
            gpio_put(LED_PIN, 1);
            printf("light2_usb power ON pin %d\n", LED_PIN);
        }
        else {
            gpio_put(LED_PIN, 0);
            printf("light2_usb power OFF pin %d\n", LED_PIN);
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
            printf("light2_usb mwwhourson is %d\n", mwwhourson);
            printf("light2_usb mwwhoursoff is %d\n", mwwhoursoff);
        }
        mwwpwr = !gpio_get(MWW_IN_PIN_PWR);
        printf("light2_usb mwwpwr pin %d is %d\n", MWW_IN_PIN_PWR, mwwpwr);
        if (mwwpwr) {
            if (mwwpwrctr < 3) {
                printf("light2_usb mwwpwrctr is %d so less than 3 so incrementing\n", mwwpwrctr);
                mwwpwrctr++;
            }
            else if (mwwpwrctr == 3) {
                printf("light2_usb mwwpwrctr is %d so flipping power state\n", mwwpwrctr);
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
                printf("light2_usb set mwwpwron to %d\n", mwwpwron);
                mwwpwrctr++;
            }
        }
        else {
            mwwpwrctr = 0;
        }
        mwwdark = !gpio_get(MWW_IN_PIN_DARK);
        printf("light2_usb mwwdark pin %d is %d\n", MWW_IN_PIN_DARK, mwwdark);
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
            printf("light2_usb led ON pin %d\n", MWW_OUT_PIN);
        }
        else {
            gpio_put(MWW_OUT_PIN, 0);
            printf("light2_usb led OFF pin %d\n", MWW_OUT_PIN);
        }
        sleep_ms(1000);
    }
#endif
    return 0;
}
