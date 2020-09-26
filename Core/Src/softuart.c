#include "softuart.h"

static uint8_t 							ScanPortBuffer;

SoftUart_S 						SUart[Number_Of_SoftUarts];
SoftUartBuffer_S 			SUBuffer[Number_Of_SoftUarts];

GPIO_PinState SoftUartGpioReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	return HAL_GPIO_ReadPin(GPIOx,GPIO_Pin);
}

void SoftUartGpioWritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,PinState);
}

SoftUartState_E SoftUartInit(uint8_t SoftUartNumber,GPIO_TypeDef *TxPort,uint16_t TxPin,GPIO_TypeDef *RxPort,uint16_t RxPin)
{
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	
	SUart[SoftUartNumber].TxNComplated=0;
	
	SUart[SoftUartNumber].RxBitConter=0;
	SUart[SoftUartNumber].RxBitShift=0;
	SUart[SoftUartNumber].RxIndex=0;

	SUart[SoftUartNumber].TxEnable=0;
	SUart[SoftUartNumber].RxEnable=0;
	
	SUart[SoftUartNumber].TxBitConter=0;
	SUart[SoftUartNumber].TxBitShift=0;
	SUart[SoftUartNumber].TxIndex=0;
	
	SUart[SoftUartNumber].TxSize=0;
	
	SUart[SoftUartNumber].Buffer=&SUBuffer[SoftUartNumber];
	
	SUart[SoftUartNumber].RxPort=RxPort;
	SUart[SoftUartNumber].RxPin=RxPin;
	
	SUart[SoftUartNumber].TxPort=TxPort;
	SUart[SoftUartNumber].TxPin=TxPin;
	
	return SoftUart_OK;
}

void SoftUartTransmitBit(SoftUart_S *SU,uint8_t Bit0_1)
{
	SoftUartGpioWritePin(SU->TxPort,SU->TxPin,(GPIO_PinState)Bit0_1);
}

SoftUartState_E SoftUartEnableRx(uint8_t SoftUartNumber)
{
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	SUart[SoftUartNumber].RxEnable=1;
	return SoftUart_OK;
}

SoftUartState_E SoftUartDisableRx(uint8_t SoftUartNumber)
{
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	SUart[SoftUartNumber].RxEnable=0;
	return SoftUart_OK;
}

uint8_t SoftUartRxAlavailable(uint8_t SoftUartNumber)
{
	return SUart[SoftUartNumber].RxIndex;
}

SoftUartState_E SoftUartReadRxBuffer(uint8_t SoftUartNumber,uint8_t *Buffer,uint8_t Len)
{
	int i;
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	for(i=0;i<Len;i++)
	{
		Buffer[i]=SUart[SoftUartNumber].Buffer->Rx[i];
	}
	for(i=0;i<SUart[SoftUartNumber].RxIndex;i++)
	{
		SUart[SoftUartNumber].Buffer->Rx[i]=SUart[SoftUartNumber].Buffer->Rx[i+Len];
	}
	SUart[SoftUartNumber].RxIndex-=Len;
	return SoftUart_OK;
}

void SoftUartTxProcess(SoftUart_S *SU)
{
	if(SU->TxEnable)
	{
		if(SU->TxBitConter==0)
		{
			SU->TxNComplated=1;
			SU->TxBitShift=0;
			SoftUartTransmitBit(SU,0);
			SU->TxBitConter++;
		}
		else if(SU->TxBitConter<9)
		{
			SoftUartTransmitBit(SU,((SU->Buffer->Tx[SU->TxIndex])>>(SU->TxBitShift))&0x01);
			SU->TxBitConter++;
			SU->TxBitShift++;
		}
		else if(SU->TxBitConter==9)
		{
			SoftUartTransmitBit(SU,1);
			SU->TxBitConter++;
		}
		else if(SU->TxBitConter==10)
		{
			//Complate
			SU->TxBitConter=0;
			
			SU->TxIndex++;
			if(SU->TxSize > SU->TxIndex)
			{
				SU->TxNComplated=1;
				SU->TxEnable=1;
			}
			else
			{
				SU->TxNComplated=0;
				SU->TxEnable=0;
			}
		}
	}
}

void SoftUartRxDataBitProcess(SoftUart_S *SU,uint8_t B0_1)
{
	if(SU->RxEnable)
	{
		if(SU->RxBitConter==0)//Start
		{
			if(B0_1)return;
			SU->RxBitShift=0;
			SU->RxBitConter++;
			SU->Buffer->Rx[SU->RxIndex]=0;
		}
		else if(SU->RxBitConter<9)//Data
		{
			SU->Buffer->Rx[SU->RxIndex]|=((B0_1&0x01)<<SU->RxBitShift);
			SU->RxBitConter++;
			SU->RxBitShift++;
		}
		else if(SU->RxBitConter==9)
		{
			SU->RxBitConter=0;
			if(B0_1)//Stop Bit
			{
				//OK
				if((SU->RxIndex)<SoftUartRxBufferSize)(SU->RxIndex)++;
			}
		}
	}
}

void SoftUartWaitUntilTxComplate(uint8_t SoftUartNumber)
{
	while(SUart[SoftUartNumber].TxNComplated);
}

SoftUartState_E SoftUartPuts(uint8_t SoftUartNumber,uint8_t *Str,uint8_t Len)
{
	int i;
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	if(SUart[SoftUartNumber].TxNComplated) return SoftUart_Error;
	
	SUart[SoftUartNumber].TxIndex=0;
	SUart[SoftUartNumber].TxSize=Len;
	
	for(i=0;i<Len;i++)
	{
		SUart[SoftUartNumber].Buffer->Tx[i]= Str[i];
	}
	
	SUart[SoftUartNumber].TxNComplated=1;
	SUart[SoftUartNumber].TxEnable=1;
	
	return SoftUart_OK;
}

uint8_t SoftUartScanRxPorts(void)
{
	int i;
	uint8_t Buffer=0x00;
	for(i=0;i<Number_Of_SoftUarts;i++) 
	{
		Buffer|=((SoftUartGpioReadPin(SUart[i].RxPort,SUart[i].RxPin)&0x01)<<i);
	}
	return Buffer;
}

uint8_t SoftUartRxGetBit(uint8_t InputChannel)
{
	return ((ScanPortBuffer>>InputChannel)&0x01);
}

void SoftUartProcessRxBuffer(void)
{
	int i;
	for(i=0;i<Number_Of_SoftUarts;i++) SoftUartRxDataBitProcess(&SUart[i],SoftUartRxGetBit(i));
}

void SoftUartHandler(void)
{
	int i;
	
	//RX
	ScanPortBuffer=SoftUartScanRxPorts();//Sampeling
	SoftUartProcessRxBuffer();
	
	//TX
	for(i=0;i<Number_Of_SoftUarts;i++)//Transfer Data
	{
		SoftUartTxProcess(&SUart[i]);
	}
}
