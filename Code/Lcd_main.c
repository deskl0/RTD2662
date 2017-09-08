//echo note :
//modify last date : 2004.07.16


#define __MAIN__

#include "reg52.h"
#include "intrins.h"

#include "Header\MAIN_DEF.H"
#include "Header\CONFIG.H"
#include "Header\ACCESS.H"
#include "Header\LCD_COEF.H"
#include "Header\LCD_FUNC.H"
#include "Header\LCD_AUTO.H"
#include "Header\LCD_MSG.H"
#include "Header\LCD_MAIN.H"
#include "Header\LCD_OSD.H"
#include "Header\FONT.H"
#include "Header\OSD.H"
#include "Header\INITIAL.H"
#include "Header\FRAME_SYNC.H"
#include "Header\SRC_CTRL.H"
#include "Header\DDC.H"

#include "Header\uart.h"
#include "Header\DevOS.h"
#include "Header\Ksv.h"

#include "Header\rgb_echo.H"
#include "Header\MTV512.h"

#if(BURNIN_MODE)
void BurnIn()
{

  static unsigned char Color = 1;

  static unsigned char ColorCounter = 100;
  
  unsigned char ucTemp;

       if (ColorCounter)      
	      ColorCounter   -= 1;
       else
       {  

           Color += 1;
		   if(Color == 17)
	       	Color = 1;


		   ucTemp = 0x0d | (Color << 4);

//		   if (PANEL_ON == bPANEL_PWR)
   		   if ( _ON == bPanel_Status)
	       {
              Free_Background();
              Wait_For_Event(EVENT_IEN_STOP);     // Wait for Frame End

              RTDSetBit(OVL_CTRL_6D,0x3f,0x00); //Red
              RTDSetByte(BGCOLOR_CONTROL_6C,BurnInColor[0][Color]);

		RTDSetBit(OVL_CTRL_6D,0x3f,0x40); //Green
              RTDSetByte(BGCOLOR_CONTROL_6C,BurnInColor[1][Color]);

		RTDSetBit(OVL_CTRL_6D,0x3f,0x80); //Blue
              RTDSetByte(BGCOLOR_CONTROL_6C,BurnInColor[2][Color]);

		RTDSetBit(VDIS_CTRL_20,0x5f,0x20 | DHS_MASK);


		   }
           ColorCounter = 255;
		              
       }	    
}
#endif

void Set_Task(unsigned char state)
{
    switch (state)
    {
    case STATE_POWERUP :
    case STATE_SOURCECHANGE :

        ucOSD_Page_Index    = 0;
        ucOSD_Item_Index0   = 0;
        ucOSD_Item_Index1   = 0;
        ucOSD_Item_Index2   = 0;
        b_rgb_VOLUME_STATUS = 0;

        usTaskCnt   = BEGIN_SOURCECHANGE;
        break;

    case STATE_MODECHANGE :

#if (RTDDEBUG == 0)
        ucOSD_Page_Index    = 0;
        ucOSD_Item_Index0   = 0;
        ucOSD_Item_Index1   = 0;
        ucOSD_Item_Index2   = 0;
        b_rgb_VOLUME_STATUS = 0;
#endif

        if (BEGIN_MODECHANGE > usTaskCnt)   
        {
            usTaskCnt   = BEGIN_MODECHANGE;
        }
        break;
    case STATE_POWERDOWN :
        // In this code, we do nothing
        break;
    }
}

void Run_Task(void)
{
    // Task of each state :
    // STATE0. (BEGIN_SOURCECHANGE > usTaskCnt >= BEGIN_MODECHANGE) 
    //  - Show input source note text. Clear note text and go to State1 after 3sec or OSD window triggered.
    // STATE1. (BEGIN_MODECHANGE > usTaskCnt >= BEGIN_SHOWHINT)
    //  - Wait for input signal being stable for 2 sec. When input signal changed, 
    //    Input Mode Detector will set task state to 1 if current task state is 2. After 2sec, hint text
    //    will be shown if input isn't valid and no OSD window on screen and go to State2, else stay in STATE1.
    // STATE2. (BEGIN_SHOWHINT > usTaskCnt >= 0)
    //  - If OSD window is on screen, go to State1. Else if input isn't valid, show hint text for 5sec and 
    //    then turn off panel.

    if (usTaskCnt)      usTaskCnt   -= 1;


    if (BEGIN_MODECHANGE <= usTaskCnt)
    {
        // STATE0 : (BEGIN_SOURCECHANGE > usTaskCnt >= BEGIN_MODECHANGE)

        if (ucOSD_Page_Index)         
        {
            usTaskCnt   = BEGIN_MODECHANGE;
        }
    }
    else if (BEGIN_SHOWHINT <= usTaskCnt)
    {
        // STATE1 : (BEGIN_MODECHANGE > usTaskCnt >= BEGIN_SHOWHINT)

        if (BEGIN_SHOWHINT == usTaskCnt)
        {
            // Stay in STATE1 if input signal is valid or OSD window is on screen
            if (ucOSD_Page_Index || (MODE_NOSUPPORT != ucMode_Curr && MODE_NOSIGNAL != ucMode_Curr && !bOverSpec))
            {
                usTaskCnt   = BEGIN_SHOWHINT + OSD_TO_HINT_DELAY;
            }
        }
    }
    else
    {
        // Task State 2 : (BEGIN_SHOWHINT > usTaskCnt >= 0)

        if (ucOSD_Page_Index)
        {
            usTaskCnt   = BEGIN_SHOWHINT + OSD_TO_HINT_DELAY;    // Go back to Task State 1
        }
        else if (0 == usTaskCnt)
        {
            usTaskCnt   = BEGIN_SHOWHINT - 1;   // Stay in Task State 2
        }
    }
}


void ISP_Check_Sum(void)
{

#if(ISPACK)
    RTDSetByte(DDC_SET_SLAVE_F0,0x6a);
#endif


}

#if(USE_MCU_DDC)
void MCU_DDC_Process(void)
{
    unsigned char i,j;

	MCU_Init();
	
    for(j=0;j<8;j++)
    {
         I2CRead(ADDR_EDID1, j*16, 16);
         for( i = 0; i < 16; i++)
 	        byMTV512_DDCRAMA[i+(j*16)] = Data[i];  // prepare DDC_RAM_1

    } 
    Delay_Xms(250);
    Delay_Xms(250);
    Delay_Xms(250);
    Delay_Xms(250);
    Delay_Xms(250);

    MCU_Init();
    Firmware_Init();
    EDID_Process();
    EnableDDC(); 
}

#endif

void Signal_Stable(unsigned char Notify)
{
 // When input format changed, OSD will be cleared. OSD_Proc() can only runs when input signal is stable.
            if (bStable)
            {
                OSD_Dispatcher(Notify);

			
                if (MODE_OSDFORCE == ucMode_Curr)   // OSD force to reset
                {
                    Reset_Mode();
                    Set_Task(STATE_MODECHANGE);     // Notify Task State Machine
                }
                else
                {
                    if (BEGIN_SHOWHINT <= usTaskCnt)
                    {


//                        if (PANEL_OFF == bPANEL_PWR)
                        if( _OFF == bPanel_Status)
                        {
                            Set_Panel(1);   // Turn on panel
                        }
                        
					    //RTDCodeW(OSD_Enable);
				        //Set_Bright_Contrast();
						if(((ucMode_Curr == MODE_NOSUPPORT)||(ucMode_Curr == MODE_NOSIGNAL)))
                             RTDSetBit(VDIS_CTRL_20, 0x5f, 0x20 | DHS_MASK);        // Normal display
						else
						     RTDSetBit(VDIS_CTRL_20, 0x5f, DHS_MASK);        // Normal display
                        
/*
#if(BURNIN_MODE)
                        if(((ucMode_Curr == MODE_NOSUPPORT)||(ucMode_Curr == MODE_NOSIGNAL)))
                             RTDSetBit(VDIS_CTRL_20, 0x5f, 0x20 | DHS_MASK);        // Normal display
						else
						     RTDSetBit(VDIS_CTRL_20, 0x5f, DHS_MASK);        // Normal display
#else
                         RTDSetBit(VDIS_CTRL_20, 0x5f, DHS_MASK);        // Normal display
#endif
*/

#if(MCU_TYPE == MCU_WINBOND)
                        bLIGHT_PWR  = LIGHT_ON;
#else
                        MCU_WriteBacklightPower(LIGHT_ON);
#endif

			//New Mode Auto
			New_Mode_Auto();		//anson 05_0314

#if(FIX_LAST_DHT)
                        if(bFrameSync)
						{
					//	   RTDSetBit(FX_LST_LEN_H_5A,0xff,0x10); //Enable the Fixed DVTOTAL & Last Line Lenghth Fucntion
					//	   RTDSetByte(FX_LST_LEN_H_5A,0x1b); //Enable the Fixed DVTOTAL & Last Line Lenghth Fucntion
						}
 	                       
#endif


#if(AS_DV_TOTAL)
                        if(bFrameSync)
						{
						      RTDSetByte(DV_TOTAL_STATUS_3D,0x00);  //Write once to clear status
                              RTDSetBit(DV_BKGD_STA_31,0x7f,0x80);
					     }
#endif
                    }
                    else if ((1 == usTaskCnt && MODE_NOSIGNAL == ucMode_Curr)||
                    		(1 == usTaskCnt && MODE_NOSUPPORT == ucMode_Curr))		//anson
//                    else if (1 == usTaskCnt && MODE_NOSIGNAL == ucMode_Curr)
                    {
                        //if (PANEL_ON == MCU_ReadPanelPower())
						if ( _ON == bPanel_Status)
                        {
                            Set_Panel(0);   // Turn off panel
                            SetMute(0);
                            // Set RTD3001 to power-saving mode, and ADC to power-down mode.
                            // Do not power down TMDS or it can't detect input signal.
                            // DO NOT put VDC and TMDS into power-down mode.
                            // and VDC can not detect input when powered down.
                            PowerDown_ADC();
                            //Device_Power(ADC_POWER,OFF);
				
                            RTDSetByte(HOSTCTRL_02, 0x42);
                            
                            Delay_Xms(250);
                            Delay_Xms(250);
                            Delay_Xms(250);
                            Delay_Xms(250);
                            Delay_Xms(250);
                        }
                    }
                }
            }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Program
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void UART_Initialize(void);

void main(void)
{
    unsigned char idata     ucNotify;
	bRED_LED   = 1;			//anson MTV512	1 = off	0 = on
	bGREEN_LED = 1;			//	    Tp2804		1 = on	0 = off
#if(USE_MCU_DDC)
    MCU_DDC_Process();
#else
   Firmware_Init();
#endif

#if(RS232_DEBUG)
	UART_Initialize();
#endif

    
	Power_Status_Init();
    ISP_Check_Sum();

#if(RS232_DEBUG)
	PutStringToScr("\nMain Loop..");
#endif


    Delay_Xms(250);
    Delay_Xms(250);
//    Init_GUD();
/*
	Delay_Xms(250);
    Delay_Xms(250);
    Delay_Xms(250);
*/    
//    SetMute(1);//val = 1 open sound , val =0 close sound 			//anson
    RTDCodeW(OSD_Enable);

    // Program Main Loop
    while (1)
    {	
        rgb_LED_LIGHT_CONTROL();
        RTD_Test();

#if (GETREGISTER)
        RTD_Get_Set();
#endif

        if(Frame_Sync_Detector())
		   continue;

        // The code below will be executed every 20ms (bNotify_Timer0_Int is set to 1)
        // bNotify_Timer0_Int must and can only be cleared in the end of the iteration.
        if (bNotify_Timer0_Int)
        {   
#if(RS232_DEBUG)
        if(fTest==1)
		{
		fTest=0;
		DebugModeLoop();
		}
#endif         
            
            // Key Translator 
            ucNotify    = Key_Trans();

            // Power Controller 
            if(Power_Control(ucNotify))
			       continue;

			   
            //Source Controller 
            if(Source_Control())
			      continue;
		

            // Input Mode Detector 
            Input_Mode_Detector();
		

#if(BURNIN_MODE)
		if(((ucMode_Curr == MODE_NOSUPPORT)||(ucMode_Curr == MODE_NOSIGNAL)) && bStable && ((stGUD3.TV_SETTING & 0x20) == 0x20)) //anson
			BurnIn();
		else if((MODE_NOSIGNAL != ucMode_Curr) && ((stGUD3.TV_SETTING & 0x20)==0x20 ))		//anson
		{
			stGUD3.TV_SETTING &= 0xDF ;
			Save_GUD3();
			return;
		}
		else  
#endif
	              Run_Task();

			Signal_Stable(ucNotify);
        
            bNotify_Timer0_Int  = 0;
        }   
        // End of main loop
    }
}

