/*
 * File:   main.c
 * Author: local
 *
 * Created on April 10, 2024, 1:34 PM
 */
#pragma config POSCMOD = NONE           // Primary Oscillator Select (primary oscillator disabled)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC Oscillator without Postscaler)
#pragma config IESO = OFF               // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled)
// CONFIG1
#pragma config WDTPS = PS32768 // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128 // WDT Prescaler (1:128)
#pragma config WINDIS = ON // Watchdog Timer Window Mode disabled
#pragma config FWDTEN = OFF // Watchdog Timer disabled
#pragma config ICS = PGx2 // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF // Writes to program memory allowed
#pragma config GCP = OFF // Code protection is disabled
#pragma config JTAGEN = OFF // JTAG port is disabled

#include "xc.h"
#include <libpic30.h>
#include <stdio.h>
#include <stdlib.h>



// Defninicja makr tak by kod byl czytelny, przejrzysty, deskryptywny i przyjazny
// uzytkownikowi
#define FCY         4000000UL   // czestotliwosc robocza oscylatora jako polowa
                                //czestotliwosci (FNOSC = FRC -> FCY = 4000000)
// Zdefiniowanie poszczegolnych pinow jako odpowiednie makra
#define LCD_E       LATDbits.LATD4
#define LCD_RW      LATDbits.LATD5
#define LCD_RS      LATBbits.LATB15
#define LCD_DATA    LATE

// Przypisanie wartosci poszcegolnych komend do wlasciwych makr
#define LCD_CLEAR   0x01    //0b00000001
#define LCD_HOME    0x02    //0b00000010
#define LCD_ON      0x0C    //0b00001100
#define LCD_OFF     0x08    //0b00001000
#define LCD_CONFIG  0x38    //0b00111000
#define LCD_CURSOR      0x80    //0b10000000
#define LINE1           0x00
#define LINE2           0x40
#define LCD_CUST_CHAR   0x40    //0b01000000
#define LCD_SHIFT_R     0x1D    //0b00011100
#define LCD_SHIFT_L     0x1B    //0b00011000

// Definicja funkcji delay w us i ms - operujacych na jednostkach czasu zamiast
// cykli pracy oscylatora

void __delay_us(unsigned long us){
    __delay32(us*FCY/1000000);
}

void __delay_ms(unsigned long ms){
    __delay32(ms*FCY/1000);
}

// Definicja funkcji wysylajacych komendy (RS = 0) i dane (RS = 1) za pomoca
// magistrali rownoleglej (LCD_DATA). Znaki i komendy maja 8 bitow!

void LCD_sendCommand(unsigned char command){
    LCD_RW = 0;     // Zapis
    LCD_RS = 0;     // Przesylanie komend
    LCD_E = 1;      // Otwarcie transmisji danych
    LCD_DATA = command;
    __delay_us(50); // Opoznienie konieczne dla zapisania danych.
    LCD_E = 0;      // Konieczne zablokowanie transmisji po przeslaniu komunikatu.
}

void LCD_sendData(unsigned char data){
    LCD_RW = 0;
    LCD_RS = 1;     // Przesylanie danych
    LCD_E = 1;
    LCD_DATA = data;
    __delay_us(50);
    LCD_E = 0;
}

// Funkcja print wyswietlajaca kolejne 8-bitowe znaki w petli while - * oznacza
// przypisanie nie wartosci zmiennej lecz jej adresu.

void LCD_print(unsigned char* string){
    while(*string){
        LCD_sendData(*string++);
    }
}

// Funkcja ustawiaj?ca kursor w wybranym miejscu

void LCD_setCursor(unsigned char row, unsigned char col){
    unsigned char address;
    if (row == 1){
        address = LCD_CURSOR + LINE1 + col;
    }
    if (row == 2){
        address = LCD_CURSOR + LINE2 + col;
    }
    LCD_sendCommand(address);
}

// Funkcja ZAPISUJACA znak (zmienna array) do PAMIECI CGRAM na wybranym slocie
// od 0 do 7

void LCD_saveCustChar(unsigned char slot, unsigned char *array) {
    unsigned char i;
    LCD_sendCommand(LCD_CUST_CHAR + (slot*8));
    for(i=0;i<8;i++){
        LCD_sendData(array[i]);
    }
}

// Funkcja inicjalizujaca wyswietlacz LCD. Wysyla niezbedne komendy jak LCD_CONFIG
// i LCD_ON

void LCD_init(){
    __delay_ms(20);
    LCD_sendCommand(LCD_CONFIG);
    __delay_us(50);     // opoznienia wynikaja ze specyfikacji wyswietlacza i czasu
                        // przetwarzania poszczegolnych komend
    LCD_sendCommand(LCD_ON);
    __delay_us(50);
    LCD_sendCommand(LCD_CLEAR);
    __delay_ms(2);
}

int append(int i, int n){
    
    return (i+1)%n;
}

int main(void) {
    
    unsigned portValue = 0x0001;
    char current6 = 0, prev6 = 0, current7 = 0, prev7 = 0, micPower = 0,current8 = 0, prev8 = 0, current9 = 0, prev9 = 0; //variables for buttons
    int i = 0, start = 0, g1mins = 0, g1secs = 0, g2mins = 0, g2secs = 0, g1Timer = 120, g2Timer =120, player = 0, timeMode = 0, startTime = 0;
    char g1minsTxt[5], g1secsTxt[5], g2minsTxt[5], g2secsTxt[5];
    
    
    TRISB = 0x7FFF;     // Ustawienie rejestrow kierunku
    TRISE = 0x0000;
    TRISA = 0x0080;     // port set to output
    TRISD = 0xFFE7;     // port set to input
    
    

    LCD_init();                     // Inicjalizacja wyswietlacza
    LCD_setCursor(1,0);             // Ustawienie kursora na poczatku drugiej linii
                                    // Wyswietlenie napisu
                                    // Wyswietlenie znaku ze slotu 0 w pamieci CGRAM
    __delay_ms(500);
    
    
    while(1){
        startTime = 1*30*(timeMode+1);
        
        g1mins = (g1Timer-(g1Timer%60))/60;
        g1secs = g1Timer%60;
        g2mins = (g2Timer-(g2Timer%60))/60;
        g2secs = g2Timer%60;
        
        sprintf(g1minsTxt, "%d", g1mins);
        sprintf(g1secsTxt, "%d ", g1secs);
        sprintf(g2minsTxt, "%d", g2mins);
        sprintf(g2secsTxt, "%d ", g2secs);
        

        

        LCD_setCursor(1,1);
        if(player == 0)LCD_print("*Gracz1: ");
        else LCD_print("Gracz1: ");
        if (g1mins < 10)LCD_print("0");
        LCD_print((unsigned char*)g1minsTxt);
        LCD_print(":");
        if (g1secs < 10)LCD_print("0");
        LCD_print((unsigned char*)g1secsTxt);
        LCD_setCursor(2,1);
        if(player == 1)LCD_print("*Gracz2: ");
        else LCD_print("Gracz2: ");
        if (g2mins < 10)LCD_print("0");
        LCD_print((unsigned char*)g2minsTxt);
        LCD_print(":");
        if (g2secs < 10)LCD_print("0");
        LCD_print((unsigned char*)g2secsTxt);
        
        
        
        
        prev6 = PORTDbits.RD6;      //scanning for a change of buttons' state
        prev7 = PORTDbits.RD7;
        prev8 = PORTAbits.RA7;
        prev9 = PORTDbits.RD13;
        
        __delay32(150000);
        current6 = PORTDbits.RD6;
        current7 = PORTDbits.RD7;
        current8 = PORTAbits.RA7;
        current9 = PORTDbits.RD13;
        
        if(start == 1){
        if(current9 - prev9 == 1) start = 0;
        if(g1Timer == 0) start = 0;
        else g1Timer--;
        __delay_ms(1000);
            
        }
        if (current7 - prev7 == 1)// time mode change
        {
            
        }
        
        if (current8 - prev8 == 1) //reset to current game time
        {
            g1Timer = 120;
            g2Timer = 120;
        }
        
        if (current9 - prev9 == 1) //change player
        {
          player++;
          if(player > 1)player = 0;
        }
        
        

    }
    
    
    
    
    LCD_sendCommand(LCD_SHIFT_R);    // Przesuniecie calej zawartosci o jedno miejsce w prawo
    return 0;
}
