// based on adafruit and sparkfun libraries

#include <string.h> // for memset
#include <stdio.h>
#include <xc.h> // for the core timer delay
#include "ssd1306.h"
#include "font.h"

#include "i2c_master_noint.h"
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

unsigned char ssd1306_write = 0b01111000; // i2c address
unsigned char ssd1306_read = 0b01111001; // i2c address
unsigned char ssd1306_buffer[512]; // 128x32/8. Every bit is a pixel

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
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    __builtin_enable_interrupts();


    
    while (1) {
   
            //unsigned char c = 'a';
        char message1[50];
        char message2[50];
        int i1=521;
        double i2;
        unsigned int time;
        
            sprintf(message1, "My var = %d", i1);
            _CP0_SET_COUNT(0);
            drawMessage(10,5,message1);
            time = _CP0_GET_COUNT();
            i2 = 24000000.0/time;
            sprintf(message2, "fps = %f", i2);
            drawMessage(19,19,message2);
            
            LATAbits.LATA4 = 0;
            _CP0_SET_COUNT(0);
            while (_CP0_GET_COUNT() < 48000000 / 20 ){// 1hz wait 0.01s  
            ;
            }
            
            LATAbits.LATA4 = 1; 
            _CP0_SET_COUNT(0);
            while (_CP0_GET_COUNT() < 48000000 / 20 ){// 1hz wait 0.01s
            ;
            }
        }
}

void drawletter(int a, int b, unsigned char var){
    int i,j;
    for (i=0; i<5; i++){
        for (j=0; j<8; j++){
        ssd1306_drawPixel(a+i,b+j,ASCII[var-0x20][i]>>j&1);
        //ssd1306_update();            
        }
    }
    ssd1306_update();
}

void drawMessage(int x, int y,unsigned char *m ){
            int s=0 ;
        //drawMessage(10,10,message);
            //drawletter(0,0,'1');
            while(m[s]!=0){
                drawletter(x + (5*s),y ,m[s]);
                s++;
            }
}


void initI2C(){
    i2c_master_setup();
    
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(0x00);
    i2c_master_send(0x00);
    i2c_master_stop();
    
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(0x01);
    i2c_master_send(0xFF);
    i2c_master_stop();
}

void setPin(unsigned char address, unsigned char register1, unsigned char value){
        i2c_master_start();
        i2c_master_send(address);
        i2c_master_send(register1);
        i2c_master_send(value);
        i2c_master_stop();
}

unsigned char readPin(unsigned char address, unsigned char register2){
        unsigned char Recv;
        i2c_master_start();
        i2c_master_send(address&0b11111110);//write 0b01000000
        i2c_master_send(register2);
        i2c_master_restart();
        i2c_master_send(address|0b00000001);//read 0b01000001
        Recv = i2c_master_recv();
        i2c_master_ack(1);
        i2c_master_stop();
        return Recv;
}



void ssd1306_setup() {
    // give a little delay for the ssd1306 to power up
    _CP0_SET_COUNT(0);
    while (_CP0_GET_COUNT() < 48000000 / 2 / 10) {
    }
    ssd1306_command(SSD1306_DISPLAYOFF);
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_command(0x80);
    ssd1306_command(SSD1306_SETMULTIPLEX);
    ssd1306_command(0x1F); // height-1 = 31
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_command(0x0);
    ssd1306_command(SSD1306_SETSTARTLINE);
    ssd1306_command(SSD1306_CHARGEPUMP);
    ssd1306_command(0x14);
    ssd1306_command(SSD1306_MEMORYMODE);
    ssd1306_command(0x00);
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYON);
    ssd1306_clear();
    ssd1306_update();
}

// send a command instruction (not pixel data)
void ssd1306_command(unsigned char c) {
    i2c_master_start();
    i2c_master_send(ssd1306_write);
    i2c_master_send(0x00); // bit 7 is 0 for Co bit (data bytes only), bit 6 is 0 for DC (data is a command))
    i2c_master_send(c);
    i2c_master_stop();
}

// update every pixel on the screen
void ssd1306_update() {
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);
    ssd1306_command(0xFF);
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);
    ssd1306_command(128 - 1); // Width

    unsigned short count = 512; // WIDTH * ((HEIGHT + 7) / 8)
    unsigned char * ptr = ssd1306_buffer; // first address of the pixel buffer
    i2c_master_start();
    i2c_master_send(ssd1306_write);
    i2c_master_send(0x40); // send pixel data
    // send every pixel
    while (count--) {
        i2c_master_send(*ptr++);
    }
    i2c_master_stop();
}

// set a pixel value. Call update() to push to the display)
void ssd1306_drawPixel(unsigned char x, unsigned char y, unsigned char color) {
    if ((x < 0) || (x >= 128) || (y < 0) || (y >= 32)) {
        return;
    }

    if (color == 1) {
        ssd1306_buffer[x + (y / 8)*128] |= (1 << (y & 7));
    } else {
        ssd1306_buffer[x + (y / 8)*128] &= ~(1 << (y & 7));
    }
}

// zero every pixel value
void ssd1306_clear() {
    memset(ssd1306_buffer, 0, 512); // make every bit a 0, memset in string.h
}


