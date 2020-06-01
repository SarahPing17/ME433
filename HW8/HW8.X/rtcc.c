#include "rtcc.h"

#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h> // for memset
#include <xc.h> // for the core timer delay
#include<sys/attribs.h>  // __ISR macro
char DAYOFTHEWEEK[7][11] = { "Sunday.", "Monday.", "Tuesday.", "Wednesday.", "Thursday.", "Friday.", "Saturday."};
// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = ON // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

int main(){
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here


    i2c_master_setup();
    ssd1306_setup();

    
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5 = 1;
    
    __builtin_enable_interrupts();


    char message1[100];
    char message2[100];
    char message3[100];
    char DAY[30];
    rtcc_setup(0x00510100, 0x20060101);
    int i = 0;
    
    while (1) {
        _CP0_SET_COUNT(0);
        LATBbits.LATB5 = !LATBbits.LATB5;
        
        
        rtccTime TIME;
        TIME = readRTCC();
        
        sprintf(message1, "Hi! %d ", i);
        drawMessage(0,0,message1);
        sprintf(message2, "%d%d:%d%d:%d%d", TIME.hr10,TIME.hr01,TIME.min10,TIME.min01,TIME.sec10,TIME.sec01);
        drawMessage(0,8,message2);
        dayOfTheWeek(TIME.wk, DAY);
        sprintf(message3, "%s, %d%d/%d%d/20%d%d", DAY,TIME.mn10,TIME.mn01,TIME.dy10,TIME.dy01,TIME.yr10,TIME.yr01);
        drawMessage(0,16,message3);
        
        ssd1306_update();
        

        i++;
        while(_CP0_GET_COUNT() < 48000000 / 4){//4 0.5s
            
        }
        
    }
}



void dayOfTheWeek(unsigned char position, char* day){
    // given the number of the day of the week, return the word in a char array
    int i = 0;
    while(DAYOFTHEWEEK[position][i]!='.'){
        day[i] = DAYOFTHEWEEK[position][i];
        i++;
    }
    day[i] = '\0';
}

void rtcc_setup(unsigned long time, unsigned long date) {
    OSCCONbits.SOSCEN = 1; // turn on secondary clock
    while (!OSCCONbits.SOSCRDY) {
    } // wait for the clock to stabilize, touch the crystal if you get stuck here

    // unlock sequence to change the RTCC settings
    SYSKEY = 0x0; // force lock, try without first
    SYSKEY = 0xaa996655; // write first unlock key to SYSKEY
    SYSKEY = 0x556699aa; // write second unlock key to SYSKEY
    // RTCWREN is protected, unlock the processor to change it
    RTCCONbits.RTCWREN = 1; // RTCC bits are not locked, can be changed by the user

    RTCCONbits.ON = 0; // turn off the clock
    while (RTCCONbits.RTCCLKON) {
    } // wait for clock to be turned off

    RTCTIME = time; // safe to update the time
    RTCDATE = date; // update the date

    RTCCONbits.ON = 1; // turn on the RTCC

    while (!RTCCONbits.RTCCLKON); // wait for clock to start running, , touch the crystal if you get stuck here
    LATBbits.LATB5 = 0;
}

rtccTime readRTCC() {
    rtccTime time;
    // copy over the BCD 
    // ...
    time.hr10 = RTCTIMEbits.HR10;
    time.hr01 = RTCTIMEbits.HR01;
    time.min10 = RTCTIMEbits.MIN10;
    time.min01 = RTCTIMEbits.MIN01;
    time.sec10 = RTCTIMEbits.SEC10;
    time.sec01 = RTCTIMEbits.SEC01;
    time.yr10 = RTCDATEbits.YEAR10;
    time.yr01 = RTCDATEbits.YEAR01;
    time.mn10 = RTCDATEbits.MONTH10;
    time.mn01 = RTCDATEbits.MONTH01;
    time.dy10 = RTCDATEbits.DAY10;
    time.dy01 = RTCDATEbits.DAY01;
    time.wk = RTCDATEbits.WDAY01;

    return time;
}

