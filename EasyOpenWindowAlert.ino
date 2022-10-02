
/*
   This file is part of EasyOpenWindowAlert Arduino project for an ATtiny85 Energy saver device.
   It soinds an alert when the Window (Reed Conact) ist open for more  than 5 minutes
   Copyright (c) 2022 by Siggi Sorglos (siggi.sorglos2@gmx.de)
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

/*
 * Attiny85 PINS
 *                ____
 * RESET        -|*   |- 5V
 * SWITCH (PB3) -|    |- (PB2) NOT_USED
 * LED    (PB4) -|    |- (PB1) BEEP_MINUS
 * GND          -|____|- (PB0) BEEP_PLUS
 */


#include <avr/interrupt.h> // needed for the additional interrupt
#include <avr/sleep.h>     // Sleep Modes
#include <avr/io.h>
#include <avr/power.h>
#include "TimerFreeTone.h" // see https://forum.arduino.cc/t/timerfreetone-library-v1-5-play-tones-without-timers-and-therefore-no-conflicts/229448

#define WD_TIMEOUT 4 // watchdog timeout 4 sec

// application parameters
#define DELAY_ALERT 5 // delay between window opening and start of alert in minutes
#define NUM_REPEAT 3  // max. 3 repetitions of the alert.
#define DURATION_ALERT 3 // duration of the alert in seconds

#define NUM_WD (60 * DELAY_ALERT) / WD_TIMEOUT // number of watchdog cycles before alert 

#define   F_CPU   1000000UL  // clock rate to make delay(), millis(), micros() work correct

// hardware configuration constants
#define LED PB4
#define SWITCH PB3
#define BEEP_PLUS PB0
#define BEEP_MINUS PB1

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

#define WINDOW_OPEN digitalRead(SWITCH) // window open / input high => digitalRead(SWITCH), window open / input low => !digitalRead(SWITCH)
// interrupt routines
ISR (PCINT0_vect){ // switch interrupt
  // pin change wake up interrupt, do nothing
}

ISR(WDT_vect) { // warchdog interrupt
  // wdt wake up interrupt, do nothing
}                  

// helper functions
void powerDown() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sleep_bod_disable();
  sei();
  sleep_cpu();
  sleep_disable();
  sei();
}

void watchdogOff(){
  cli();
  asm("WDR");
  MCUSR &= ~ bit (WDRF);
  WDTCR |= bit (WDCE) | bit (WDE);
  WDTCR = 0x00;
  sei();
}

void configurePinChangeInterrupt() {
  // configure pin change interrupt
  PCMSK  |= bit (PCINT3);  // want pin 3
  GIFR   |= bit (PCIF);    // clear any outstanding interrupts
  GIMSK  |= bit (PCIE);    // enable pin change interrupts
  sei();  // enable interrupts
}

// setup
void setup() {
  //clock_prescale_set(clock_div_8);    // Prescaler 8 => CPU-Clock = 1 MHz, Prescaler 256 => CPU-CLock = 31,25 kHz
  // put your setup code here, to run once:
  pinMode(SWITCH, INPUT);  //  no external pullup / pulldown => pinMode(SWITCH, INPUT_PULLUP);
  pinMode(BEEP_PLUS, OUTPUT);
  pinMode(BEEP_MINUS, OUTPUT);
  pinMode(LED, OUTPUT);

  // short blink at startup to inidicate readyness.
  digitalWrite(LED, HIGH);
  delay (200);
  digitalWrite(LED, LOW);
  delay (200);
}

// main loop
void loop() {
  // sleep and use pin change interrupt to wake up
  // double blink before going to sleep
  for (int i = 0; i<2; i++) {
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
  }
  adc_disable(); 
  configurePinChangeInterrupt(); 
  powerDown();             
  GIMSK  &= ~bit (PCIE);      // disable pin change interrupt. do not wake up by pin change if waiting for watchdog is active
  //cli();                    // wake up here, disable interrupts 
  delay (100);                // avoid bouncing contacts
  
  digitalWrite(LED, HIGH);
  if (WINDOW_OPEN) 
    delay(2000);
  else
    delay(500);
  digitalWrite(LED, LOW);
  
  for (int repeat = 0; repeat < NUM_REPEAT; repeat++) {  // repeat alert max. NUM_REPEAT times
    // delay alert in sleeping mode with watchdog timeout interrupt
    for (int num_wd_cycle = 0; num_wd_cycle < NUM_WD; num_wd_cycle++) { // number of times the watchdog timer delays the alert   
      if (WINDOW_OPEN) {
        // flash LED
        digitalWrite(LED, HIGH);
        delay (100);
        digitalWrite(LED, LOW);
        //sei();                          // enable ISR
        WDTCR = bit (WDIE) | bit (WDP3); // | bit (WDP0);  // watchdog time 4s
        MCUCR = bit (SE) + bit (SM1);   // select power down mode
        sleep_cpu();                    // goto sleep
        //cli();                          //wake up here, disable interrupts
      }
      else break;
    }
      
    if (WINDOW_OPEN) { 
      //for (int freq = 2000; freq < 3000; freq += 10) {
      for (int i = 0; i < DURATION_ALERT; i++) { 
        if (WINDOW_OPEN) {
          // Alarm on + blink LED
          digitalWrite(LED, HIGH);
          TimerFreeTone(BEEP_PLUS, BEEP_MINUS, 2000, 500);
          digitalWrite(LED, LOW);
          delay (500);
        }
        else break; 
      }
    }
    else break;
  }
  watchdogOff();
}
