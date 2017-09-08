//***********************************************************************
//                                      Myson Addition DDCRAM 
//***********************************************************************
typedef unsigned char Byte;

#define _DDC_
#include "Header\DDC.h"
#include "Header\MTV512.H"
#include "Header\access.h"
#include "Header\Timer.h"
#include "reg52.h"

#if(USE_MCU_DDC)

Byte xdata EDID_Cmd_Buffer;

void EnableDDC(void);
//void EnableDDCWriter();

void EDID_Process();
//void MovEEPROM(Byte *array);
//void MovEEPROM_2(Byte *array);

Byte code EDID_DATA1[128] = 
{

  0x00,0xff,0xff,0xff,0xFF,0xFF,0xFF,0x00,0x5A,0x63,0x02,0x08,0x01,0x01,0x01,0x01,
  0x29,0x0A,0x01,0x02,0x1D,0x1F,0x17,0xB9,0xEB,0x00,0xB8,0xA0,0x57,0x49,0x9B,0x26,
  0x10,0x48,0x4C,0xFF,0xFE,0x80,0x31,0x59,0x45,0x59,0x71,0x4F,0x81,0x40,0x81,0x80,
  0x01,0x01,0x01,0x01,0x01,0x01,0xC3,0x1E,0x00,0x20,0x41,0x00,0x20,0x30,0x10,0x60,
  0x13,0x00,0x2C,0xE1,0x10,0x00,0x00,0x1E,0x00,0x00,0x00,0xFF,0x00,0x32,0x30,0x48,
  0x30,0x30,0x34,0x31,0x30,0x30,0x32,0x36,0x30,0x0A,0x00,0x00,0x00,0xFD,0x00,0x32,
  0xB4,0x1E,0x46,0x0A,0x01,0x0A,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xFC,
  0x00,0x52,0x45,0x41,0x4C,0x54,0x45,0x4B,0x0A,0x20,0x20,0x20,0x20,0x20,0x00,0x01
  //     R   E     A    L    T   E     K
  
};



void EDID_Process()
{ 
  Byte i,checksum=0;

  
  if(EDID_Cmd_Buffer==0xaa)
  {
     for(i=0;i<128;i++)
     	  checksum=checksum+byMTV512_DDCRAMA[i];
     if(checksum==0) // checksum correct
     {
        if((byMTV512_DDCRAMA[20]&0x80)==0x80)
            MovEEPROM(&byMTV512_DDCRAMA,1); //DVI SAVE TO LAST 128 BYTE
        else
            MovEEPROM(&byMTV512_DDCRAMA,0); //SUB SAVE TO LAST 128 BYTE
            EDID_Cmd_Buffer=0xff;    //Initial Command Buffer          
     }       
  }    

}
void EnableDDC(void)
{
   EDID_Cmd_Buffer = 0xff;     //Initial Command Buffer
   M512_IIC_CTR = 0xc0;       //Enable IIC Interface and define ddc2 active at HSDA/HSCL
   M512_IIC_INTEN = 0x04;    //Detect WslvA1 IIC Stop Condition
   M512_INTFLG = 0x00;        //Clear IIC Interrupt register
   byM512_CTRSLVB = 0x00;     //Define IIC Protocal Slave Address Bit
   M512_PadMode2=0xa0;      //OPEN DDCRAM1 and DDCRAM2 Chanell(HSDA1/HSDA2  HSCL1/HSCL2) 
   byM512_DDCCTRA1=0xd0;    //Enable DDC1 and DDCRAM 128 Access
   byM512_SLVA1ADR=0x80|(0xa0>>1);  //DDC Slave A1 address
   byM512_DDCCTRA2=0xd0;    //Enable DDC1 and DDCRAM 128 Access
   byM512_SLVA2ADR=0x80|(0xa0>>1);  //DDC Slave A2 address
   
}
//*******************************************************************************************
void ReceiveEDIDINT1(void) interrupt 2
{
   Byte tempflag;  
   tempflag = M512_INTFLG;
   EX1 = 0; 
   
   if((tempflag & WslvA1I) != 0)  //DDCRAMA IIC Stop Interrupt detect
   {
       M512_INTFLG = tempflag & (~WslvA1I);  //Clear DDCRAMA IIC Stop Interrupt detect 
       EDID_Cmd_Buffer = 0xaa;   //Set the Writer EEPROM Command
   }  
 
   EX1 =1;

}

//*******************************************************************************************
void MovEEPROM(Byte *array,unsigned char index)   //128 byte
{
    Byte a,b;

    for (a=0;a<8;a++)
	{
	  if(index == 0)
        I2CSendAddr(ADDR_EDID1, a*16, 0);     
	  else
        I2CSendAddr(ADDR_EDID1, (0x80)+(a*16), 0);     

	  for (b=0;b<16;b++)
 		 I2CSendByte(array[(a*16)+b]);  //DDCRAM Write to EEPROM

      I2CSendStop(); 
	  Delay_Xms(30);
	}
}
//*******************************************************************************************
/*
void MovEEPROM_2(Byte *array)   //128 byte
{
    Byte a,b;

    for (a=0;a<8;a++)
	{
      I2CSendAddr(ADDR_EDID1, (0x80)+(a*16), 0);     

	  for (b=0;b<16;b++)
 		 I2CSendByte(array[(a*16)+b]);  //DDCRAM Write to EEPROM

      I2CSendStop(); 
	  Delay_Xms(30);
	}
}
*/
#endif//#if(USE_MCU_DDC)
