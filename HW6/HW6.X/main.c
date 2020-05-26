/* 
// * File:   main.c
// * Author: pingy
// *
// * Created on 2020?5?12?, ??10:44
// */
//#include "imu.h"
//#include "i2c_master_noint.h"
//#include "ssd1306.h"
//#include "font.h"
//#include <stdio.h>
//#include <stdlib.h>
//
//#include <string.h> // for memset
//#include <xc.h> // for the core timer delay
//#include<sys/attribs.h>  // __ISR macro
//
//// DEVCFG0
//#pragma config DEBUG = OFF // disable debugging
//#pragma config JTAGEN = OFF // disable jtag
//#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
//#pragma config PWP = OFF // disable flash write protect
//#pragma config BWP = OFF // disable boot write protect
//#pragma config CP = OFF // disable code protect
//
//// DEVCFG1
//#pragma config FNOSC = PRIPLL // use primary oscillator with pll
//#pragma config FSOSCEN = OFF // disable secondary oscillator
//#pragma config IESO = OFF // disable switching clocks
//#pragma config POSCMOD = HS // high speed crystal mode
//#pragma config OSCIOFNC = OFF // disable clock output
//#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
//#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
//#pragma config WDTPS = PS1048576 // use largest wdt
//#pragma config WINDIS = OFF // use non-window mode wdt
//#pragma config FWDTEN = OFF // wdt disabled
//#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%
//
//// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
//#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
//#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
//#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
//
//// DEVCFG3
//#pragma config USERID = 0 // some 16bit userid, doesn't matter what
//#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
//#pragma config IOL1WAY = OFF // allow multiple reconfigurations
//
//int main(){
//    __builtin_disable_interrupts(); // disable interrupts while initializing things
//
//    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
//    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
//
//    // 0 data RAM access wait states
//    BMXCONbits.BMXWSDRM = 0x0;
//
//    // enable multi vector interrupts
//    INTCONbits.MVEC = 0x1;
//
//    // disable JTAG to get pins back
//    DDPCONbits.JTAGEN = 0;
//
//    // do your TRIS and LAT commands here
//
//    
//    i2c_master_setup();
//    ssd1306_setup();
//    
//    TRISAbits.TRISA4 = 0;
//    LATAbits.LATA4 = 1;
//    
//    __builtin_enable_interrupts();
//
//
//    
//    while (1) {
//   
//            //unsigned char c = 'a';
//        char message1[50];
//        char message2[50];
//        int i1=521;
//        double i2;
//        unsigned int time;
//        
//            sprintf(message1, "My var = %d", i1);
//            _CP0_SET_COUNT(0);
//            drawMessage(10,5,message1);
//            time = _CP0_GET_COUNT();
//            i2 = 24000000.0/time;
//            sprintf(message2, "fps = %f", i2);
//            drawMessage(19,19,message2);
//            
//            LATAbits.LATA4 = 0;
//            _CP0_SET_COUNT(0);
//            while (_CP0_GET_COUNT() < 48000000 / 20 ){// 1hz wait 0.01s  
//            ;
//            }
//            
//            LATAbits.LATA4 = 1; 
//            _CP0_SET_COUNT(0);
//            while (_CP0_GET_COUNT() < 48000000 / 20 ){// 1hz wait 0.01s
//            ;
//            }
//        }
//}


