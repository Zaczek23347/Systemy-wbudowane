/*
 * File:   main.c
 * Author: local
 *
 * Created on 3 kwietnia 2024, 16:52
 */

// CONFIG2
#pragma config POSCMOD = NONE           // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRC              // Oscillator Select (Primary Oscillator with PLL module (HSPLL, ECPLL))
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
#include <libpic30.h>
#include <math.h>
#define BIT_VALUE(value,noBit) (value >> noBit) & 1

unsigned portValue = 0, bcdValue = 0, snakeMove = 0, queueMove = 0, queueBuffor = 0, tens = 0, ones = 0, queueEnd = 0, iq = 7, counter = 0, BIT = 0;
char prevS6 = 6, prevS7 = 7, currentS6 = 0, currentS7, program = 0;
int val = 1, xor = 0;

int sprzezenie(unsigned int v){
    //sprzezenie w oparciu o b1110011
    return BIT_VALUE(v,0)^BIT_VALUE(v,1)^BIT_VALUE(v,2)^BIT_VALUE(v,5)^BIT_VALUE(v,6);
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    
    // sprawdz ktory podprogram ma sie uruchomic
    if (program == 0){ // licznik binarny od 0 do 255
        portValue++;
        LATA = portValue;
    }
    if (program == 1){ // licznik binarny od 255 do 0
        portValue--;
        LATA = portValue;
    }
    if (program == 2){ // licznik Graya od 0 do 255
        portValue++;
        LATA = (portValue >> 1) ^ portValue;
    }
    if (program == 3){ // licznik Graya od 255 do 0
        portValue--;
        LATA = (portValue >> 1) ^ portValue;
    }
    if (program == 4){ // licznik BCD od 0 do 99
        bcdValue++;
        ones = bcdValue%10;
        tens = (bcdValue-ones)/10;
        LATA = (tens*pow(2,4)+ones);
    }
    if (program == 5){ // licznik BCD od 99 do 0
        bcdValue--;
        ones = bcdValue%10;
        tens = (bcdValue-ones)/10;
        LATA = (tens*pow(2,4)+ones);
    }
    if (program == 6){ // wezyk poruszajacy sie lewo-prawo
        while(snakeMove < 5){
            snakeMove++;
            LATA = 7 * pow(2,snakeMove);
            __delay32(1500000);
            
            prevS6 = PORTDbits.RD6;
            prevS7 = PORTDbits.RD7;
            __delay32(15000);
            currentS6 = PORTDbits.RD6;
            currentS7 = PORTDbits.RD7;
            
            if(currentS6-prevS6 == -1){
            break;
            }
            if(currentS7-prevS7 == -1){
            break;
            }

        }
        while(snakeMove > 0){
            snakeMove--;
            LATA = 7 * pow(2,snakeMove);
            __delay32(1500000);
            
            prevS6 = PORTDbits.RD6;
            prevS7 = PORTDbits.RD7;
            __delay32(15000);
            currentS6 = PORTDbits.RD6;
            currentS7 = PORTDbits.RD7;
            
            if(currentS6-prevS6 == -1){
            break;
            }
            if(currentS7-prevS7 == -1){
            break;
        }
        }
        __delay32(150000);
    } 
    if (program == 7){ // kolejka
        queueMove = 1*pow(2,queueBuffor);
        if(queueBuffor == iq){
            queueEnd = 255-(pow(2,iq)-1);
            iq--;
            queueMove = 0;
            queueBuffor = 0;
            LATA = queueEnd;
        }
        else{
        LATA = queueEnd+queueMove;
        queueBuffor++;
        }  
    }
    if (program == 8){ // generator liczb pseudolosowych w oparciu b1110011
        for(int i=0;i<5;i++){
            xor = sprzezenie(val);
            val = val>>1;
            val += xor<<7;
            LATA = val;
        }
        

    }
    _T1IF = 0;
}

int main(void) {
    
    TRISA = 0x0000;
    TRISD = 0xFFFF;
    T1CON = 0x8030;
    _T1IE = 1;
    _T1IP = 1;
    PR1 = 0x0FFF;
    while(1){
        // przemieszczanie si? po programach za pomoc? przycisk�w
        prevS6 = PORTDbits.RD6;
        prevS7 = PORTDbits.RD7;
        __delay32(15000);
        currentS6 = PORTDbits.RD6;
        currentS7 = PORTDbits.RD7;
        
        if(currentS6-prevS6 == -1){
            program--;
        }
        if(currentS7-prevS7 == -1){
            program++;
        }
        if(program > 8){
            program = 0;
        }
        if(program < 0){
            program = 8;
        }
        if(bcdValue > 99) bcdValue = 1;
        if(bcdValue == 0) bcdValue = 99;
        if(queueBuffor > 8) queueBuffor = 0;
        if(iq == 0) iq = 7,queueBuffor = 0, queueEnd = 0;
    }
        
    return 0;
}
