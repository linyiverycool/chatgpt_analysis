/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include  "stm32f10x.h"
#include  "stm32f10x_conf.h" 
#include  "bsp.h"
extern __IO uint32_t TimingDelay ;

extern u8 FLAG_1;

extern u8 Send_Step;//Send_Flag
extern u16 Grid_flag; 

/*
*********************************************************************************************************
* Function    ��void BSP_Configuration(void)
* Description : Ӳ����ʼ�� 
* Para        : none
* Return(s)   : none
* Example     : BSP_Configuration();
*********************************************************************************************************
*/
void BSP_Configuration(void)
{
	BSP_RCC_Configuration();
    BSP_GPIO_Configuration(); 
	BSP_EXTI_Configuration();
	//BSP_USART_Configuration();
}



void BSP_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*********USART TX**************/
	GPIO_InitStructure.GPIO_Pin = BSP_IO_TXD4;	    //UART4/UART5  TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;            //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);		      
    /*********USART RX**************/
    GPIO_InitStructure.GPIO_Pin = BSP_IO_RXD4;                 // UART4	RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;       // ���ÿ�©����
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    		        
}
/*
*********************************************************************************************************
* Function    ��void BSP_RCC_Configuration(void)
* Description : ʱ�����ú���,ϵͳʱ������Ϊ72MHZ������ʱ������ 
* Para        : none
* Return(s)   : none
* Example     : BSP_RCC_Configuration();
*********************************************************************************************************
*/
void BSP_RCC_Configuration(void)
{
	SystemInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);// ����ӳ�䡢IO�жϽ��ж�дǰ��AFIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG , ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // APB2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
}
/*
*********************************************************************************************************
* Function    ��void BSP_EXTI_Configuration(void)
* Description : IO�ж� 
* Para        : none
* Return(s)   : none
* Example     : none
*********************************************************************************************************
*/
void BSP_EXTI_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    //EXTI_InitTypeDef EXTI_InitStructure;
    /* Configure one bit for preemption priority */
    /* ���ȼ��� ˵������ռ���ȼ����õ�λ��������Ӧ���ȼ����õ�λ��   ��������1�� 3 
    0�飺  ��ռ���ȼ�ռ0λ�� ��Ӧ���ȼ�ռ4λ
    1�飺  ��ռ���ȼ�ռ1λ�� ��Ӧ���ȼ�ռ3λ
    2�飺  ��ռ���ȼ�ռ2λ�� ��Ӧ���ȼ�ռ2λ
    3�飺  ��ռ���ȼ�ռ3λ�� ��Ӧ���ȼ�ռ1λ
    4��:   ��ռ���ȼ�ռ4λ�� ��Ӧ���ȼ�ռ0λ  */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;			     //���ô���UART4�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	     //��ռ���ȼ� 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;               // �����ȼ�0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/*
*********************************************************************************************************
* Function    ��void BSP_USART_Configuration(USART_TypeDef* USARTx)
* Description : ����ͨ�Ų������� 
* Para        : none
* Return(s)   : none
* Example     : BSP_USART_Configuration(USART1);
*********************************************************************************************************
*/

void BSP_USART_Configuration(USART_TypeDef* USARTx)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = 115200;					//����115200bps
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//����λ8λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;			//ֹͣλ1λ
    USART_InitStructure.USART_Parity = USART_Parity_No;				//��У��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ

    /* Configure USART1 */
    USART_Init(USARTx, &USART_InitStructure);							//���ô��ڲ�������
 
  
    /* Enable USART1 Receive and Transmit interrupts */
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);                      //ʹ�ܽ����ж�
    USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);						//ʹ�ܷ��ͻ�����ж�
    //USART_ITConfig(USART1, USART_IT_TC, ENABLE);						//ʹ�ܷ�������ж�

    /* Enable the USART1 */
    USART_Cmd(USARTx, ENABLE);	
}
/*
*********************************************************************************************************
* Function    ��void BSP_USART_OUT(void)
* Description : ��Ƭ�����ڷ�������
* Para        : none
* Return(s)   : none
* Example     : BSP_USART_OUT(USART1, uint8_t *Data,uint16_t Len)
*********************************************************************************************************
*/
void BSP_USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);      //�ж��Ƿ�����ɡ�
        USART_SendData(USARTx, Data[i]);					    
	     
	}
}

void UART4_IRQHandler(void)
{
		static u8 RxCnt;
		uint8_t RxData;
   	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
 	 {	
  		 RxData = USART_ReceiveData(UART4);   //�����Ĵ��������ݻ��浽���ջ�������
		 
			 if(RxData==0x81) RxCnt=1;
			 if(RxCnt==2)
			 {
					RxCnt=0;
					if(RxData==0x50) 			{	Send_Step=2; Grid_flag=0;}//������Ŀ��ɹ�,���ֹͣ;
					else if(RxData==0x52)	{	Send_Step=1;}//��������ɹ�,��ͣ����;	Send_Flag=0;
					else if(RxData==0x55)	{	Send_Step=1;}//������������;Send_Flag=1;	
			 }
			 if(RxCnt==1)	RxCnt++;
			 
			/* if(RxData==0x81) RxCnt=1;
			 if(RxCnt==2)
			 {
					RxCnt=0;
					if(RxData==0x50) 			{	Send_Step=2; Grid_flag=0;}//������Ŀ��ɹ�,���ֹͣ;
					else if(RxData==0x52)	{	Send_Step=1;}//��������ɹ�,��ͣ����;	Send_Flag=0;
					else if(RxData==0x55)	{	Send_Step=1;}//������������;Send_Flag=1;	
			 }
			 if(RxCnt==1)	RxCnt++;
			 */

 	 }

	if(USART_GetITStatus(UART4, USART_IT_TXE) != RESET)                   //�����Ϊ�˱���STM32 USART ��һ���ֽڷ�����ȥ��BUG 
 	 { 
    	 USART_ITConfig(UART4, USART_IT_TXE, DISABLE);					     //��ֹ�����������жϣ� 
 	 }	
}





/****************************************************************************
* ��    �ƣ�void Delay(__IO uint32_t nTime)
* ��    �ܣ���ʱ��ʱ���� 1msΪ��λ
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/  
/*void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}*/
/****************************************************************************
* ��    �ƣ�void TimingDelay_Decrement(void)
* ��    �ܣ���ȡ���ĳ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/  
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}


