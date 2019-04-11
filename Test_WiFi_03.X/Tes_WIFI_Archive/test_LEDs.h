#ifndef TEST_LEDS_H
#define	TEST_LEDS_H

#define LED1DIR TRISBbits.TRISB8
#define LED2DIR TRISBbits.TRISB9
#define LED3DIR TRISAbits.TRISA7

#define LED1SET LATBbits.LATB8
#define LED2SET LATBbits.LATB9
#define LED3SET LATAbits.LATA7

#define LED1READ PORTBbits.RB8 // Wifi Light
#define LED2READ PORTBbits.RB9 // SPI Light
#define LED3READ PORTAbits.RA7 // UART Light


#endif	/* XC_HEADER_TEMPLATE_H */

