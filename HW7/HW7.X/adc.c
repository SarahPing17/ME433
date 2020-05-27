#include "adc.h"

#define SAMPLE_TIME 10 // in core timer ticks, use a minimum of 250 ns

#include "ws2812b.h"

/* 
 * File:   main.c
 * Author: pingy
 *
 * Created on 2020?5?12?, ??10:44
 */
#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h> // for memset
#include <xc.h> // for the core timer delay
#include<sys/attribs.h>  // __ISR macro

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
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

    adc_setup();
    i2c_master_setup();
    ssd1306_setup();
    ws2812b_setup();
    ctmu_setup();
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    __builtin_enable_interrupts();


    char message1[100];
    char message2[100];
    char message3[100];

    
    while (1) {
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = !LATAbits.LATA4;
        //average ADC value
        int ADC_value1=0;
        int ADC_value2=0;
        int i,j;
        for(i=0;i<10;i++){
            ADC_value1 += ctmu_read(5, 50);
        }
        ADC_value1 = ADC_value1/10;
        for(j=0;j<10;j++){
            ADC_value2 += ctmu_read(4, 50);
        }
        ADC_value2 = ADC_value2/10;
        //calculate position
        float Delta_L,Delta_R;
        float L_Pos,R_Pos;
        float Pos=0.0;
        Delta_L = 1019.0 - ADC_value1;
        Delta_R = 1020.0 - ADC_value2;
        L_Pos = (Delta_L*100.0)/(Delta_L+Delta_R);
        R_Pos = ((1-Delta_R)*100.0)/(Delta_L+Delta_R);
        Pos = (L_Pos+R_Pos)/2.0;
        
        sprintf(message1, "V1: %d ", ADC_value1);
        drawMessage(0,0,message1);
        sprintf(message2, "V2: %d ", ADC_value2);
        drawMessage(0,8,message2);
        sprintf(message3, "V: %f ", Pos);
        drawMessage(0,16,message3);
        
        ssd1306_update();
        
        //set color
        wsColor c[4];        
        float br1=0.0;
        float br2=0;
        if(ADC_value1+ADC_value2>1900){
            c[0]=HSBtoRGB(1, 1, 0);
            c[1]=HSBtoRGB(70, 1, 0);
            c[2]=HSBtoRGB(130, 1, 0);
            c[3]=HSBtoRGB(220, 1, 0);
            ws2812b_setColor(&c, 4);
        }
        else{
                if(Pos>5){
                    br1 = 20.0/Pos;
                    br2 = 0.0;
                    c[0]=HSBtoRGB(10, 1, br1);
                    c[1]=HSBtoRGB(70, 1, 0);
                    c[2]=HSBtoRGB(130, 1, 0);
                    c[3]=HSBtoRGB(220, 1, 0);
                    ws2812b_setColor(&c, 4);
                }
                if(Pos<5&&Pos>-5){
                    br1 = 20.0/Pos;
                    br2 = 0.0;
                    c[0]=HSBtoRGB(10, 1, 0);
                    c[1]=HSBtoRGB(70, 1, br1);
                    c[2]=HSBtoRGB(130, 1, -br1);
                    c[3]=HSBtoRGB(220, 1, 0.1);
                    ws2812b_setColor(&c, 4);
                }
                if(Pos<-5){
                    br2 = -20.0/Pos;
                    br1 = 0.0;
                    c[0]=HSBtoRGB(10, 1, 0);
                    c[1]=HSBtoRGB(70, 1, 0);
                    c[2]=HSBtoRGB(130, 1, 0);
                    c[3]=HSBtoRGB(220, 1, br2);
                    ws2812b_setColor(&c, 4);
                }
            

        }
        
        while(_CP0_GET_COUNT() < 48000000 / 2 / 20){
            
        }
        
    }
}

unsigned int adc_sample_convert(int pin) {
    unsigned int elapsed = 0, finish_time = 0;
    AD1CHSbits.CH0SA = pin; // connect chosen pin to MUXA for sampling
    AD1CON1bits.SAMP = 1; // start sampling
    elapsed = _CP0_GET_COUNT();
    finish_time = elapsed + SAMPLE_TIME;
    while (_CP0_GET_COUNT() < finish_time) {
        ; // sample for more than 250 ns
    }
    AD1CON1bits.SAMP = 0; // stop sampling and start converting
    while (!AD1CON1bits.DONE) {
        ; // wait for the conversion process to finish
    }
    return ADC1BUF0; // read the buffer with the result
}

void adc_setup() {
    ANSELB = 1;// set analog pins with ANSEL

    AD1CON3bits.ADCS = 1; // ADC clock period is Tad = 2*(ADCS+1)*Tpb = 2*2*20.3ns = 83ns > 75ns
    IEC0bits.AD1IE = 0; // Disable ADC interrupts
    AD1CON1bits.ADON = 1; // turn on A/D converter
}

void ctmu_setup() {
    // base level current is about 0.55uA
    CTMUCONbits.IRNG = 0b11; // 100 times the base level current
    CTMUCONbits.ON = 1; // Turn on CTMU

    // 1ms delay to let it warm up
    _CP0_SET_COUNT(0);
    while (_CP0_GET_COUNT() < 48000000 / 2 / 1000) {
    }
}

int ctmu_read(int pin, int delay) {
    int start_time = 0;
    AD1CHSbits.CH0SA = pin;
    AD1CON1bits.SAMP = 1; // Manual sampling start
    CTMUCONbits.IDISSEN = 1; // Ground the pin
    // Wait 1 ms for grounding
    start_time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < start_time + 48000000 / 2 / 1000) {
    }
    CTMUCONbits.IDISSEN = 0; // End drain of circuit

    CTMUCONbits.EDG1STAT = 1; // Begin charging the circuit
    // wait delay core ticks
    start_time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < start_time + delay) {
    }
    AD1CON1bits.SAMP = 0; // Begin analog-to-digital conversion
    CTMUCONbits.EDG1STAT = 0; // Stop charging circuit
    while (!AD1CON1bits.DONE) // Wait for ADC conversion
    {}
    AD1CON1bits.DONE = 0; // ADC conversion done, clear flag
    return ADC1BUF0; // Get the value from the ADC
}

