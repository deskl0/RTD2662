#define __POWERCTRL__

#include "Header\Lcd_main.h"
#include "Header\access.h"
#include "Header\config.h"
#include "Header\Osd.h"
#include "Header\Initial.h"
#include "Header\Lcd_msg.h"
#include "Header\Frame_Sync.h"
#include "Header\Lcd_func.h"
#include "Header\Font.h"


void PowerDown_ADC(void)
{
    RTDSetByte(PLL1_CTRL_D6, 0xf2);     // Power down PLL1
    RTDSetByte(PLL2_CTRL_DA, 0x40);     // Power down PLL2
    RTDSetByte(ADC_CTRL_E6, 0x40);      // Power down ADC
}

void PowerUp_ADC(void)
{
    RTDSetByte(PLL1_CTRL_D6, 0xf3);     // Power up PLL1
    RTDSetByte(PLL2_CTRL_DA, 0x41);     // Power up PLL2
    RTDSetByte(ADC_CTRL_E6, 0x47);      // Power up ADC


    RTDSetByte(PLL2_FILTER_DD, 0x5f);

}

void PowerDown_TMDS(void)
{
    RTDSetByte(TMDS_OUTPUT_ENA_A0, 0x0F);    
}

void PowerUp_TMDS(void)
{
   RTDSetByte(TMDS_OUTPUT_ENA_A0, 0x8F);
}


void PowerDown_VDC(void)
{
#if (VIDEO_CHIP != VDC_NONE)

#if(MCU_TYPE == MCU_WINBOND)
    bVDC_PWR    = VDC_OFF;
#else
    MCU_WriteVideoPower(VDC_OFF);
#endif

#endif
}

void PowerUp_VDC(void)
{
#if (VIDEO_CHIP != VDC_NONE)

#if(MCU_TYPE == MCU_WINBOND)
     bVDC_PWR    = VDC_ON;
#else
     MCU_WriteVideoPower(VDC_ON);
#endif

#endif
}

void PowerDown_LVDS(void)
{
#if (OUTPUT_BUS == LVDS_TYPE)
RTDCodeW(LVDS_POWERDOWN);
#endif
}

void PowerUp_LVDS(void)
{
#if (OUTPUT_BUS == LVDS_TYPE)
RTDCodeW(LVDS_POWERUP);
#endif
}

void Set_Panel(unsigned char status)
{
    if (status)
    {
        PowerUp_LVDS();
		//Device_Power(LVDS_POWER,ON);

        Delay_Xms(10);


        RTDSetByte(VDIS_SIGINV_21, 0x00 | DISP_EO_SWAP | DISP_RB_SWAP | DISP_ML_SWAP);       // DHS, DVS, DEN, DCLK MUST NOT be inverted.

        RTDSetBit(VDIS_CTRL_20, 0xfd, 0x01);    // DHS, DVS, DEN, DCLK and data are clamped to 0
        Delay_Xms(20);
        RTDSetBit(VDIS_CTRL_20, 0xfe, 0x00);    // Stop display timing
        Delay_Xms(20);

#if(MCU_TYPE == MCU_WINBOND)
        bPANEL_PWR  = PANEL_ON;
#else
        MCU_WritePanelPower(PANEL_ON);
#endif
        bPanel_Status = _ON;

        
        Delay_Xms(40);
		


        RTDSetByte(VDIS_SIGINV_21, DISP_INV | DISP_EO_SWAP | DISP_RB_SWAP | DISP_ML_SWAP);       // DHS, DVS, DEN, DCLK MUST NOT be inverted.

        RTDSetBit(VDIS_CTRL_20, 0xff, 0x03);    // DHS, DVS, DEN, DCLK in normal operation
        
        RTDSetBit(DIS_TIMING0_3A, 0xff, 0x20);  // Force display timing enable
        RTDSetBit(DIS_TIMING0_3A, 0xdf, 0x00);  // Stop forcing

        //Delay_Xms(200);
        //Delay_Xms(200);
        Load_VLC_Font(Font_Global, 0x00, 0xa1);
      	if( (stGUD1.FUNCTION & 0x07) == JAPANESS )
      	{
      		Load_VLC_Font(Font_East_J, 0x573, 0x4B);
      	}
      	else
      	{
     	   	Load_VLC_Font(Font_East_C, 0x5a9, 0x45);
      	}
      	Load_VLC_Font(Font_Icons, 0x828, 0x70);
        Load_VLC_Font(Font_add, 0x20a, 0x06);				//anson   3A(addr)  x  9  = 20A
														//   12 x18 / 8 /3 = 9
	RTDSetByte(STATUS0_01, 0x00);  // Write once to clear status
        RTDSetByte(STATUS1_1F, 0x00);  // Write once to clear status

        
#if(MCU_TYPE == MCU_WINBOND)
        bLIGHT_PWR  = LIGHT_ON;
#else
        MCU_WriteBacklightPower(LIGHT_ON);
#endif

    }
    else
    {
#if(MCU_TYPE == MCU_WINBOND)
        bLIGHT_PWR  = LIGHT_OFF;
#else
        MCU_WriteBacklightPower(LIGHT_OFF);
#endif

        Delay_Xms(120);

        RTDOSDW(OSD_Reset);


        RTDSetByte(VDIS_SIGINV_21, 0x00 | DISP_EO_SWAP | DISP_RB_SWAP | DISP_ML_SWAP);       // DHS, DVS, DEN, DCLK MUST NOT be inverted.

        RTDSetBit(VDIS_CTRL_20, 0xfd, 0x01);    // DHS, DVS, DEN, DCLK and data are clamped to 0
        Delay_Xms(20);
        RTDSetBit(VDIS_CTRL_20, 0xfe, 0x00);    // Stop display timing
        Delay_Xms(20);
#if(MCU_TYPE == MCU_WINBOND)	 
        bPANEL_PWR  = PANEL_OFF;
#else
        MCU_WritePanelPower(PANEL_OFF);
#endif
        bPanel_Status = _OFF;
        Delay_Xms(40);

        PowerDown_LVDS();
		//Device_Power(LVDS_POWER,OFF);
    }
}

            //////////////////////
            // Power Controller //
            //////////////////////
bit Power_Control(unsigned char Notify)
{
            if (0 == bPower_Status)
            {

                if (NOTIFY_POWERUP == Notify)
                {
                    bPower_Status   = 1;

                    Power_Up_Init();
                    ISP_Check_Sum();

                    bStable     = 0;                    // Assume input signal is not stable when power up
                    bReload     = 1;                    // Always reload font when power up
                    bFrameSync  = 0;                    // Not sync yet
                    ucModeCnt   = MODE_DETECT_FREQ;     // Reset Input Mode Detector
                    
                    Measure_Mode();                     // Measure mode-timing
                    Set_Task(STATE_POWERUP);            // Notify Task State Machine
#if (RTDDEBUG)
                    ucMode_PrevAct  = MODE_NOSIGNAL;
                    ucMode_QuitCnt  = 0;
                    ucDebug         = 0;
#endif

#if (POWER_KEY_TYPE == TYPE_ONE_TOUCH)
                    // Save power status when power-key is one-touch type
                    stGUD1.FUNCTION &= 0xf7;
                    Save_GUD1();
                    Delay_Xms(10);
#endif
                }

                bNotify_Timer0_Int  = 0;

//               continue;   // leave current iteration.
                 return _TRUE;
            }
            else if (NOTIFY_POWERDOWN == Notify)
            {
#if (POWER_KEY_TYPE == TYPE_ONE_TOUCH)
                // Save power status when power-key is one-touch type
                stGUD1.FUNCTION |= 0x08;
                Save_GUD1();
                Delay_Xms(10);
#endif
                bPower_Status   = 0;

                Set_Panel(0);                   // Turn off panel
                SetMute(0);

                RTDSetByte(HOSTCTRL_02, 0x42);  // Set RTD to power-saving

                I2CWrite(V_DISABLE);

                PowerDown_ADC();
                PowerDown_VDC();
                PowerDown_TMDS();
				

                Delay_Xms(250);
                Delay_Xms(250);
                Delay_Xms(250);
                Delay_Xms(250);
                Delay_Xms(250);

                Set_Task(STATE_POWERDOWN);      // Notify Task State Machine

                bNotify_Timer0_Int  = 0;

                //continue;   // leave current iteration.
				return _TRUE;
            }
			return _FALSE;
}
