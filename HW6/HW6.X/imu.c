#include "imu.h"

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

    imu_setup();
    ssd1306_setup();
    
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    __builtin_enable_interrupts();


    signed short imu[7];
    char message[200];
//    char message2[50];
//    char message3[50];

    
    while (1) {
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = !LATAbits.LATA4;
        
        
        imu_read(IMU_OUT_TEMP_L, imu, 7);
        
//        if(0) {
            sprintf(message, "g: %d %d %d ", imu[1], imu[2], imu[3]);
            drawMessage(0,0,message);
            sprintf(message, "a: %d %d %d ", imu[4], imu[5], imu[6]);
            drawMessage(0,8,message);
            sprintf(message, "t: %d ", imu[0]);
            drawMessage(0,16,message);
//        } else{
//            float bar;
//            bar_x(imu[5], 1);//-
////            bar_y(imu[4], 1);
////            sprintf(message, "b: %d ", bar);
////            drawMessage(64,16,message);
//        }
        
        ssd1306_update();
        
        while(_CP0_GET_COUNT() < 48000000 / 2 / 20){
            
        }
        
    }
}

void bar_x(signed short a, unsigned char color){
    ssd1306_drawPixel(64, 16, color);
    int i=0;
    float b=64.0/8000.0*a;
    for(i=0;i<b;i++){
        ssd1306_drawPixel(64+b, 16, color);
        ssd1306_update();
    }
//    return b;
}

void imu_setup(){
    unsigned char who = 0;
    i2c_master_setup();
    
    i2c_master_start();
    i2c_master_send(0b11010110);
    i2c_master_send(0x0F);
    i2c_master_restart();
    i2c_master_send(0b11010111);
    who = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    
    // read from IMU_WHOAMI
    
    if (who != 0b01101001){
        while(1){}
    }

    // init IMU_CTRL1_XL
    setPin(0b11010110, IMU_CTRL1_XL, 0b10000010);
    // init IMU_CTRL2_G
    setPin(0b11010110, IMU_CTRL2_G, 0b10001000);
    // init IMU_CTRL3_C
    setPin(0b11010110, IMU_CTRL3_C, 0b00000100);

}

void imu_read(unsigned char reg, signed short * data, int len){
    unsigned char data_i2c[200];
    I2C_read_multiple(0b11010110, reg, data_i2c , len*2);
    int i = 0;
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
//    for(i=0; i<len; i++){
//    data[i] = data[i]| data_i2c[i*2+1];
//    data[i] = data[i] << 8;
//    data[i] = data[i]| data_i2c[i*2];       
//    }
    int k=0;
        for (i=0; i<len; i++){
        data[i] = (data_i2c[k+1]<<8)|data_i2c[k];
        k=k+2;
    }

    // turn the chars into the shorts
}

void I2C_read_multiple(unsigned char address, unsigned char register1, unsigned char * data, int length){
        int i;
        i2c_master_start();
        i2c_master_send(address&0b11111110);//write 0b01000000
        i2c_master_send(register1);
        i2c_master_restart();
        i2c_master_send(address|0b00000001);//read 0b01000001
        for(i=0;i<length-1;i++){//13, length=7
            data[i] = i2c_master_recv();
            i2c_master_ack(0);
        }
        data[length-1] = i2c_master_recv();
        i2c_master_ack(1);
        i2c_master_stop();
}
