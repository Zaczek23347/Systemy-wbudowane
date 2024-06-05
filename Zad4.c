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

int append(int i){
    
    return (i+1)%3;
}



// Zdefiniowanie znaku niestandardowego w postaci tablicy 8x5 (8 linii po 5 kropek)

unsigned char trefl[8] = {
    0b00000,
    0b00000,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b00100,
    0b00100
};

unsigned char pik[8] = {
    0b00000,
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b11111,
    0b10101,
    0b00100
};

unsigned char karo[8] = {
    0b00000,
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
};

unsigned char kier[8] = {
    0b00000,
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100
};

int main(void) {
    
    unsigned portValue = 0x0001;
    char current6 = 0, prev6 = 0, current7 = 0, prev7 = 0; //variables for buttons
    
    TRISB = 0x7FFF;     // Ustawienie rejestrow kierunku
    TRISE = 0x0000;
    TRISA = 0x0080;     // port set to output
    TRISD = 0xFFE7;     // port set to input
    
    

    LCD_init();                     // Inicjalizacja wyswietlacza
    LCD_saveCustChar(0, trefl);
    LCD_saveCustChar(1, pik);
    LCD_saveCustChar(2, karo);
    LCD_saveCustChar(3, kier);
    LCD_setCursor(1,5);             // Ustawienie kursora na poczatku drugiej linii
    LCD_print("Vegas");             // Zapisanie znaku 'symbol1' do pamieci CGRAM
    LCD_setCursor(2,5);             // Ustawienie kursora na poczatku drugiej linii
    LCD_print("Kasyno");            // Wyswietlenie napisu
                                    // Wyswietlenie znaku ze slotu 0 w pamieci CGRAM
    __delay_ms(500);
    
    int i = 0;
    
    while(1){
        LATA = portValue;
        prev6 = PORTDbits.RD6;      //scanning for a change of buttons' state
        prev7 = PORTDbits.RD7;
        __delay32(150000);
        current6 = PORTDbits.RD6;
        current7 = PORTDbits.RD7;

        if (current6 - prev6 == 1) //button up
        {
            portValue++;
        }

        if (current7 - prev7 == 1)  //button down
        {
            portValue--;
        }
        
        
    }
    
    
    
    
    LCD_sendCommand(LCD_SHIFT_R);    // Przesuniecie calej zawartosci o jedno miejsce w prawo
    return 0;
}
