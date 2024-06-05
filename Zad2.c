/* 
 * File:   main.c
 * Author: local
 *
 * Created on March 27, 2024, 5:29 PM
 */

#include <stdio.h>
#include <stdlib.h>

// CONFIG2
#pragma config POSCMOD = HS             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRIPLL           // Oscillator Select (Primary Oscillator with PLL module (HSPLL, ECPLL))
#pragma config IESO = OFF               // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

#include "xc.h"

int main(void) {
    TRISA = 0x0000;
    TRISB = 0xFFFF;
    
    AD1CON1 = 0x80E4;
    AD1CON2 = 0x0404;
    AD1CON3 = 0x0F00;
    AD1CHS = 0x00;
    AD1CSSL = 0X20;
   
    unsigned value = 0;
    unsigned int alarmTime = 0, potValue = 0;
    char currentS6 = 0, currentS7 = 0, prevS6 = 0, prevS7 = 0;
    
    while(1){
        while(!AD1CON1bits.DONE);
        prevS6 = PORTDbits.RD6;
        prevS7 = PORTDbits.RD7;
        __delay32(1500000);
        currentS6 = PORTDbits.RD6;
        currentS7 = PORTDbits.RD7;
        potValue = ADC1BUF0;
        potValue = potValue>>2;
        if(potValue >= 127){
            alarmTime++;
            __delay32(10000000);
            LATA = 1;
            __delay32(10000000);
            LATA = 0;
            while(alarmTime >= 5 && PORTDbits.RD6){
                LATA = 255;
            }
        }
        else{
            LATA = 0;
            alarmTime = 0;
        }
        __delay32(1000000);
    }
    return 0;
}