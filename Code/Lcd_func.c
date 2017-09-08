#define __FUNC__

#include "reg52.h"
#include "intrins.h"

#include "Header\MAIN_DEF.H"
#include "Header\MTV512.H"
#include "Header\CONFIG.H"
#include "Header\ACCESS.H"
#include "Header\OSD.H"
#include "Header\LCD_COEF.H"
#include "Header\LCD_MAIN.H"
#include "Header\LCD_AUTO.H"
#include "Header\LCD_FUNC.H"
#include "Header\FRAME_SYNC.H"
#include "Header\LCD_OSD.H"

// val =1 open sound,val = 0 close sound
void SetMute(bit val)
{
//anson MTV 512
	if(val == 0)   //關聲音
	{
		bMUTE = 0;			// 1 : On   0 : Off
		bSTANDBY = 1;
	}
	else 		//開聲音
	{
		bMUTE = 1;
		bSTANDBY = 0;
	}

//anson Tp2804
/*
	if(val == 1)
	{
		bMUTE = 0;			// 1 : Off   0 : On
		bSTANDBY = 0;
	}
	else 
	{
		bMUTE = 1;
		bSTANDBY = 1;
	}
*/
}

#if (TV_CHIP != TV_NONE)
#include "TUNER.H"
#endif

void SetVolume()
{
    stGUD3.VOLUME   &= 0x1f;

#if (AUDIO_TYPE == AUDIO_LM4832)
    Data[0] = 10;
    Data[1] = ADDR_LM4832;
    Data[2] = 0x00;
    Data[3] = 0x02;
    Data[4] = 0x00;                     // Input Volume - 0 dB
    Data[5] = 0x26;                     // Bass         - 0 dB
    Data[6] = 0x46;                     // Treble       - 0 dB
    Data[7] = 0x60 | stGUD3.VOLUME;     // Right Volume
    Data[8] = 0x80 | stGUD3.VOLUME;     // Left Volume

    // Mic 1 selected with 20dB when input source is VGA or DVI
    // Both Mic 1 and 2 selected with 20dB when input source is AV or S-Video
    Data[9] = (SOURCE_AV <= (stGUD1.INPUT_SOURCE & 0x07)) ? 0xa6 : 0xa4;
    
    I2CWrite(Data);
#endif

#if (AUDIO_TYPE == AUDIO_PWM2)
    Data[0] = 6;
    Data[1] = Y_INC;
    Data[2] = OSD_ROW_90;
    Data[3] = 0x80;
    Data[4] = 0x01;
#if (INV_VOLUME)
    Data[5] = (0xff -(stGUD3.VOLUME << 3));
#else
	Data[5] = (stGUD3.VOLUME << 3);
#endif
	Data[6] = 0;
    RTDWrite(Data);
#endif


#if (AUDIO_TYPE == AUDIO_PWM0)

    Data[0] = 6;
    Data[1] = Y_INC;
    Data[2] = OSD_ROW_90;
    Data[3] = 0x00;
    Data[4] = 0x01;
#if (INV_VOLUME)
    Data[5] = (0xff -(stGUD3.VOLUME << 3));
#else
	Data[5] = (stGUD3.VOLUME << 3);
#endif
	Data[6] = 0;
    RTDWrite(Data);
#endif
}

void WriteGamma(unsigned char code *arrayR, unsigned char code *arrayG, unsigned char code *arrayB)
{
    unsigned char   n   = 0;

    RTDSetBit(COLOR_CTRL_5D, 0xfb, 0x10);   // Disable GAMMA & Enable Access Channel
    

    // GAMMA_RED
    bRTD_SCSB   = 0;
    RTDSendAddr(RED_GAMMA_64, WRITE, N_INC);
    do
    {
        RTDSendByte(arrayR[n]);
    }
    while (++n);    // if n is 0xff, then n will be 0x00 after increased.

#if(MCU_TYPE == MCU_WINBOND)
    bRTD_SCLK   = 0; 
    bRTD_SCLK   = 1;           
    bRTD_SCSB   = 1;
    
    // GAMMA_GREEN
    bRTD_SCSB   = 0;
#else
    MCU_WriteRtdSclk(_LOW); 
    MCU_WriteRtdSclk(_HIGH);           
    MCU_WriteRtdScsb(_HIGH);
    
    // GAMMA_GREEN
    MCU_WriteRtdScsb(_LOW);
#endif

    RTDSendAddr(GRN_GAMMA_65, WRITE, N_INC);
    do
    {
        RTDSendByte(arrayG[n]);
    }
    while (++n);
#if(MCU_TYPE == MCU_WINBOND)
    bRTD_SCLK   = 0; 
    bRTD_SCLK   = 1;           
    bRTD_SCSB   = 1;
    
    //GAMMA_BLUE
    bRTD_SCSB   = 0;
#else
    MCU_WriteRtdSclk(_LOW); 
    MCU_WriteRtdSclk(_HIGH);           
    MCU_WriteRtdScsb(_HIGH);
    
    //GAMMA_BLUE
    MCU_WriteRtdScsb(_LOW);
#endif
    RTDSendAddr(BLU_GAMMA_66, WRITE, N_INC);
    do
    {
        RTDSendByte(arrayB[n]);
    }
    while (++n);

#if(MCU_TYPE == MCU_WINBOND)
    bRTD_SCLK   = 0; 
    bRTD_SCLK   = 1;           
    bRTD_SCSB   = 1;
#else
    MCU_WriteRtdSclk(_LOW); 
    MCU_WriteRtdSclk(_HIGH);           
    MCU_WriteRtdScsb(_HIGH);
#endif

    
    RTDSetBit(COLOR_CTRL_5D, 0xef, 0x04);   // Enable GAMMA & Diable Access Channel
}

void WriteDither(unsigned char code *array , bit new_dither)
{
    unsigned char   n;
    if(new_dither)
      RTDSetBit(FX_LST_LEN_H_5A,0xff,0x80);
    else
      RTDSetBit(FX_LST_LEN_H_5A,0x7f,0x00);

    RTDSetBit(COLOR_CTRL_5D, 0xb7, 0x68);   // Enable DITHER & Enable Access Channels

#if(MCU_TYPE == MCU_WINBOND)
    bRTD_SCSB   = 0;
    RTDSendAddr(DITHER_PORT_67, WRITE, N_INC);

    for (n = 0; n < 8; n++)     RTDSendByte(array[n]);

    bRTD_SCLK   = 0; 
    bRTD_SCLK   = 1;           
    bRTD_SCSB   = 1;
#else
    MCU_WriteRtdScsb(_LOW);
    RTDSendAddr(DITHER_PORT_67, WRITE, N_INC);

    for (n = 0; n < 8; n++)     RTDSendByte(array[n]);

    MCU_WriteRtdSclk(_LOW); 
    MCU_WriteRtdSclk(_HIGH);           
    MCU_WriteRtdScsb(_HIGH);
#endif
    
    //RTDSetBit(COLOR_CTRL_5D, 0xdf, 0x48);   // Enable DITHER & Disable Access Channels
	RTDSetBit(COLOR_CTRL_5D, 0x1f, 0x88);   // Enable DITHER & Disable Access Channels
}

void WriteSU_COEF(unsigned char code *arrayH, unsigned char code *arrayV)
{
    unsigned char   n;
    


    RTDSetBit(FILTER_CTRL1_1C, 0xfc, 0x01);     // Enable H-Coeff access
#if(MCU_TYPE == MCU_WINBOND)
    bRTD_SCSB   = 0;
    RTDSendAddr(FILTER_PORT_1D, WRITE, N_INC);
    for (n = 0; n < 128; n++)    RTDSendByte(arrayH[n]);

    bRTD_SCLK   = 0; 
    bRTD_SCLK   = 1;           
    bRTD_SCSB   = 1;

    RTDSetBit(FILTER_CTRL1_1C, 0xfc, 0x03);     // Enable V-Coeff access
    
    bRTD_SCSB   = 0;
    RTDSendAddr(FILTER_PORT_1D, WRITE, N_INC);
    for (n = 0; n < 128; n++)    RTDSendByte(arrayV[n]);

    bRTD_SCLK   = 0; 
    bRTD_SCLK   = 1;           
    bRTD_SCSB   = 1;

#else
    MCU_WriteRtdScsb(_LOW);
    RTDSendAddr(FILTER_PORT_1D, WRITE, N_INC);
    for (n = 0; n < 128; n++)    RTDSendByte(arrayH[n]);

    MCU_WriteRtdSclk(_LOW); 
    MCU_WriteRtdSclk(_HIGH);           
    MCU_WriteRtdScsb(_HIGH);

    RTDSetBit(FILTER_CTRL1_1C, 0xfc, 0x03);     // Enable V-Coeff access
    
    MCU_WriteRtdScsb(_LOW);
    RTDSendAddr(FILTER_PORT_1D, WRITE, N_INC);
    for (n = 0; n < 128; n++)    RTDSendByte(arrayV[n]);

    MCU_WriteRtdSclk(_LOW); 
    MCU_WriteRtdSclk(_HIGH);           
    MCU_WriteRtdScsb(_HIGH);

#endif

    RTDSetBit(FILTER_CTRL1_1C, 0xfc, 0xc4);     // Disable filter coefficient access
}

void Set_H_Position(void)
{
#if(AS_NON_FRAMESYNC)
    RTDSetBit(ODD_CTRL_8E,0xef,0x00);
#endif

    // if the backporch is far small then the standard one,
	// it is possibile that even IHS_Delay decrease to zero still can't correct the H position
	// so adjust the usIPH_ACT_STA first and turn back to original value later

    if(stMUD.H_POSITION < ucH_Min_Margin)
    {
       usIPH_ACT_STA = usIPH_ACT_STA - (ucH_Min_Margin - stMUD.H_POSITION);
       stMUD.H_POSITION = ucH_Min_Margin;
    }

#if(ALIGN_LEFT == CLOCK_ALIGN)
    ((unsigned int*)Data)[4] = usIPH_ACT_STA + (stMUD.CLOCK >> 2) - 32;
#else
    ((unsigned int*)Data)[4] = usIPH_ACT_STA + (stMUD.CLOCK >> 1) - 64;
#endif


    Wait_For_Event(EVENT_IEN_STOP);
        
    Data[0] = 5;
    Data[1] = Y_INC;
    Data[2] = IPH_ACT_STA_06;
    Data[3] = (unsigned char)((unsigned int*)Data)[4];
    Data[4] = (unsigned char)(((unsigned int*)Data)[4] >> 8);
    Data[5] = 0;    
    RTDWrite(Data);

    // Update IHS delay according to phase
//    Set_Phase(stMUD.PHASE & 0x7c);
    //Data[0]     = PROGRAM_HDELAY + (stMUD.H_POSITION - ucH_Min_Margin);
     Data[12] = (stMUD.H_POSITION - ucH_Min_Margin) + PROGRAM_HDELAY;
	
	RTDSetByte(IHS_DELAY_8D, Data[12]);
   

#if(AS_NON_FRAMESYNC)
    if(bFrameSync && bStable)
        RTDSetBit(ODD_CTRL_8E,0xef,0x10);
#endif
}

#if(PANEL_TYPE == PANEL_HYUNDAI)		//hgxxxx 0522 for Hyundai
#define MAX_MOVE_LINE	2
bit	bSTEP_VPOS	= 0;
static unsigned char idata ucPre_V_POS = 0x80;
void Set_V_Position4HD(void);

void Set_V_Position(void)
{
	unsigned char tempstore;
	tempstore = stMUD.V_POSITION;
	bSTEP_VPOS = 1;
    do
	{
		if(stMUD.V_POSITION > ucPre_V_POS)
		{
			if(stMUD.V_POSITION - ucPre_V_POS > MAX_MOVE_LINE)
				stMUD.V_POSITION = ucPre_V_POS + MAX_MOVE_LINE;
		}
		else
		{
			if(ucPre_V_POS - stMUD.V_POSITION > MAX_MOVE_LINE)
				stMUD.V_POSITION = ucPre_V_POS - MAX_MOVE_LINE;
		}
		Set_V_Position4HD();
		stMUD.V_POSITION = tempstore;
	}
	while(stMUD.V_POSITION != ucPre_V_POS);
	bSTEP_VPOS = 0;
}

void Set_V_Position4HD(void)
#else
void Set_V_Position(void)
#endif

{
    unsigned int    usIV_Temp, usDV_Temp;


#if(AS_NON_FRAMESYNC)
    RTDSetBit(ODD_CTRL_8E,0xef,0x00);
#endif

#if(AS_DV_TOTAL)
    RTDSetBit(DV_BKGD_STA_31,0x7f,0x00);
#endif

    if ((ucV_Max_Margin - 1)  < stMUD.V_POSITION)
    {
       
        RTDSetByte(IVS_DELAY_8C, (PROGRAM_VDELAY + stMUD.V_POSITION - (ucV_Max_Margin - 1)));


        usIV_Temp   = usIPV_ACT_STA + (ucV_Max_Margin - 1) - 128;
        usDV_Temp   = (unsigned int)ucDV_Delay + (ucV_Max_Margin - 1) - 128;
    }
    else
    {

        RTDSetByte(IVS_DELAY_8C, PROGRAM_VDELAY);


        usIV_Temp   = usIPV_ACT_STA + stMUD.V_POSITION - 128;
        usDV_Temp   = (unsigned int)ucDV_Delay + stMUD.V_POSITION - 128;
    }

    Wait_For_Event(EVENT_IEN_START);

    Data[0] = 4;
    Data[1] = N_INC;
    Data[2] = IV_DV_LINES_38;
    Data[3] = (unsigned char)usDV_Temp;
    Data[4] = 5;    
    Data[5] = Y_INC;
    Data[6] = IPV_ACT_STA_0A;
    Data[7] = (unsigned char)usIV_Temp;
    Data[8] = (unsigned char)(usIV_Temp >> 8);
    Data[9] = 0;
    RTDWrite(Data);

    Wait_For_Event(EVENT_IEN_START);


	RTDSetByte(STATUS0_01, 0x00);  // Clear status
    RTDSetByte(STATUS1_1F, 0x00);  // Clear status


#if(AS_NON_FRAMESYNC)
    if(bFrameSync && bStable)
        RTDSetBit(ODD_CTRL_8E,0xef,0x10);
#endif

#if(AS_DV_TOTAL)
    if(bFrameSync && bStable)
    {
       RTDSetByte(DV_TOTAL_STATUS_3D,0x00);  //Write once to clear status
       RTDSetBit(DV_BKGD_STA_31,0x7f,0x80);
    }
#endif

#if(PANEL_TYPE == PANEL_HYUNDAI)		//hgxxxx 0522 for Hyundai
	ucPre_V_POS = stMUD.V_POSITION;
#endif
}

/*
void Set_V_Position(void)
{
    unsigned int    usIV_Temp, usDV_Temp;
#if(PANEL_TYPE == PANEL_HANDAI)
	unsigned int	usDHT,usDHTc;
	unsigned char	ucIVD0,ucIVD1;
#endif

#if(AS_NON_FRAMESYNC)
    RTDSetBit(ODD_CTRL_8E,0xef,0x00);
#endif


    if (ucV_Max_Margin < stMUD.V_POSITION)
    {

        RTDSetByte(IVS_DELAY_8C, (PROGRAM_VDELAY + stMUD.V_POSITION - ucV_Max_Margin));


        usIV_Temp   = usIPV_ACT_STA + ucV_Max_Margin - 128;
        usDV_Temp   = (unsigned int)ucDV_Delay + ucV_Max_Margin - 128;
    }
    else
    {

        RTDSetByte(IVS_DELAY_8C, PROGRAM_VDELAY);


        usIV_Temp   = usIPV_ACT_STA + stMUD.V_POSITION - 128;
        usDV_Temp   = (unsigned int)ucDV_Delay + stMUD.V_POSITION - 128;
    }

#if(PANEL_TYPE == PANEL_HANDAI)
	//DHT =  xxxxxxxx_xxx00000 (0x22) + 00000000_00xxxxx0 (0x2e)
	RTDRead(0x22, 0x02, Y_INC);
	usDHT = 256 * (Data[1]&0x07) + Data[0] + 2;
	RTDRead(0x2E, 0x01, Y_INC);
	//to restore DV_tatal
	Data[15] = Data[0];
	usDHT =	usDHT*32 + ((Data[0]>>2) & 0x3e);


	//compensated DH_Total
	RTDRead(IVS_DELAY_8C, 0x01, Y_INC);
	//original IVS delay
	ucIVD0 = Data[0];  
	//The IVS delay you are going to set
	ucIVD1 = (ucV_Max_Margin < stMUD.V_POSITION) ? (0x80 | (PROGRAM_VDELAY + stMUD.V_POSITION - ucV_Max_Margin)) : (0x80 | PROGRAM_VDELAY);
	//original IVS_DVS_Delay
	RTDRead(IV_DV_LINES_38, 0x01, Y_INC);
	//usDV_Temp  => IVS_DVS_delay you are going to set
	usDHTc = (unsigned long)usDHT * (usVsync + usDV_Temp - Data[0] + ucIVD1 - ucIVD0)/usVsync - 0x40;//-2
    
    


	//modify DH_Total
    Wait_For_Event(EVENT_IEN_STOP);
	Data[0] = 5;
    Data[1] = Y_INC;
    Data[2] = DH_TOTAL_22;
    Data[3] = (unsigned char)((usDHTc>>5) & 0x00fe);
	Data[4] = (unsigned char)((usDHTc>>13) & 0x0007);
	Data[5] = 4;
	Data[6] = Y_INC;
	Data[7] = 0x2e;
	Data[8] = (Data[15] & 0x07) | (((usDHTc+1)<<2) & 0x00f8);
	Data[9] = 0;
    RTDWrite(Data);

#endif
    Wait_For_Event(EVENT_IEN_START);

    Data[0] = 4;
    Data[1] = N_INC;
    Data[2] = IV_DV_LINES_38;
    Data[3] = (unsigned char)usDV_Temp;
    Data[4] = 5;    
    Data[5] = Y_INC;
    Data[6] = IPV_ACT_STA_0A;
    Data[7] = (unsigned char)usIV_Temp;
    Data[8] = (unsigned char)(usIV_Temp >> 8);
    Data[9] = 0;
    RTDWrite(Data);


#if(PANEL_TYPE == PANEL_HANDAI)
    //restore DH_Total
	usDHT = usDHT - 0x40;	
    Wait_For_Event(EVENT_DEN_STOP);
	Data[0] = 5;
    Data[1] = Y_INC;
    Data[2] = 0x22;
    Data[3] = (unsigned char)((usDHT>>5) & 0x00fe);
	Data[4] = (unsigned char)((usDHT>>13) & 0x0007);
	Data[5] = 4;
	Data[6] = Y_INC;
	Data[7] = 0x2e;
	Data[8] = (Data[15] & 0x07) | ((usDHT<<2) & 0x00f8);
	Data[9] = 0;
    RTDWrite(Data);	
#endif

    Wait_For_Event(EVENT_IEN_START);


	RTDSetByte(STATUS0_01, 0x00);  // Clear status
    RTDSetByte(STATUS1_1F, 0x00);  // Clear status


#if(AS_NON_FRAMESYNC)
    if(bFrameSync && bStable)
        RTDSetBit(ODD_CTRL_8E,0xef,0x10);
#endif
}
*/
void Set_Clock(void)
{
    unsigned char   ucM_Code, ucN_Code, ucTemp0, ucTemp1, ucResult;

    // Issac :
    // In this F/W, the frequency of PLL1 is fixed to 24.576*19/2=233.472MHz.
    // Our goal is to find the best M/N settings of PLL2 according to the relationship below
    // Best Fav = 233.472 * 32 / 31 = 241.003Mhz, and pixel rate = Fav * M / N
    // Too small or large N code will cause larger jitter of ADC clock.
    // In this F/W, I limite N code value between 16 and 31.

    unsigned int    usClock = usADC_Clock + (unsigned int)stMUD.CLOCK - 128;    // Pixel clock number
    unsigned long   ulRate  = (unsigned long)24576 * usClock / usStdHS;         // Pixel clock in kHz

#if(AS_PLL_NONLOCK)
    RTDSetBit(ODD_CTRL_8E,0xdf,0x00);
#endif

#if(AS_NON_FRAMESYNC)
    RTDSetBit(ODD_CTRL_8E,0xef,0x00);
#endif

#if (TUNE_APLL)
    RTDSetBit(DV_TOTAL_STATUS_3D, 0xdf, 0x00);//Disable PE Max Measurement
    RTDSetByte(DV_TOTAL_STATUS_3D,0x40); //clear PE Max value
    ucPE_Max = 0;
#endif

/*

    ((unsigned int *)Data)[0]  = 375;

    ucM_Code    = 0;
    ucN_Code    = 0;
    ucResult    = 0;
    ucTemp0     = 7;
    do
    {
        ucTemp1 = ulRate * ucTemp0 / 241003;

        if (2 <= ucTemp1)
        {
            ((unsigned long *)Data)[2]  = ulRate * ucTemp0 / ucTemp1;
            
            ((unsigned int *)Data)[1]   = (unsigned long)3735552000 / ((unsigned long *)Data)[2];

            if (15500 <= ((unsigned int *)Data)[1])
            {
                if (15700 >= ((unsigned int *)Data)[1])
                {
                    if (0 != ucResult || (((unsigned long)usClock * ucTemp0 / ucTemp1) * ucTemp1) != ((unsigned long)usClock * ucTemp0))
                    {
                        ucN_Code    = ucTemp0;
                        ucM_Code    = ucTemp1;
                        break;
                    }
                }

                ((unsigned int *)Data)[1]   = ((unsigned int *)Data)[1] - 15500;
            }
            else
            {
                if (15375 <= ((unsigned int *)Data)[1])
                {
                    if (0 != ucResult || (((unsigned long)usClock * ucTemp0 / ucTemp1) * ucTemp1) != ((unsigned long)usClock * ucTemp0))
                    {
                        ucN_Code    = ucTemp0;
                        ucM_Code    = ucTemp1;
                        break;
                    }
                }
                
                ((unsigned int *)Data)[1]   = 15500 - ((unsigned int *)Data)[1];
            }
            
            if (((unsigned int *)Data)[0] > ((unsigned int *)Data)[1])
            {
                ((unsigned int *)Data)[0]   = ((unsigned int *)Data)[1];

                ucN_Code    = ucTemp0;
                ucM_Code    = ucTemp1;
            }
        }

        ucTemp1 = ucTemp1 + 1;

        if (2 <= ucTemp1)
        {
            ((unsigned long *)Data)[2]  = ulRate * ucTemp0 / ucTemp1;

            ((unsigned int *)Data)[1]   = (unsigned long)3735552000 / ((unsigned long *)Data)[2];

            if (15500 <= ((unsigned int *)Data)[1])
            {
                if (15700 >= ((unsigned int *)Data)[1])
                {
                    if (0 != ucResult || (((unsigned long)usClock * ucTemp0 / ucTemp1) * ucTemp1) != ((unsigned long)usClock * ucTemp0))
                    {
                        ucN_Code    = ucTemp0;
                        ucM_Code    = ucTemp1;
                        break;
                    }
                }

                ((unsigned int *)Data)[1]   = ((unsigned int *)Data)[1] - 15500;
            }
            else
            {
                if (15375 <= ((unsigned int *)Data)[1])
                {
                    if (0 != ucResult || (((unsigned long)usClock * ucTemp0 / ucTemp1) * ucTemp1) != ((unsigned long)usClock * ucTemp0))
                    {
                        ucN_Code    = ucTemp0;
                        ucM_Code    = ucTemp1;
                        break;
                    }
                }
                
                ((unsigned int *)Data)[1]   = 15500 - ((unsigned int *)Data)[1];
            }

            if (((unsigned int *)Data)[0] > ((unsigned int *)Data)[1])
            {
                ((unsigned int *)Data)[0]   = ((unsigned int *)Data)[1];

                ucN_Code    = ucTemp0;
                ucM_Code    = ucTemp1;
            }
        }

        if (0 == ucResult)
        {
            if (43 < ucTemp0 && 60000 < ulRate)
            {
                ucTemp0     = 10;
                ucResult    = 1;

                ((unsigned int *)Data)[0]  = 500;
            }
        }
        else
        {
            if (36 < ucTemp0 && 200 > ((unsigned int *)Data)[0])
            {
                break;
            }
        }
    }
    while (53 >= ++ucTemp0);

    if (8 >= ucN_Code)
    {
        ucN_Code    = ucN_Code * 3;
        ucM_Code    = ucM_Code * 3;
    }
    else if (12 >= ucN_Code)
    {
        ucN_Code    = ucN_Code * 2;
        ucM_Code    = ucM_Code * 2;
    }

    usClock     = usClock - 1;

    Wait_For_Event(EVENT_IEN_STOP);

    RTDSetByte(I_CODE_MB_CA, 0x18);
    RTDSetByte(I_CODE_LB_C9, 0x00);
    RTDSetByte(P_CODE_CB, 0x18);

    Data[0]     = 5;
    Data[1]     = Y_INC;
    Data[2]     = PLL1_M_D7;
    Data[3]     = 0x11;
    Data[4]     = 0x00;
    Data[5]     = 0;
    RTDWrite(Data);

*/

    ((unsigned int *)Data)[0]  = 500;

    ucM_Code    = 0;
    ucN_Code    = 0;
    ucResult    = 0;
    ucTemp0     = 7;
    do
    {
        //Fav * PLL2_M / PLL2_N = ulRate
        //PLL2_M = ulRate * PLL2_N / Fav;
        //ucTemp1 = ulRate * ucTemp0 / 253687;// (20/2 * 24.576 *32/31)
        ucTemp1 = ulRate * ucTemp0 / 215634;// (17/2 * 24.576 *32/31)

        if (2 <= ucTemp1)
        {   //Fav = ulRate * PLL2_N / PLL2_M
            ((unsigned long *)Data)[2]  = ulRate * ucTemp0 / ucTemp1;
                                                        //(20/2 * 24.576 * 16)
			//((unsigned int *)Data)[1]   = (unsigned long)393216000 / ((unsigned long *)Data)[2];
            ((unsigned int *)Data)[1]   = (unsigned long)334223600 / ((unsigned long *)Data)[2];
            // > 31/2 = 15.5
            if (1550 <= ((unsigned int *)Data)[1])
            {
                if (1570 >= ((unsigned int *)Data)[1])
                {
                    ucN_Code    = ucTemp0;
                    ucM_Code    = ucTemp1;
                    break;
                }

                ((unsigned int *)Data)[1]   = ((unsigned int *)Data)[1] - 1550;
            }
            else //小於32/31時，jitter爛的比較快
            {
                if (1537 <= ((unsigned int *)Data)[1])
                {
                    ucN_Code    = ucTemp0;
                    ucM_Code    = ucTemp1;
                    break;
                }
                
                ((unsigned int *)Data)[1]   = 1550 - ((unsigned int *)Data)[1];
            }
            
            if (((unsigned int *)Data)[0] > ((unsigned int *)Data)[1])
            {
                ((unsigned int *)Data)[0]   = ((unsigned int *)Data)[1];

                ucN_Code    = ucTemp0;
                ucM_Code    = ucTemp1;
            }
        }

        ucTemp1 = ucTemp1 + 1;

        if (2 <= ucTemp1)
        {
            ((unsigned long *)Data)[2]  = ulRate * ucTemp0 / ucTemp1;

            //((unsigned int *)Data)[1]   = (unsigned long)393216000 / ((unsigned long *)Data)[2];
            ((unsigned int *)Data)[1]   = (unsigned long)334223600 / ((unsigned long *)Data)[2];

            if (1550 <= ((unsigned int *)Data)[1])
            {
                if (1570 >= ((unsigned int *)Data)[1])
                {
                    ucN_Code    = ucTemp0;
                    ucM_Code    = ucTemp1;
                    break;
                }

                ((unsigned int *)Data)[1]   = ((unsigned int *)Data)[1] - 1550;
            }
            else
            {
                if (1537 <= ((unsigned int *)Data)[1])
                {
                    ucN_Code    = ucTemp0;
                    ucM_Code    = ucTemp1;
                    break;
                }
                
                ((unsigned int *)Data)[1]   = 1550 - ((unsigned int *)Data)[1];
            }

            if (((unsigned int *)Data)[0] > ((unsigned int *)Data)[1])
            {
                ((unsigned int *)Data)[0]   = ((unsigned int *)Data)[1];

                ucN_Code    = ucTemp0;
                ucM_Code    = ucTemp1;
            }
        }
    }
    while (53 >= ++ucTemp0);



/*
    if (8 >= ucN_Code)
    {
        ucN_Code    = ucN_Code * 3;
        ucM_Code    = ucM_Code * 3;
    }
    else if (12 >= ucN_Code)
    {
        ucN_Code    = ucN_Code * 2;
        ucM_Code    = ucM_Code * 2;
    }
*/
    usClock     = usClock - 1;

    Wait_For_Event(EVENT_IEN_STOP);
/*
    RTDSetByte(I_CODE_LB_C9, 0x8c);
    RTDSetByte(I_CODE_MB_CA, 0x24);
    RTDSetByte(P_CODE_CB, 0x18);
*/

    RTDSetByte(I_CODE_LB_C9, 0x8c);
    RTDSetByte(I_CODE_MB_CA, 0x20);
    RTDSetByte(P_CODE_CB, 0x18);

    Data[0]     = 5;
    Data[1]     = Y_INC;
    Data[2]     = PLL1_M_D7;
    Data[3]     = 0x0f;//0x12;
    Data[4]     = 0x00;
    Data[5]     = 0;
    RTDWrite(Data);

    Data[0]     = 5;
    Data[1]     = Y_INC;
    Data[2]     = PLLDIV_CC;
    Data[3]     = (unsigned char)(usClock >> 8);
    Data[4]     = (unsigned char)usClock;
    Data[5]     = 5;
    Data[6]     = Y_INC;
    Data[7]     = PLL2_M_DB;
    Data[8]     = ucM_Code - 2;
    Data[9]     = ucN_Code - 2;
    Data[10]    = 0;
    RTDWrite(Data);

    PowerUp_ADC();
    //Device_Power(ADC_POWER,ON);
/*    
    Wait_For_Event(EVENT_IVS);
*/
    Delay_Xms(10);

//    Wait_For_Event(EVENT_IVS);
//    Wait_For_Event(EVENT_IVS);


    RTDSetByte(I_CODE_LB_C9, 0xfc);
    RTDSetByte(I_CODE_MB_CA, 0x21);
    RTDSetByte(P_CODE_CB, 0x17);


//    RTDSetByte(P_CODE_CB, 0x15);
//    RTDSetByte(I_CODE_MB_CA, 0x35);
//    RTDSetByte(I_CODE_LB_C9, 0xdc);


    Wait_For_Event(EVENT_IVS);

    ulRate  = (unsigned long)24576 * usADC_Clock / usStdHS;         // Pixel clock in kHz

#if(NEW_PI_CODE)
/*
    RTDSetByte(I_CODE_LB_C9,0xFC);
    RTDSetByte(I_CODE_MB_CA,0x25);
    RTDSetByte(P_CODE_CB, 0x17);    
*/

    if(ucI_Code)
    {
       if((ucI_Code & 0x80) == 0x80)
           RTDSetBit(I_CODE_MB_CA,0xdf,0x20);  //Set the I_Code[13] to 1;
       else
           RTDSetBit(I_CODE_MB_CA,0xdf,0x00);  //Set the I_Code[13] to 0;

       ucI_Code = ucI_Code & 0x7f;

	   RTDSetByte(I_CODE_LB_C9,0x18 | ((ucI_Code & 0x07) << 5));
	   RTDSetBit(I_CODE_MB_CA,0xfc,0x04 | ((ucI_Code & 0x18) >> 3));
	   RTDSetByte(P_CODE_CB,P_Code);
    }
    else
    {
       RTDSetByte(I_CODE_LB_C9,0x1C);
       RTDSetByte(I_CODE_MB_CA,0x11); //Use old PFD first
       RTDSetByte(P_CODE_CB, 0x16);    
    }

//    RTDSetByte(I_CODE_LB_C9,0xdc);
//    RTDSetByte(I_CODE_MB_CA,0x35);
//    RTDSetByte(P_CODE_CB, 0x16);    

	
#else
    RTDSetByte(I_CODE_LB_C9,0x03);
    RTDSetByte(I_CODE_MB_CA,0x00);
    if (90000 < ulRate)
    {
        RTDSetByte(P_CODE_CB, 0x19);
        //RTDSetByte(P_CODE_CB, 0x1a);
        
    }
    else if (60000 < ulRate)
    {
        RTDSetByte(P_CODE_CB, 0x19);
        //RTDSetByte(P_CODE_CB, 0x1a);        
    }
    else if (35000 < ulRate)
    {
        //RTDSetByte(P_CODE_CB, 0x19);
        RTDSetByte(P_CODE_CB, 0x18);
        Delay_Xms(2);

        //RTDSetByte(P_CODE_CB, 0x1b);
        RTDSetByte(P_CODE_CB, 0x1a);
    }
    else
    {
        //RTDSetByte(P_CODE_CB, 0x19);
        RTDSetByte(P_CODE_CB, 0x18);
        Delay_Xms(2);

        //RTDSetByte(P_CODE_CB, 0x1b);
        RTDSetByte(P_CODE_CB, 0x1a);
    }
#endif
 
      if(!bAutoInProgress)
           Wait_For_Event(EVENT_IVS);
 //   Wait_For_Event(EVENT_IVS);

#if (TUNE_APLL)
    if (ucPE_Level)
    {
        //RTDSetByte(I_CODE_LB_C9, 0x00);
        //RTDSetByte(I_CODE_MB_CA, 0xff);

        Delay_Xms(2);
    }
#endif


	RTDSetByte(STATUS0_01, 0x00);  // Clear status
    RTDSetByte(STATUS1_1F, 0x00);  // Clear status


#if(AS_PLL_NONLOCK)
   if(bFrameSync && bStable)
        RTDSetBit(ODD_CTRL_8E,0xdf,0x20);
#endif

#if(AS_NON_FRAMESYNC)
    if(bFrameSync && bStable)
        RTDSetBit(ODD_CTRL_8E,0xef,0x10);
#endif
} 

void Set_Phase(unsigned char phase)
{
    unsigned char   ucX_Ctrl, ucY_Ctrl, ucSelect;


    unsigned long   ulRate  = (unsigned long)24576 * usADC_Clock / usStdHS;     // Standard pixel clock in kHz

#if(MORE_PHASE)
    Data[0] = phase & 0x03;
#endif
    phase   &= 0x7c;


    ucSelect    = phase & 0x3c;

    phase       = phase >> 2;
    ucX_Ctrl    = (4 >= phase || 29 <= phase) ? 0x80 : (13 <= phase && 21 >= phase) ? 0x80 : 0x00;
    ucY_Ctrl    = (12 >= phase || 29 <= phase) ? 0x01 : 0x00;

#if(MORE_PHASE)
    ucY_Ctrl    = ucX_Ctrl | (ucSelect << 1) | ucY_Ctrl | (Data[0] << 1);
#else
    ucY_Ctrl    = ucX_Ctrl | (ucSelect << 1) | ucY_Ctrl;
#endif




    // Issac :
    // Code below is to select stable HSYNC latch edge.
    // There is about 12.23ns delay between input clock into ADC and output from ADC.
    // Calculating the corresponding phase delay for 6.187ns 
    // Original Formula :
	// ucSelect = 32 * 12.23 / (1000000 / ulRate); //for ver C



    ucSelect    = ulRate * 391 / 1000000;


//    ucDebug_Value0 = ucSelect;
    ucSelect    = 32 >= ucSelect ? 32 - ucSelect : 64 - ucSelect;

//    ucDebug_Value1 = ucSelect;

    //Calculate the absolute value from the selected phase to transition
    ucX_Ctrl    = (phase >= ucSelect) ? phase - ucSelect : ucSelect - phase;

    Data[0]     = PROGRAM_HDELAY + (stMUD.H_POSITION - ucH_Min_Margin);
    //compensate the H position shift due to the phase select
   ucSelect    = (phase + 6) < ucSelect ? Data[0] - 1 : Data[0];
//	ucSelect    = Data[0];

    

     Wait_For_Event(EVENT_IEN_STOP);

       //select a correct edge to latch the stable data
    RTDSetByte(MEAS_HS_LATCH_4E, (6 < ucX_Ctrl && 26 > ucX_Ctrl) ? 0x00 : 0x10);
//    RTDSetByte(MEAS_HS_LATCH_4E, 0x00);
    RTDSetByte(IHS_DELAY_8D, ucSelect);
	

    RTDSetByte(PLL_PHASE_9F, ucY_Ctrl);

	Delay_Xms(2);

	RTDSetByte(STATUS0_01, 0x00);  // Clear status
    RTDSetByte(STATUS1_1F, 0x00);  // Clear status

}


//////////////////////////////////////////////////////////////////////////////////////////////////////


void Set_Bright_Contrast(void)
{
    // Set RTD's contrast and brightness
    if (100 < stGUD0.CONTRAST)          stGUD0.CONTRAST         = 50;
    if (100 < stGUD0.RTD_R_CONTRAST)    stGUD0.RTD_R_CONTRAST   = 50;
    if (100 < stGUD0.RTD_G_CONTRAST)    stGUD0.RTD_G_CONTRAST   = 50;
    if (100 < stGUD0.RTD_B_CONTRAST)    stGUD0.RTD_B_CONTRAST   = 50;

    Data[0] = 9;
    Data[1] = Y_INC;
    Data[2] = BRIGHT_R_5E;

#if (0)
    if (SOURCE_VGA == (stGUD1.INPUT_SOURCE & 0x07))
    {
        Data[3] = 0x7f;
        Data[4] = 0x7f;
        Data[5] = 0x7f;
    }
    else
#endif
    {
        Data[3] = 0x7d;
        Data[4] = 0x7d;
        Data[5] = 0x7d;
    }

	switch((stGUD1.INPUT_SOURCE & 0x18) >> 3)
	{
	case 0:
		Data[13] = stGUD4.C1_G;
		Data[14] = stGUD4.C1_B;
		Data[15] = stGUD4.C1_R;		
		break;
	case 1:
		Data[13] = stGUD4.C2_G;
		Data[14] = stGUD4.C2_B;
		Data[15] = stGUD4.C2_R;		
		break;
	case 2:
		Data[13] = stGUD4.C3_G;
		Data[14] = stGUD4.C3_B;
		Data[15] = stGUD4.C3_R;		
		break;
	default:
		Data[13] = stGUD0.RTD_G_CONTRAST;
		Data[14] = stGUD0.RTD_B_CONTRAST;
		Data[15] = stGUD0.RTD_R_CONTRAST;
		break;
	}
#if(ANALOG_CONTRAST)
    if((stGUD1.INPUT_SOURCE & 0x07) == SOURCE_VGA)
        Data[6] = 0x80; //if using the analog gain to adjust contrast,digital gain alway set to 128
    else
      Data[6] = (50 >= stGUD0.CONTRAST) ? 78 + stGUD0.CONTRAST : 104 + (stGUD0.CONTRAST >> 1);
#else
    Data[6] = (50 >= stGUD0.CONTRAST) ? 78 + stGUD0.CONTRAST : 104 + (stGUD0.CONTRAST >> 1);  
#endif


	Data[7] = (50 >= Data[13]) ? (unsigned int)Data[6] * (75 + (Data[13] >> 1)) / 100
	                                        : (unsigned int)Data[6] * (88 + (Data[13] >> 2)) / 100;

	Data[8] = (50 >= Data[14]) ? (unsigned int)Data[6] * (75 + (Data[14] >> 1)) / 100
                                            : (unsigned int)Data[6] * (88 + (Data[14] >> 2)) / 100;

	Data[6] = (50 >= Data[15]) ? (unsigned int)Data[6] * (75 + (Data[15] >> 1)) / 100
                                            : (unsigned int)Data[6] * (88 + (Data[15] >> 2)) / 100;

    Data[9] = 0;
    RTDWrite(Data);

    if (100 < stGUD0.BRIGHT)    stGUD0.BRIGHT   = 50;

    Data[0] = 6;
    Data[1] = Y_INC;
    Data[2] = OSD_ADDR_MSB_90;
#if(PWM0 == BRIGHTNESS_PWM)  //PWM0 OUTPUT
    Data[3] = 0x00;
    Data[4] = 0x01;
#elif(PWM1 == BRIGHTNESS_PWM) //PWM1 OUTPUT
    Data[3] = 0x40;
    Data[4] = 0x01;
#else                         //PWM2 OUTPUT
    Data[3] = 0x80;
    Data[4] = 0x01;
#endif


#if (INV_BRIGHTNESS)
//////////////////////////////////////////////////////////////////////////////////////////////////////
//anson				TP2804
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	#if (PANEL_TYPE == PANEL_CHIMEI)
		Data[5] = MIN_BRIGHTNESS + (unsigned int)(255 - 147) * (100 - stGUD0.BRIGHT) /100;	//anson Tp2804
	#elif (PANEL_TYPE == PANEL_HANNSTAR)
		Data[5] = MIN_BRIGHTNESS + (unsigned int)(255 - 147) * (100 - stGUD0.BRIGHT) /100;	//anson Tp2804
	#elif (PANEL_TYPE == PANEL_CPT)
		Data[5] = 35 + (unsigned int)(220 - 0) * stGUD0.BRIGHT /100;						//anson MTV512
	#elif (PANEL_TYPE == PANEL_QDI)
		Data[5] = 38 + (unsigned int)(217 - 0) * stGUD0.BRIGHT /100;						//anson MTV512
	#else //(PANEL_TYPE == PANEL_SHARP)
		Data[5] = MIN_BRIGHTNESS + (unsigned int)(255 - 143) * (100 - stGUD0.BRIGHT) /100;	//anson
	#endif
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
//anson				MTV512
//////////////////////////////////////////////////////////////////////////////////////////////////////
	#if (PANEL_TYPE == PANEL_CHIMEI)
		Data[5] = 38 + (unsigned int)(217 - 0) * stGUD0.BRIGHT /100;						//anson MTV512
	#elif (PANEL_TYPE == PANEL_HANNSTAR)
		Data[5] = 38 + (unsigned int)(217 - 0) * stGUD0.BRIGHT /100;						//anson MTV512
	#elif (PANEL_TYPE == PANEL_CPT)
		Data[5] = 35 + (unsigned int)(220 - 0) * stGUD0.BRIGHT /100;						//anson MTV512
	#elif (PANEL_TYPE == PANEL_QDI)
		Data[5] = 38 + (unsigned int)(217 - 0) * stGUD0.BRIGHT /100;						//anson MTV512
	#else //(PANEL_TYPE == PANEL_SHARP)
		Data[5] = MIN_BRIGHTNESS + (unsigned int)(255 - 143) * (100 - stGUD0.BRIGHT) /100;	//anson
	#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
#else
    Data[5] = MIN_BRIGHTNESS + (unsigned int)(MAX_BRIGHTNESS - MIN_BRIGHTNESS) * stGUD0.BRIGHT / 100;
#endif
    Data[6] = 0;


    RTDWrite(Data);
}

void Set_Gamma(void)
{
    switch (stGUD1.FUNCTION & 0x60)
    {
    case 0x20 :
        WriteGamma(GAMMA_1, GAMMA_1, GAMMA_1);
        break;
    case 0x40 :
        WriteGamma(GAMMA_2, GAMMA_2, GAMMA_2);
        break;
    case 0x60 :
        WriteGamma(GAMMA_3, GAMMA_3, GAMMA_3);
        break;
    default :
        RTDSetBit(COLOR_CTRL_5D, 0xeb, 0x00);   // Disable gamma function and its access channel
        break;
    }
}

void Set_Dithering(void)
{
#if (DISP_BIT == DISP_18BIT)

    WriteDither(DITHER_1,0);

#if(NEW_DITHER)
    WriteDither(NEW_DITHER_TABLE,NEW_DITHER);
#endif

#else


    WriteDither(DITHER_1,0);

#if(NEW_DITHER)
    WriteDither(NEW_DITHER_TABLE,NEW_DITHER);
#endif

    RTDSetBit(COLOR_CTRL_5D, 0x97, 0x00);       //  Disable dithering function and its access channel

#endif
}

void Sharpness(void)
{
    if (MODE_NOSIGNAL != ucMode_Curr && MODE_NOSUPPORT != ucMode_Curr)
    {

        // For RTD2020 rev.B and later
        RTDSetByte(FILTER_CTRL0_1B, 0xc4);

        switch (stGUD1.FILTER)
        {
        case 0 :
            WriteSU_COEF(SU_COEF_1, SU_COEF_1);
            break;
        case 1 :
            WriteSU_COEF(SU_COEF_2, SU_COEF_2);
            break;
        case 2 :
            if((usIPH_ACT_WID == 1280) && (DISP_SIZE == DISP_1280x1024))
			{
               WriteSU_COEF(SU_COEF_7, SU_COEF_7);
			}
            else
			{
               WriteSU_COEF(SU_COEF_3, SU_COEF_3);
			}

            break;
        case 3 :
            WriteSU_COEF(SU_COEF_4, SU_COEF_4);
            break;
        default :
            WriteSU_COEF(SU_COEF_5, SU_COEF_5);
            break;
        }

        RTDSetByte(FILTER_CTRL0_1B, 0xc7);
    }
}


#if(ANALOG_CONTRAST)

unsigned char Set_Contrast_Gain(unsigned char ContrastValue, unsigned char ColorValue)
{
     if(ContrastValue >= 45) //decrease gain
     {
	    if(ColorValue & 0x80)
		{
		   ColorValue &= 0x7f;
		   ColorValue = ColorValue + ContrastValue - 45;
		   if(ColorValue > 0x7f)
			  ColorValue  = 0xff; 	//(minus)
		   else
		      ColorValue |= 0x80;	//(minus)
		}
		else
		{
		    if(ColorValue > (ContrastValue - 45))
			    ColorValue = ColorValue - (ContrastValue - 45);
			else
			    ColorValue = 0x80 | (ContrastValue - 45 - ColorValue);
		}
	 }
     else  //increase gain
     {
        if(ColorValue & 0x80)
		{
		   ColorValue &= 0x7f;
		   if((45 - ContrastValue) > ColorValue)
		        ColorValue = 45 - ContrastValue - ColorValue;
		   else
		        ColorValue = 0x80 | (ColorValue - (45 - ContrastValue));
		   
		}
		else
			   ColorValue = ColorValue + (45 - ContrastValue);
     }
 
     return ColorValue;
}

#endif

void SetColorGainOffset(unsigned char addr,unsigned char parameter,unsigned char bios)
{
   	if(bios & 0x80)
	{	//(minus)
		bios &= 0x7f;
		RTDSetByte(addr, (parameter >= bios) ? parameter - bios : 0);
    }
    else
    {	//(plus)
		RTDSetByte(addr, (parameter >= (0xff - bios)) ? 0xff : parameter + bios);
    }   
}

void SetADC_Gain(void)
{
/*
#if (SWAP_RED_BLUE)
    RTDSetByte(REDGAIN_E0, stGUD2.AD_B_GAIN);
    RTDSetByte(GRNGAIN_E1, stGUD2.AD_G_GAIN);
    RTDSetByte(BLUGAIN_E2, stGUD2.AD_R_GAIN);
#else
    RTDSetByte(REDGAIN_E0, stGUD2.AD_R_GAIN);
    RTDSetByte(GRNGAIN_E1, stGUD2.AD_G_GAIN);
    RTDSetByte(BLUGAIN_E2, stGUD2.AD_B_GAIN);
#endif
*/

    unsigned char   ucTempContrast = 50;
    unsigned char   ucRate1  = (unsigned long)98 * usADC_Clock / usStdHS/4;
	unsigned char   ucRed = 0,ucGreen = 0,ucBlue = 0;


	if(110 < ucRate1)
	{
	    ucRed   =  0x80 | 29;
		ucGreen =  0x80 | 28;
		ucBlue  =  0x80 | 27;
	}
	else if(100 < ucRate1)
	{
   	    ucRed   =  0x80 | 10;
		ucGreen =  0x80 | 12;
		ucBlue  =  0x80 | 10;

	}
	else if(90 < ucRate1)
	{
   	    ucRed   =  1;
		ucGreen =  1;
		ucBlue  =  2;

	}
	else if(70 < ucRate1)
	{
   	    ucRed   =  6;
		ucGreen =  5;
		ucBlue  =  5;

	}
	else if(48 < ucRate1)
	{
	   ucRed   =  0;
	   ucGreen =  0;
	   ucBlue  =  0;
	}
	else if(38 < ucRate1)
	{
   	    ucRed   =  8;
		ucGreen =  9;
		ucBlue  =  8;

	}
	else
	{
   	    ucRed   =  2;
		ucGreen =  5;
		ucBlue  =  4;
	}

	
//    ucRate  = (100 >= ucRate) ? 0 : ucRate - 100;	
#if(ANALOG_CONTRAST)
    if (100 < stGUD0.CONTRAST)	stGUD0.CONTRAST = 50;
	ucTempContrast = (unsigned int)stGUD0.CONTRAST * 9 / 10;

    ucRed   = Set_Contrast_Gain(ucTempContrast,ucRed);
    ucGreen = Set_Contrast_Gain(ucTempContrast,ucGreen);
    ucBlue  = Set_Contrast_Gain(ucTempContrast,ucBlue); 


    //(stGUD2.AD_X_GAIN - ucRate + 50 - stGUD0.CONTRAST)
#endif

#if(SWAP_RED_BLUE)
    SetColorGainOffset(BLUGAIN_E2,stGUD2.AD_R_GAIN,ucRed);
#else
    SetColorGainOffset(REDGAIN_E0,stGUD2.AD_R_GAIN,ucRed);
#endif

    SetColorGainOffset(GRNGAIN_E1,stGUD2.AD_G_GAIN,ucGreen);

#if(SWAP_RED_BLUE)
    SetColorGainOffset(REDGAIN_E0,stGUD2.AD_B_GAIN,ucBlue);
#else
    SetColorGainOffset(BLUGAIN_E2,stGUD2.AD_B_GAIN,ucBlue);
#endif


}

void SetADC_Offset(void)
{
/*
#if (SWAP_RED_BLUE)
    RTDSetByte(REDOFST_E3, stGUD2.AD_B_OFFSET);
    RTDSetByte(GRNOFST_E4, stGUD2.AD_G_OFFSET);
    RTDSetByte(BLUOFST_E5, stGUD2.AD_R_OFFSET);
#else
    RTDSetByte(REDOFST_E3, stGUD2.AD_R_OFFSET);
    RTDSetByte(GRNOFST_E4, stGUD2.AD_G_OFFSET);
    RTDSetByte(BLUOFST_E5, stGUD2.AD_B_OFFSET);
#endif
*/
    unsigned char   ucRed = 0, ucBlue = 0, ucGreen = 0;
    unsigned char   ucRate  = (unsigned long)98 * usADC_Clock / usStdHS/4;


	if(110 < ucRate)
	{
	  ucRed = 10;
	  ucGreen = 11;
	  ucBlue = 11;
	}
	else if(90 < ucRate)
	{
      ucRed = 5;
	  ucGreen = 7;
	  ucBlue = 5;
	}
	else if(60 < ucRate)
	{
	  ucRed = 0;
	  ucGreen = 0;
	  ucBlue = 0;
	}
	else if(48 < ucRate)
	{
	  ucRed = 0x80 | 1;
	  ucGreen = 0;
	  ucBlue = 0x80 | 1;
	}
	else
	{
	  ucRed = 0;
	  ucGreen = 0x80 | 1;
	  ucBlue = 0x80 | 1;
	}
/*
    if (110 < ucRate)
    {
#if (SWAP_RED_BLUE)
        RTDSetByte(REDOFST_E3, stGUD2.AD_B_OFFSET <= 255 - 12 ? stGUD2.AD_B_OFFSET + 12 : 255);
        RTDSetByte(GRNOFST_E4, stGUD2.AD_G_OFFSET <= 255 - 12 ? stGUD2.AD_G_OFFSET + 12 : 255);
        RTDSetByte(BLUOFST_E5, stGUD2.AD_R_OFFSET <= 255 - 10 ? stGUD2.AD_R_OFFSET + 10 : 255);
#else
        RTDSetByte(REDOFST_E3, stGUD2.AD_R_OFFSET <= 255 - 10 ? stGUD2.AD_R_OFFSET + 10 : 255);
        RTDSetByte(GRNOFST_E4, stGUD2.AD_G_OFFSET <= 255 - 12 ? stGUD2.AD_G_OFFSET + 12 : 255);
        RTDSetByte(BLUOFST_E5, stGUD2.AD_B_OFFSET <= 255 - 12 ? stGUD2.AD_B_OFFSET + 12 : 255);
#endif
    }
	*/
   

#if(SWAP_RED_BLUE)
    SetColorGainOffset(BLUOFST_E5,stGUD2.AD_R_OFFSET,ucRed);
#else
    SetColorGainOffset(REDOFST_E3,stGUD2.AD_R_OFFSET,ucRed);
#endif

    SetColorGainOffset(GRNOFST_E4,stGUD2.AD_G_OFFSET,ucGreen);

#if(SWAP_RED_BLUE)
    SetColorGainOffset(REDOFST_E3,stGUD2.AD_B_OFFSET,ucBlue);
#else
    SetColorGainOffset(BLUOFST_E5,stGUD2.AD_B_OFFSET,ucBlue);
#endif


}

void SetADC_GainOffset(void)
{
    SetADC_Gain();
    SetADC_Offset();
}
#if(VIDEO_CHIP != VDC_NONE)
void SetVDC_Color(void)
{
    if (SOURCE_YUV == (stGUD1.INPUT_SOURCE & 0x07))
    {
        Data[0] = 6;
        Data[1] = ADDR_VIDEO;
        Data[2] = VDC_BRIGHT_YUV_CTRL;
        Data[3] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VBRIGHT;
        Data[4] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VCONTRAST   ^ 0x80;
        Data[5] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VSATURATION ^ 0x80;
        I2CWrite(Data);
    }
    else
    {
        Data[0] = 7;
        Data[1] = ADDR_VIDEO;
        Data[2] = VDC_BRIGHT_CTRL;
        Data[3] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VBRIGHT;
        Data[4] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VCONTRAST   ^ 0x80;
        Data[5] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VSATURATION ^ 0x80;
        Data[6] = ((LP_VIDEO_MODE_USER_DATA)&stMUD)->VHUE        ^ 0x80;
        I2CWrite(Data);
    }
}
#endif
//mega #if(MCU_TYPE == MCU_WINBOND)		//anson 050519
/*
void Wait_For_Event(unsigned char event)
{
	unsigned char t;

	t = 100;

	RTDSetByte(0x1f,0x00);
   	do
	{
	   	RTDRead(STATUS1_1F, 1, Y_INC);
		Data[0] = Data[0] & event;
		t--;
		Delay_Xms(1);
   	}while((Data[0] == 0) && (t));

    RTDSetByte(0x1f,0x00);

}
*/

void Wait_For_Event(unsigned char event)
{
    unsigned char   ucDelayCnt  = 80;   // 80ms timeout
    unsigned char   ucProtect   = 24;   // 24ms protect

	RTDSetByte(STATUS1_1F,0x00);          // Clear status (status register will be cleared after write)
    
    bNotify_Timer1_Int  = 0;
    
    Data[0] = 0;
    Data[1] = 0;
    TR1     = 1;
    do
    {
        if (bNotify_Timer1_Int)
        {
            bNotify_Timer1_Int  = 0;

            if (Data[1] & (EVENT_IVS | EVENT_IEN_START))
            {
                Data[1]     = 0;
                ucProtect   = 24;
            }
            else if (ucProtect)
            {
                ucProtect   = ucProtect - 1;
            }

            if (--ucDelayCnt)   TR1 = 1;
        }


        RTDRead(STATUS1_1F, 1, N_INC);  // Read Status1 


		if(Data[0])
			RTDSetByte(STATUS1_1F,0x00);

        
        Data[1] |= (Data[0] & (EVENT_IVS | EVENT_IEN_START));
        Data[0] &= event;

    }
    while (0 == Data[0] && 0 != ucDelayCnt);
}

#if 0  //mega		//anson 050519
//#else
void Wait_For_Event(unsigned char event)
{
    unsigned char   ucDelayCnt  = 80;   // 80ms timeout
    unsigned char   ucProtect   = 24;   // 24ms protect

	RTDSetByte(STATUS1_1F,0x00);          // Clear status (status register will be cleared after write)
    
    bNotify_Timer1_Int  = 0;
    
    Data[0] = 0;
    Data[1] = 0;
//  TR1     = 1;
    do
    {
        if (bNotify_Timer1_Int)
        {
            bNotify_Timer1_Int  = 0;

            if (Data[1] & (EVENT_IVS | EVENT_IEN_START))
            {
                Data[1]     = 0;
                ucProtect   = 24;
            }
            else if (ucProtect)
            {
                ucProtect   = ucProtect - 1;
            }

            if (--ucDelayCnt)   TR0 = 1;//TR1 = 1;
        }


        RTDRead(STATUS1_1F, 1, N_INC);  // Read Status1 


		if(Data[0])
			RTDSetByte(STATUS1_1F,0x00);

        
        Data[1] |= (Data[0] & (EVENT_IVS | EVENT_IEN_START));
        Data[0] &= event;

		/*
        if (bAutoInProgress && (event & EVENT_INPUT) && 0 == ucProtect)
        {
#if (TYPE_OF_8051 == INT_8051)
#if (LIGHT_OFF)
            SetPortBit(LIGHT_PWR_PORT, 0xff, LIGHT_PWR_MASK);
#else
            SetPortBit(LIGHT_PWR_PORT, ~LIGHT_PWR_MASK, 0);
#endif
#else
            bLIGHT_PWR  = LIGHT_OFF;
#endif
        }
		*/
    }
    while (0 == Data[0] && 0 != ucDelayCnt);
}

#endif//mega
///////////////////////////////////////////////////////////////////////////////////////////////

void Save_GUD0(void)
{
    Data[0]     = 11;
    Data[1]     = ADDR_EROM1;
    Data[2]     = 0xE0;
    Data[3]     = stGUD0.CONTRAST;
    Data[4]     = stGUD0.BRIGHT;
    Data[5]     = stGUD0.RTD_R_CONTRAST;
    Data[6]     = stGUD0.RTD_G_CONTRAST;
    Data[7]     = stGUD0.RTD_B_CONTRAST;
    Data[8]     = stGUD0.RTD_R_BRIGHT;
    Data[9]     = stGUD0.RTD_G_BRIGHT;
    Data[10]    = stGUD0.RTD_B_BRIGHT;
    I2CWrite(Data);

    Delay_Xms(SET_2404_DELAY);
}

void Load_GUD0(void)
{
    I2CRead(ADDR_EROM1, 0xE0 , 8);

    stGUD0.CONTRAST         = Data[0];
    stGUD0.BRIGHT           = Data[1];
    stGUD0.RTD_R_CONTRAST   = Data[2];
    stGUD0.RTD_G_CONTRAST   = Data[3];
    stGUD0.RTD_B_CONTRAST   = Data[4];
    stGUD0.RTD_R_BRIGHT     = Data[5];
    stGUD0.RTD_G_BRIGHT     = Data[6];
    stGUD0.RTD_B_BRIGHT     = Data[7];
}

void Save_GUD1(void)
{
    Data[0]     = 10;
    Data[1]     = ADDR_EROM1;
    Data[2]     = 0xE8;
    Data[3]     = stGUD1.FUNCTION;
    Data[4]     = stGUD1.INPUT_SOURCE;
    Data[5]     = stGUD1.FILTER;
    Data[6]     = stGUD1.OSD_POSH;
    Data[7]     = stGUD1.OSD_POSV;
    Data[8]     = stGUD1.OSD_TIMEOUT;
    Data[9]     = stGUD1.OSD_INPUT;			//anson 05_0314
    Data[10]     = 0;						//anson 05_0314
    I2CWrite(Data);

    Delay_Xms(SET_2404_DELAY);
}

void Load_GUD1(void)
{
    I2CRead(ADDR_EROM1, 0xE8 , 7);
    
	stGUD1.FUNCTION			= Data[0];
	stGUD1.INPUT_SOURCE	= Data[1];
	stGUD1.FILTER			= Data[2];
	stGUD1.OSD_POSH		= Data[3];
	stGUD1.OSD_POSV		= Data[4];
	stGUD1.OSD_TIMEOUT		= Data[5];
	stGUD1.OSD_INPUT 		= Data[6];		//anson 05_0314

}

void Save_GUD2(void)
{
    Data[0]     = 9;
    Data[1]     = ADDR_EROM1;
    Data[2]     = 0xF0;
    Data[3]     = stGUD2.AD_R_GAIN;
    Data[4]     = stGUD2.AD_G_GAIN;
    Data[5]     = stGUD2.AD_B_GAIN;
    Data[6]     = stGUD2.AD_R_OFFSET;
    Data[7]     = stGUD2.AD_G_OFFSET;
    Data[8]     = stGUD2.AD_B_OFFSET;

    I2CWrite(Data);
    Delay_Xms(SET_2404_DELAY);
}

void Load_GUD2(void)
{
    I2CRead(ADDR_EROM1, 0xF0 , 6);
    
    stGUD2.AD_R_GAIN    = Data[0];
    stGUD2.AD_G_GAIN    = Data[1];
    stGUD2.AD_B_GAIN    = Data[2];
    stGUD2.AD_R_OFFSET  = Data[3];
    stGUD2.AD_G_OFFSET  = Data[4];
    stGUD2.AD_B_OFFSET  = Data[5];
}

void Save_GUD3(void)
{
    Data[0]     = 7;
    Data[1]     = ADDR_EROM1;
    Data[2]     = 0xF8;
    Data[3]     = stGUD3.VOLUME;
    Data[4]     = stGUD3.CURR_CHANNEL;
    Data[5]     = stGUD3.PREV_CHANNEL;
    Data[6]     = stGUD3.TV_SETTING;

    I2CWrite(Data);
    Delay_Xms(SET_2404_DELAY);
}

void Load_GUD3(void)
{
    I2CRead(ADDR_EROM1, 0xF8 , 4);
    
    stGUD3.VOLUME       = Data[0];
    stGUD3.CURR_CHANNEL = Data[1];
    stGUD3.PREV_CHANNEL = Data[2];
    stGUD3.TV_SETTING   = Data[3];
}

void Save_GUD4(void)
{
    Data[0]     = 12;
    Data[1]     = ADDR_EROM1;
    Data[2]     = 0xD0;
    Data[3]     = stGUD4.C1_R;
    Data[4]     = stGUD4.C1_G;
    Data[5]     = stGUD4.C1_B;
    Data[6]     = stGUD4.C2_R;
    Data[7]     = stGUD4.C2_G;
    Data[8]     = stGUD4.C2_B;
    Data[9]     = stGUD4.C3_R;
    Data[10]    = stGUD4.C3_G;
    Data[11]    = stGUD4.C3_B;
    I2CWrite(Data);

    Delay_Xms(SET_2404_DELAY);
}

void Load_GUD4(void)
{
    I2CRead(ADDR_EROM1, 0xD0 , 9);

    stGUD4.C1_R   = Data[0];
    stGUD4.C1_G   = Data[1];
    stGUD4.C1_B   = Data[2];
    stGUD4.C2_R   = Data[3];
    stGUD4.C2_G   = Data[4];
    stGUD4.C2_B   = Data[5];
    stGUD4.C3_R   = Data[6];
    stGUD4.C3_G   = Data[7];
	stGUD4.C3_B   = Data[8];
}


void Save_MUD(unsigned char mode_num)
{
    if (0 == mode_num || 64 < mode_num)  return;
    
    Data[0] = 7;
    Data[1] = ADDR_EROM0;
    Data[2] = (mode_num - 1) << 2;	        
    Data[3] = stMUD.H_POSITION;
    Data[4] = stMUD.V_POSITION;
    Data[5] = stMUD.CLOCK;
    Data[6] = (stMUD.PHASE & 0x7c);// | (stMUD.P_Code - 0x19);
    I2CWrite(Data);
    
    Delay_Xms(SET_2404_DELAY);
}

void Load_MUD(unsigned char mode_num)
{
    if (0 == mode_num || 64 < mode_num)  return;

    I2CRead(ADDR_EROM0, (mode_num - 1) << 2, 4);

    stMUD.H_POSITION    = Data[0];
    stMUD.V_POSITION    = Data[1];
    stMUD.CLOCK         = Data[2];
    stMUD.PHASE         = Data[3] & 0x7c;
//	stMUD.P_Code        = (Data[3] & 0x03) + 0x19;

	
}

void Init_GUD(void)     // GU <= Default
{
    stGUD0.CONTRAST         = INIT_EEPROM0[3];
    stGUD0.BRIGHT           = INIT_EEPROM0[4];
    stGUD0.RTD_R_CONTRAST   = INIT_EEPROM0[5];
    stGUD0.RTD_G_CONTRAST   = INIT_EEPROM0[6];
    stGUD0.RTD_B_CONTRAST   = INIT_EEPROM0[7];
    stGUD0.RTD_R_BRIGHT     = INIT_EEPROM0[8];
    stGUD0.RTD_G_BRIGHT     = INIT_EEPROM0[9];
    stGUD0.RTD_B_BRIGHT     = INIT_EEPROM0[10];

    stGUD1.FUNCTION         = INIT_EEPROM0[11];
    stGUD1.INPUT_SOURCE     = INIT_EEPROM0[12];
    stGUD1.FILTER           = INIT_EEPROM0[13];
    stGUD1.OSD_POSH         = INIT_EEPROM0[14];
    stGUD1.OSD_POSV         = INIT_EEPROM0[15];
    stGUD1.OSD_TIMEOUT      = INIT_EEPROM0[16];
    stGUD1.OSD_INPUT      = INIT_EEPROM0[17];		//anson 05_0314

    stGUD2.AD_R_GAIN        = INIT_EEPROM1[3];
    stGUD2.AD_G_GAIN        = INIT_EEPROM1[4];
    stGUD2.AD_B_GAIN        = INIT_EEPROM1[5];
    stGUD2.AD_R_OFFSET      = INIT_EEPROM1[6];
    stGUD2.AD_G_OFFSET      = INIT_EEPROM1[7];
    stGUD2.AD_B_OFFSET      = INIT_EEPROM1[8];

    stGUD3.VOLUME           = INIT_EEPROM1[11];
    stGUD3.CURR_CHANNEL     = INIT_EEPROM1[12];
    stGUD3.PREV_CHANNEL     = INIT_EEPROM1[13];
    stGUD3.TV_SETTING       = INIT_EEPROM1[14];

	stGUD4.C1_R             = INIT_EEPROM2[3];
	stGUD4.C1_G             = INIT_EEPROM2[4];
	stGUD4.C1_B             = INIT_EEPROM2[5];
	stGUD4.C2_R             = INIT_EEPROM2[6];
	stGUD4.C2_G             = INIT_EEPROM2[7];
	stGUD4.C2_B             = INIT_EEPROM2[8];
	stGUD4.C3_R             = INIT_EEPROM2[9];
	stGUD4.C3_G             = INIT_EEPROM2[10];
	stGUD4.C3_B             = INIT_EEPROM2[11];

	
    I2CWrite(INIT_EEPROM0);
    Delay_Xms(SET_2404_DELAY);

    I2CWrite(INIT_EEPROM1);
    Delay_Xms(SET_2404_DELAY);

	I2CWrite(INIT_EEPROM2);
    Delay_Xms(SET_2404_DELAY);

//	Save_GUD4();

}

void Init_MUD(void)
{
    unsigned char   ucModeIdx;

	//anson 05_0314
	Data[0]     = 11;
	Data[1]     = ADDR_EROM1;
	Data[2]     = 0;
	Data[3]     = 0xff;
	Data[4]     = 0xff;
	Data[5]     = 0xff;
	Data[6]     = 0xff;
	Data[7]     = 0xff;
	Data[8]     = 0xff;
	Data[9]     = 0xff;
	Data[10]    = 0xff;
	I2CWrite(Data);
	Delay_Xms(SET_2404_DELAY);
/*
    // Reset frame-Sync and TV channel settings
    Data[0]     = 11;
    Data[1]     = ADDR_EROM1;
    Data[2]     = 0;
    Data[3]     = 0x00;
    Data[4]     = 0x00;
    Data[5]     = 0x00;
    Data[6]     = 0x00;
    Data[7]     = 0x00;
    Data[8]     = 0x00;
    Data[9]     = 0x00;
    Data[10]    = 0x00;
    
    ucModeIdx   = 0;
    do
    {
        Data[2]     = ucModeIdx << 3;
        ucModeIdx   = ucModeIdx + 1;

        I2CWrite(Data);
        Delay_Xms(SET_2404_DELAY);
    }
    while (26 > ucModeIdx);
*/
    // Reset display settings
    Data[0]     = 7;
    Data[1]     = ADDR_EROM0;
    Data[2]     = 0;
    Data[3]     = 0x80; // stMUD.H_POSITION;
    Data[4]     = 0x80; // stMUD.V_POSITION;
    Data[5]     = 0x80; // stMUD.CLOCK;
    Data[6]     = 0x01; // stMUD.PHASE;

    ucModeIdx   = 0;
    do
    {
        Data[2]     = ucModeIdx << 2;
        ucModeIdx   = ucModeIdx + 1;

        switch (ucModeIdx)
        {
        case MODE_YUV60HZ :
            Data[3] = 0xbc;         // VBRIGHT
            Data[4] = 0x65 ^ 0x80;  // VCONTRAST
            Data[5] = 0x64 ^ 0x80;  // VSATURATION
            Data[6] = 0x00 ^ 0x80;  // VHUE
            break;
        case MODE_YUV50HZ :
            Data[3] = 0xb1;         // VBRIGHT
            Data[4] = 0x5f ^ 0x80;  // VCONTRAST
            Data[5] = 0x65 ^ 0x80;  // VSATURATION
            Data[6] = 0x00 ^ 0x80;  // VHUE
            break;
#if (VIDEO_CHIP == VDC_SAA7118)
        case MODE_VIDEO60HZ :
            Data[3] = 0x96;         // VBRIGHT
            Data[4] = 0x48 ^ 0x80;  // VCONTRAST
            Data[5] = 0x4a ^ 0x80;  // VSATURATION
            Data[6] = 0xff ^ 0x80;  // VHUE
            break;
        case MODE_VIDEO50HZ :
            Data[3] = 0x83;         // VBRIGHT
            Data[4] = 0x48 ^ 0x80;  // VCONTRAST
            Data[5] = 0x43 ^ 0x80;  // VSATURATION
            Data[6] = 0x00 ^ 0x80;  // VHUE
            break;
#else
        case MODE_VIDEO60HZ :
            Data[3] = 0x95;         // VBRIGHT
            Data[4] = 0x47 ^ 0x80;  // VCONTRAST
            Data[5] = 0x48 ^ 0x80;  // VSATURATION
            Data[6] = 0xff ^ 0x80;  // VHUE
            break;
        case MODE_VIDEO50HZ :
            Data[3] = 0x82;         // VBRIGHT
            Data[4] = 0x47 ^ 0x80;  // VCONTRAST
            Data[5] = 0x42 ^ 0x80;  // VSATURATION
            Data[6] = 0x00 ^ 0x80;  // VHUE
            break;
#endif
        }
        
        I2CWrite(Data);
        Delay_Xms(SET_2404_DELAY);
    }
    while (64 > ucModeIdx);
}

void Check_EEPROM(void)
{
    I2CRead(ADDR_EROM1, 0xFE , 2);
    
    if ((INIT_EEPROM1[17] != Data[0]) || (INIT_EEPROM1[18] != Data[1]))
    {
        Delay_Xms(SET_2404_DELAY);

        Init_GUD();
        Init_MUD();
    }
    else
    {
        Load_GUD0();            // Read Global User Data 0 from EEPROM 2404
        Load_GUD1();            // Read Global User Data 1 from EEPROM 2404
        Load_GUD2();            // Read Global User Data 2 from EEPROM 2404
        Load_GUD3();            // Read Global User Data 3 from EEPROM 2404
		Load_GUD4();            // Read Global User Data 4 from EEPROM 2404
    }
}

void Free_Background(void)  // Force to FreeRun & Background
{
    RTDCodeW(FreeV);                            // Switch to free-running mode

    //if (PANEL_OFF == bPANEL_PWR)
    if ( _OFF == bPanel_Status)
    {

    RTDSetByte(VDIS_SIGINV_21, 0x00 | DISP_EO_SWAP | DISP_RB_SWAP | DISP_ML_SWAP);       // DHS, DVS, DEN, DCLK MUST NOT be inverted.
    RTDSetBit(VDIS_CTRL_20, 0xfd, 0x01);    // DHS, DVS, DEN, DCLK and data are clamped to 0
    }

    RTDSetBit(VGIP_CTRL_04, 0xfe, 0x00);        // Stop sampling input pixels


    RTDSetBit(VDIS_SIGINV_21, 0x0f, SOURCE_TV == (stGUD1.INPUT_SOURCE & 0x07) ? 0x10 | DISP_EO_SWAP | DISP_RB_SWAP | DISP_ML_SWAP  : 
	                                                                    0x00 | DISP_EO_SWAP | DISP_RB_SWAP | DISP_ML_SWAP);

}


void Reset_Mode(void)
{  
    unsigned char ucTimeout = 120;

#if(MCU_TYPE == MCU_WINBOND)
    bLIGHT_PWR  = LIGHT_OFF;        // Turn off BackLight for reset display
#else
    MCU_WriteBacklightPower(LIGHT_OFF);        // Turn off BackLight for reset display
#endif


    RTDSetByte(HOSTCTRL_02, 0x40);  // Wake RTD up

#if(FIX_LAST_DHT)
    RTDSetByte(FX_LST_LEN_H_5A,0x00); //Disable the Fixed DVTOTAL & Last Line Lenghth Fucntion
#endif


#if(SPREAD_SPECTRUM)
  RTDSetBit(SPREAD_SPECTRUM_99,0x0f,0x00); //Disable spread spectrum  
#endif
#if(AS_NON_FRAMESYNC)
    RTDSetBit(ODD_CTRL_8E,0xef,0x00);
#endif

#if(AS_PLL_NONLOCK)
    RTDSetBit(ODD_CTRL_8E,0xdf,0x00);
#endif
#if(AS_DV_TOTAL)
    RTDSetBit(DV_BKGD_STA_31,0x7f,0x00);
#endif

#if(TMDS_ENABLE)
   RTDSetBit(TMDS_CORRECTION_FF,0xfc,0x00);
#endif

RTDSetByte(STATUS0_01, 0x00);  // Clear status
RTDSetByte(STATUS1_1F, 0x00);  // Clear status
RTDSetByte(DV_TOTAL_STATUS_3D,0x00);



    Free_Background();
    
    RTDOSDW(OSD_Reset);            // Clear OSD
	RTDSetBit(OVL_CTRL_6D, 0xfe, 0x00);	// Disable overlay control

#if (TUNE_APLL)
    RTDSetBit(DV_TOTAL_STATUS_3D, 0xdf, 0x00);//Disable PE Max Measurement
    //RTDSetBit(DV_TOTAL_STATUS_3D, 0x7f,0x40); //clear PE Max value
	RTDSetByte(DV_TOTAL_STATUS_3D,0x40); //clear PE Max value
    do
    {
         RTDRead(DV_TOTAL_STATUS_3D, 1, N_INC);
         Delay_Xms(1); 
		 
    }while( --ucTimeout && ((Data[0] & 0x40 ) == 0x40));
	ucPE_Max = 0;
	ucPE_Level = 0;
#endif


    ucMode_Curr     = MODE_NOSIGNAL;
    ucMode_Found    = MODE_NOSUPPORT;

    ucMode_Times    = 0;
    ucAV_Mode       = 0;
    bStable         = 0;
    bReload         = 1;
    bFrameSync      = 0;
	ucPE_Max        = 0;
	ucI_Code        = 0;
    bOverSpec       = 0;

    if (SOURCE_AV == ucInputSrc || SOURCE_SV == ucInputSrc || SOURCE_TV == ucInputSrc)
    {
        RTDCodeW(VIDEO_INI);
        I2CWrite((SOURCE_SV == ucInputSrc) ? SV_DETECT : AV_DETECT);

        if (SOURCE_TV == ucInputSrc)
        {
            // Select TV signal input pin and disable AGC of video decoder
            I2CWrite(TV_SOURCE_SEL);
            I2CWrite(TV_SCAN_GAIN);
        }
    }
#if (VIDEO_CHIP == VDC_SAA7118)
    else if (SOURCE_YUV == ucInputSrc)
    {
        RTDCodeW(VIDEO_INI);
        I2CWrite(YUV_DETECT);
    }
#endif

}


#if (TV_CHIP != TV_NONE)

void Set_TV_Channel()
{
    if (0 == stGUD3.CURR_CHANNEL)
    {
        stGUD3.CURR_CHANNEL = 1;
    }
    else if (MAX_CATV_NUM < stGUD3.CURR_CHANNEL)
    {
        stGUD3.CURR_CHANNEL = MAX_CATV_NUM;
    }
    else if (0 == (stGUD3.TV_SETTING & 0x01) && MAX_AIR_NUM < stGUD3.CURR_CHANNEL)
    {
        stGUD3.CURR_CHANNEL = MAX_AIR_NUM;
    }

    ((unsigned int *)Data)[1]   = (stGUD3.TV_SETTING & 0x01) ? CATV_Freq[stGUD3.CURR_CHANNEL - 1] : AIR_Freq[stGUD3.CURR_CHANNEL - 1];

#if (TV_CHIP == TV_FI1236)

    Data[0] = 6;
    Data[1] = ADDR_TUNER;
    Data[4] = 0xce;
    Data[5] = (VHF_LOW_FREQ > (((unsigned int *)Data)[1] / 16)) ? 0xa0 : (VHF_HIGH_FREQ > (((unsigned int *)Data)[1] / 16)) ? 0x90 : 0x30;

    I2CWrite(Data);

#endif

#if (TV_CHIP == TV_FQ1216)

    Data[0] = 6;
    Data[1] = ADDR_TUNER;
    Data[4] = 0x8e;
    Data[5] = (VHF_LOW_FREQ > (((unsigned int *)Data)[1] / 16)) ? 0xa1 : (VHF_HIGH_FREQ > (((unsigned int *)Data)[1] / 16)) ? 0x91 : 0x31;

    I2CWrite(Data);  

#endif
}

void Prev_Channel()
{
    if (0 == stGUD3.CURR_CHANNEL)   stGUD3.CURR_CHANNEL = 1;

    stGUD3.PREV_CHANNEL = stGUD3.CURR_CHANNEL;

    Data[1] = 0;

    while (1)
    {
        if (1 == stGUD3.CURR_CHANNEL)
            stGUD3.CURR_CHANNEL = (stGUD3.TV_SETTING & 0x01) ? MAX_CATV_NUM : MAX_AIR_NUM;
        else
            stGUD3.CURR_CHANNEL = stGUD3.CURR_CHANNEL - 1;

        if (stGUD3.PREV_CHANNEL == stGUD3.CURR_CHANNEL)     break;

        Data[2] = stGUD3.CURR_CHANNEL - 1;
        Data[3] = 1 << (7 - (Data[2] & 0x07));
        Data[4] = 0xd0 + (Data[2] >> 3);

        if (Data[4] != Data[1])
        {
            Data[1] = Data[4];
            I2CRead(ADDR_EROM1, Data[4], 1);
        }

        if (Data[0] & Data[3])
        {
            Set_TV_Channel();
            Save_GUD3();
            break;
        }
    }
}

void Next_Channel()
{
    if (0 == stGUD3.CURR_CHANNEL)   stGUD3.CURR_CHANNEL = 1;

    stGUD3.PREV_CHANNEL = stGUD3.CURR_CHANNEL;

    Data[1] = 0;

    while (1)
    {
        if (((0 == (stGUD3.TV_SETTING & 0x01)) && (MAX_AIR_NUM == stGUD3.CURR_CHANNEL)) || MAX_CATV_NUM == stGUD3.CURR_CHANNEL)
            stGUD3.CURR_CHANNEL = 1;
        else
            stGUD3.CURR_CHANNEL = stGUD3.CURR_CHANNEL + 1;

        if (stGUD3.PREV_CHANNEL == stGUD3.CURR_CHANNEL)     break;

        Data[2] = stGUD3.CURR_CHANNEL - 1;
        Data[3] = 1 << (7 - (Data[2] & 0x07));
        Data[4] = 0xd0 + (Data[2] >> 3);

        if (Data[4] != Data[1])
        {
            Data[1] = Data[4];
            I2CRead(ADDR_EROM1, Data[4], 1);
        }

        if (Data[0] & Data[3])
        {
            Set_TV_Channel();
            Save_GUD3();
            break;
        }
    }
}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THE CODE BELOW IS ONLY FOR DEBUG
// RTD_Get_Set()    - For ICE
// RTD_Test()       - For KINGMICE
// OSD_Show_Check() - Display information on OSD
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (GETREGISTER)
void RTD_Get_Set(void)
{
    if (ucGETCHIP)
    {
        if (ucGETCHIP & 0xf0)   // write
        {   
            Data[0] = 4;
            Data[2] = ucGETADDR;
            Data[3] = ucGETDATA0;
            Data[4] = 0;
            
            switch (ucGETCHIP & 0x0f)
            {
            case 01 : //RTD
                Data[1] = N_INC;
                RTDWrite(Data);
                break;
            case 02 : //ADC
                Data[1] = ADDR_ADC;
                I2CWrite(Data);  
                break;
            case 03 : //VIDEO
                Data[1] = ADDR_VIDEO;
                I2CWrite(Data);  
                break;
            case 04 : //EEPROM0
                Data[1] = ADDR_EROM0;
                I2CWrite(Data);
                break;
            case 05 : //EEPROM1
                Data[1] = ADDR_EROM1;
                I2CWrite(Data);
                break;
            default:
                break;
            }
        }
        else    // read
        {
            switch (ucGETCHIP)
            {
            case 01 : //RTD
                RTDRead(ucGETADDR, 2, Y_INC);
                break;
            case 02 : //ADC
                I2CRead(ADDR_ADC, ucGETADDR , 2);
                break;
            case 03 : //VIDEO
                I2CRead(ADDR_VIDEO, ucGETADDR , 2);
                break;
            case 04 : //EEPROM0
                I2CRead(ADDR_EROM0, ucGETADDR , 2);
                break;
            case 05 : //EEPROM1
                I2CRead(ADDR_EROM1, ucGETADDR , 2);
                break;
            default:
                break;
            }
            ucGETDATA0  = Data[0];
            ucGETDATA1  = Data[1];
        }
        ucGETCHIP   = 0;
    }
}
#endif

#if (KINGMICE)

void RTD_Test(void)
{
    static unsigned char idata ucI2C_Addr;
    static unsigned char idata ucI2C_Data;
    static unsigned char idata ucRTD_Data;

    do
    {
        while (ucROM_A4 & 0x01)             // if action,
        {
            if (ucROM_A4 & 0x02)            // read   
            {                            
                if (ucROM_A4 & 0x10)        // if IIC_index        
                {       
                    I2CRead(ucI2C_Addr, ucROM_A3, 0x01);
                    ucPRN_7 = Data[0];
                }
                else if (ucROM_A4 & 0x40)   // if RTD_index 
                {
                    RTDRead(ucROM_A3, 0x01, N_INC);                   
                    ucPRN_7 = Data[0];
                }
                else if (ucROM_A4 & 0x80)   // if 8051_index 
                {
                    ucPRN_7 = *((unsigned char *)ucROM_A3);
                }
            }
            else                            // write
            {                            
                if (ucROM_A4 & 0x04)        // if IIC_addr
                {
                    ucI2C_Addr  = ucROM_A3;
                }
                else if (ucROM_A4 & 0x08)   // if IIC_data
                {
                    ucI2C_Data  = ucROM_A3;
                }
                else if (ucROM_A4 & 0x10)   // if IIC_index 
                {                       
                    Data[0] = 4;
                    Data[1] = ucI2C_Addr;
                    Data[2] = ucROM_A3;
                    Data[3] = ucI2C_Data;
                    I2CWrite(Data);
                }
                else if (ucROM_A4 & 0x20)   // if RTD_data 
                {
                    ucRTD_Data  =  ucROM_A3;
                }
                else if (ucROM_A4 & 0x40)   // if RTD_index 
                {                                       
                    Data[0] = 4;
                    Data[1] = N_INC;
                    Data[2] = ucROM_A3;
                    Data[3] = ucRTD_Data;
                    Data[4] = 0;
                    RTDWrite(Data);
                }                
                else if (ucROM_A4 & 0x80)   // if 8051_index 
                {                                      
                    *((unsigned char *)ucROM_A3)    = Data[0];
                }                 
            } 
            
            if (ucROM_A5)   { }         // clear action 
        }                               
    }
    while (ucROM_A0 & 0x80);
}



#else

void RTD_Test(void)
{

#if(ISPACK)
   static unsigned char idata  ucAddr, ucValue;

    unsigned char   ucStatus, ucStop, DDC_SUB_IN, DDC_DATA_IN;
	unsigned char idata *MEM_MAP;

//	Data[0] = (unsigned char)(usHsync >> 8);
//	Data[1] = (unsigned char)usHsync;
//	Data[2] = (unsigned char)(usVsync >> 8);
//	Data[3] = (unsigned char)usVsync;
//	Data[4] = stGUD1.OSD_POSV;
//	Data[5] = ucV_Min_Margin;
//	Data[6] = stMUD.V_POSITION;
//	Data[7] = (unsigned char)(usIPV_ACT_STA >> 8);
//	Data[8] = (unsigned char)(usIPV_ACT_STA);

    ucStop  = 0;
    do
    {
	    RTDRead(DDC_STATUS_F4, 1,N_INC);

        //ucStatus    = DDC_STATUS;       // read DDC_STATUS;
		ucStatus    = Data[0];       // read DDC_STATUS;
        
        if (ucStatus & 0x04)            // DDC_DATA_IN latched
        {
		    RTDRead(DDC_SUB_IN_F1, 2, Y_INC);
			DDC_SUB_IN = Data[0];
			DDC_DATA_IN = Data[1];

            RTDSetByte(DDC_STATUS_F4,0x00);  //Write once to clear status

            if (DDC_SUB_IN & 0x80)      // run/stop command		
            {
                ucStop = DDC_DATA_IN;
				
            }
            else if (DDC_SUB_IN & 0x40) // read command
            {
                switch (DDC_SUB_IN & 0x0f)
                {
                case 0x01 :
                    //RTD_ADDR_PORT   = DDC_DATA_IN;
                    //DDC_DATA_OUT    = RTD_DATA_PORT;	
					RTDRead(DDC_DATA_IN, 1, N_INC);
					RTDSetByte(DDC_DATA_OUT_F3, Data[0]);
                    break;

//#if(ISPACK) 
//               case 0x02 :
//			        ucValue = DDC_DATA_IN;
//			        DDC_DATA_OUT    = Data[ucValue];
//			   break;
//#endif

                case 0x04 :
                    I2CRead(ucAddr, DDC_DATA_IN, 1);
                    //DDC_DATA_OUT    = Data[0];
                    RTDSetByte(DDC_DATA_OUT_F3, Data[0]);
                    break;
                case 0x05 :
                    //DDC_DATA_OUT    = *((unsigned char *)DDC_DATA_IN);
					MEM_MAP = DDC_DATA_IN;
					RTDSetByte(DDC_DATA_OUT_F3, *MEM_MAP);
                    break;
                case 0x07 :
                    switch (DDC_DATA_IN)
                    {
                    case 0x80:
                        //DDC_DATA_OUT    = P0;
						RTDSetByte(DDC_DATA_OUT_F3, P0);
                        break;
                    case 0x90:
                        //DDC_DATA_OUT    = P1;
						RTDSetByte(DDC_DATA_OUT_F3, P1);
                        break;
                    case 0xa0:
                        //DDC_DATA_OUT    = P2;
						RTDSetByte(DDC_DATA_OUT_F3, P2);
                        break;
                    case 0xb0:
                        //DDC_DATA_OUT    = P3;
						RTDSetByte(DDC_DATA_OUT_F3, P3);
                        break;                    
#if(GETSFR)
					default:
					    //DDC_DATA_OUT    = GetSFRPort(DDC_DATA_IN);
						RTDSetByte(DDC_DATA_OUT_F3, GetSFRPort(DDC_DATA_IN));
						break;
#endif
                    }
                    
                }
            }
            else
            {
                switch (DDC_SUB_IN & 0x0f)
                {
                case 0x00 :
                    //RTD_ADDR_PORT   = ucAddr;
                    //RTD_DATA_PORT   = DDC_DATA_IN;
					RTDSetByte(ucAddr,DDC_DATA_IN);
                    break;
                case 0x01 :
                case 0x02 :
                    ucAddr  = DDC_DATA_IN;
                    break;
                case 0x03 :
                case 0x06 :
                case 0x08 :
                    ucValue = DDC_DATA_IN;
                    break;
                case 0x04 :
                    Data[0] = 4;
                    Data[1] = ucAddr;
                    Data[2] = DDC_DATA_IN;
                    Data[3] = ucValue;
                    I2CWrite(Data);
                    break;
                case 0x05 :
                    MEM_MAP = DDC_DATA_IN;

                    break;
                case 0x07 :
				    ucAddr = DDC_DATA_IN;
                    switch (ucAddr)
                    {
                    case 0x80 :
                        //SetPortBit(0, 0, ucValue);  
					    P0  = ucValue;
                        break;
                    case 0x90 :
                        //SetPortBit(1, 0, ucValue);  
					    P1  = ucValue;
                        break;
                    case 0xa0 :
                        //SetPortBit(2, 0, ucValue);  
					    P2  = ucValue;
                        break;
                    case 0xb0 :
                        //SetPortBit(3, 0, ucValue);  
					    P3  = ucValue;
                        break;                    
#if(GETSFR)
					default:
					    SetSFRPort(ucAddr, ucValue);
						break;
#endif
                    }
                    break;
                }
            }
        }
    }
    while (1 == ucStop);

#endif

}


#endif
////////////////////////////////////////////////////////////////////////////////////////////
//	anson add
////////////////////////////////////////////////////////////////////////////////////////////
void Set_Spread(void)
{
	RTDSetBit(SPREAD_SPECTRUM_99, 0x00, (stGUD3.SPREAD << 4));
	RTDSetBit(DCLK_OFFSET_MSB_9B, 0xff, 0x20);
}

void Init_FRecall(void)				//anson
{
	stGUD0.CONTRAST         = INIT_EEPROM0[3];
	stGUD0.BRIGHT           = INIT_EEPROM0[4];
//	stGUD0.RTD_R_CONTRAST   = INIT_EEPROM0[5];
//	stGUD0.RTD_G_CONTRAST   = INIT_EEPROM0[6];
//	stGUD0.RTD_B_CONTRAST   = INIT_EEPROM0[7];
	stGUD0.RTD_R_BRIGHT     = INIT_EEPROM0[8];
	stGUD0.RTD_G_BRIGHT     = INIT_EEPROM0[9];
	stGUD0.RTD_B_BRIGHT     = INIT_EEPROM0[10];
	
	stGUD1.FUNCTION         = INIT_EEPROM0[11];
	stGUD1.INPUT_SOURCE     = INIT_EEPROM0[12];
	stGUD1.FILTER           = INIT_EEPROM0[13];
	stGUD1.OSD_POSH         = INIT_EEPROM0[14];
	stGUD1.OSD_POSV         = INIT_EEPROM0[15];
	stGUD1.OSD_TIMEOUT      = INIT_EEPROM0[16];
	stGUD1.OSD_INPUT      = INIT_EEPROM0[17];

	stGUD3.VOLUME           = INIT_EEPROM1[11];
	stGUD3.CURR_CHANNEL     = INIT_EEPROM1[12];
	stGUD3.PREV_CHANNEL     = INIT_EEPROM1[13];

	I2CWrite(INIT_EEPROM0);
	Delay_Xms(SET_2404_DELAY);
	
//	I2CWrite(INIT_EEPROM1);		//anson 1223
//	Delay_Xms(SET_2404_DELAY);

	Save_GUD0();
	Save_GUD3();					//anson 05_0303
}

void Init_FACTORY(void)		//anson
{
	unsigned char   ucTemp1, ucTemp2;

	// Save the global settings we don't want to reset
	ucTemp1 = stGUD1.FUNCTION & 0x07;       // Language select
	ucTemp2 = stGUD1.INPUT_SOURCE & 0x07;   // Source select

	if (JAPANESS < ucTemp1)
		ucTemp1 = ENGLISH;

	// Reset OSD time-out timer
	usOSD_Timer = (unsigned int)2 << 9;     // 20 sec

	// Reset global settings to default
	Init_FRecall();

	// Reset OSD time-out timer
	usOSD_Timer = (unsigned int)stGUD1.OSD_TIMEOUT << 9;

	// Restore the global settings we don't want to reset
	stGUD1.FUNCTION     = (stGUD1.FUNCTION & 0xf8) | ucTemp1;
	stGUD1.INPUT_SOURCE = (stGUD1.INPUT_SOURCE & 0xf8) | ucTemp2;
	Save_GUD1();

	// Reset brightness and contrast to default
	Set_Bright_Contrast();
	SetADC_Gain();				//anson
		
#if (AUDIO_TYPE != AUDIO_NONE)
		SetVolume();
#endif

	if (SOURCE_VGA == (stGUD1.INPUT_SOURCE & 0x07))
	{
		if (ERROR_INPUT == Auto_Config())   ucMode_Curr = MODE_OSDFORCE;
	}

	// Reset OSD position
	OSD_Position(OSD_ENABLE);

	// Reset all mode settings to default
	Init_MUD();

	// Read default settings for current mode
	Load_MUD(ucMode_Curr);

	// Leave current mode and search mode again
//	ucMode_Curr = MODE_OSDFORCE;
}

