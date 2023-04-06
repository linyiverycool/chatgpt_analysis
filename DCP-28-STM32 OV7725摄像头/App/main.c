/*
 * @file          app.c
 * @brief         ����������
 * @ucos-ii       V2.86 
 * @function      USART1---UART5 ͨ��    
 * @compiler      RealView MDK-ARM Compiler V4.11
 * @peripheral    V3.5.0
 * @author  	  Yuhaiyang 
 * @date   	      2015/10/26
 * @corporation   �㽭���ǿƽ��豸���޹�˾
 * @website       http://www.zjqiushi.com/
*/
/* Includes ------------------------------------------------------------------*/
#include  "stm32f10x.h"
#include  "stm32f10x_conf.h" 
#include  "sys.h"
#include  "bsp.h"
//#include "ov7670.h"	
#include "ov7725.h" 		 
#include "sccb.h"	
#include "STDIO.h"
#include "exti.h"
#include "delay.h"
/* Private variables ---------------------------------------------------------*/
// Systick delay
__IO uint32_t TimingDelay = 0 ;

extern u8 ov_sta;	//��exit.c�� �涨��

int redThre = 55; // 115~135    
int saturationTh = 7; //55~65  
u8 FLAG_1=0;
u16 x_arr[330],y_arr[330];
u16 l,h;

u8 Send_Flag,Send_Step;//
u8 Send_Aim,Send_Full;
u16 Send_X,Send_Y; 
u16 Aim_Num; 
u16 Grid_flag=0; 


//����LCD��ʾ(OV7725)
void OV7725_camera_refresh(void)
{
	u32 i;
 	u8 color1,color2;
 	u16 dadt2;
	u8 R,G,B,minValue,maxValue;//
	double HI,SI,VI;
	u16 x_mid,y_mid;
	u16 h_min,h_max,h_num;
	u16 l_min,l_max,l_num;
	
	x_mid=0;		y_mid=0;
	
	if(ov_sta)//��֡�жϸ���
	{
		OV7725_RRST=0;				//��ʼ��λ��ָ�� 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//��λ��ָ����� 
		OV7725_RCK_H;
		
		for(l=0;l<240;l++)
		{
			for(h=0;h<320;h++)
			{
				OV7725_RCK_L;
				color1 = ((GPIOE->IDR&0XFF00)>>8);	//������
				OV7725_RCK_H; 
				OV7725_RCK_L;
				color2 = ((GPIOE->IDR&0XFF00)>>8);	//������
				OV7725_RCK_H; 
				
				dadt2=(color1<<8)|color2;
				
				
				R = (dadt2>>8)&0xf8;
				G = (dadt2>>3)&0xfc;
				B = dadt2<<3;
				
					if(B > G)	{minValue = G;	maxValue = B;}
					else	{minValue = B;	maxValue = G;}
					if(R >= maxValue)	maxValue = R;
					else if(R < minValue)	minValue = R;
					if(maxValue == R)	HI = (G-B)*1.0/(maxValue-minValue);
					else if(maxValue == G)	HI = 2+ (B-R)*1.0/(maxValue-minValue);
					else if(maxValue == B)	HI = 4+ (R-G)*1.0/(maxValue-minValue);
					HI = HI*60;
					if(HI <0 ) HI = HI+360;
					VI = maxValue;
					SI = (maxValue-minValue)*1.0/maxValue;
					
					//R > RT  R>=G>=B  S>=((255-R)*ST/RT)
					// H=360,��ɫ; H=300,Ʒ��; H=60,��ɫ;S=0,ֻ�лҶ�;S=0,V=0,��;S=0,V=1,��;
					//  V>75% �� S>20% ����ɫ����;(255�ı���);
					//  V>75% �� S<20% ��ɫ;
					//  V<25% ��ɫ;
					//  ����Ϊ��ɫ����; V>25% && V<75% ;
					if( ((HI > 300)||(HI < 60)) && (VI>64) ) 	// &&      && (SI>51) (SI>71))||(VI<240))  && ((VI>191)&&(SI>51))
					{
						//dadt2 = 0x0000;	// ��ɫ;
						h_num++;
						if(h_num==1) h_min = h;
					}
					else
					{
						if(h_num!=0)
						{
							h_max = h;
							if((h_max-h_min)>50) 
							{
								for(i=h_min;i<=h_max;i++)
								x_arr[i]=1;
								y_arr[l]=1;
							}
						}
						h_num=0;
						dadt2 = 0xffff;	// ��ɫ;
					}
		
			}
		}
		
	
		// ȡ���ĵ�;(X,Y)=(y_mid,x_mid);
		h_max=0;h_min=0;
		for(h = 0; h < 320; h ++)
		{
			if(x_arr[h]!=0)		{	h_min=h;	break;	}
		}
		for(h = 319; h > 0; h --)
		{
			if(x_arr[h]!=0)		{	h_max=h;	break;	}
		}
		
		l_max=0;l_min=0;
		for(l = 0; l < 240; l ++)
		{
			if(y_arr[l]!=0)		{	l_min=l;	break;	}
		}
		for(l = 239; l > 0; l --)
		{
			if(y_arr[l]!=0)		{	l_max=l;	break;	}
		}
			
			if((h_max!=h_min) && (l_max!=l_min))
			{
					Send_Aim = 0xFB;// ��Ŀ�����;
				
					Grid_flag=1;
					
			if(((h_max-h_min)>75) && ((l_max-l_min)>75))
			//		if((h_min<5)||(h_max>315)||(l_min<5)||(l_max>235))
					Send_Full = 0xFC;// ����ȫ���ڻ�����;
					else 
					Send_Full = 0xFD;// ���岿���ڻ�����;
					
					Send_X = (l_max+l_min)/2;
					Send_Y = 320-(h_max+h_min)/2;// ������������  ;
			}
			else
			{
				//Aim_Num++;
				//if(Aim_Num>2)
				//{
				//	Aim_Num=5;
					Grid_flag=0;
					if(Send_Step==2)Send_Step=1;
					Send_Aim = 0xFA;// û��Ŀ�����;
				//}
			}
 		ov_sta=0;					//����֡�жϱ��
	}
}


int main(void)
{
    BSP_Configuration();
	BSP_USART_Configuration(UART4);
	 delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    if (SysTick_Config(72000))		    //ʱ�ӽ����ж�ʱ1msһ��  ���ڶ�ʱ 
    {
        while (1);
    }
		
FLAG_1=0;
		
	while(1)//��ʼ��OV7725_OV7670
	{
		if(OV7725_Init()==0)//��ʼ��OV7725,//����0:�ɹ�
		{
			//sensor=OV7725;
					OV7725_Window_Set(320,240,0);//QVGAģʽ���		 	   
			OV7725_Light_Mode(0);//��ƽ������;0:�Զ�,1:̫��sunny;2,����cloudy;3,�칫��office;4,����home;
			OV7725_Color_Saturation(4);//ɫ������;sat:-2~+2;
			OV7725_Brightness(-4);//��������;//bright��-2~+2;
			OV7725_Contrast(-4);//�Աȶ�����;//contrast��-2~+2;
			OV7725_Special_Effects(0);//��Ч����;0:��ͨģʽ;1,��Ƭ;2,�ڰ�;3,ƫ��ɫ;4,ƫ��ɫ;5,ƫ��ɫ;6,����;
			//OV7725_Window_Set(12,176,240,320);//���ô���
			OV7725_CS=0;
			break;
			/*
			OV7725_Light_Mode(0);//��ƽ������;0:�Զ�,1:̫��sunny;2,����cloudy;3,�칫��office;4,����home;
			OV7725_Color_Saturation(4);//ɫ������;sat:-4~+4;
			OV7725_Brightness(4);//��������;//bright��-4~+4;
			OV7725_Contrast(4);//�Աȶ�����;//contrast��-4~+4;
			OV7725_Special_Effects(0);//��Ч����;0:��ͨģʽ;1,��Ƭ;2,�ڰ�;3,ƫ��ɫ;4,ƫ��ɫ;5,ƫ��ɫ;6,����;
			
			OV7725_CS=0;
			break;
			*/
		}
	}
									  
	EXTI15_10_Init();						//ʹ���ⲿ�ж�8,����֡�ж�	
	
		Send_Aim=0xFA;	
	//	Send_Flag=1;
		Send_Step=1;				 	 
	//
 	while(1)
	{
		for(h = 0; h < 320; h ++)	x_arr[h]=0;
		for(l = 0; l < 240; l ++)	y_arr[l]=0;
		
			switch(Send_Step)
			{
				case 1:	// �Ƿ���Ŀ�����;
					Grid_flag=0;
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, 0x80);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_Aim);
					break;
				case 2:// ������������;// ���岿���ڻ�����;
					if(Grid_flag==1)
					{
						Grid_flag=0;
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, 0x80);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, 0xFE);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_Full);
				
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_X/100);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_X/10%10);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_X%10);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_Y/100);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_Y/10%10);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, Send_Y%10);
				
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(UART4, 0xFE);
					}
					break;
				default:break;
			}
		//if(Send_Aim==0xFB)
	//	if(Send_Flag==1)
	//	{
		/*
			switch(Send_Aim)
			{
				case 0xFA:	// �Ƿ���Ŀ�����;
						Grid_flag=0;
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, 0x80);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, 0XFA);
					break;
				case 0xFB:// ������������;// ���岿���ڻ�����;
					if(Grid_flag==1)
					{
						Grid_flag=0;
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, 0x80);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, 0xFB);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_Full);
						
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_X/100);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_X/10%10);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_X%10);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_Y/100);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_Y/10%10);
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, Send_Y%10);
						
						while(USART_GetFlagStatus(UART4, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
						USART_SendData(UART4, 0xFE);
					}
					break;
				default:break;
			}
	//	}

		*/

		OV7725_camera_refresh();	//������ʾ
	}	

}


