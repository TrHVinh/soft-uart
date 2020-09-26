#include "main.h"

#define 							Number_Of_SoftUarts		6

#define								SoftUartTxBufferSize	32
#define								SoftUartRxBufferSize	64

typedef enum {
	SoftUart_OK,
	SoftUart_Error
}SoftUartState_E;

typedef struct{
	uint8_t					Tx[SoftUartTxBufferSize];
	uint8_t					Rx[SoftUartRxBufferSize];
}SoftUartBuffer_S;

typedef struct {
	__IO uint8_t 			TxNComplated;
	//__IO uint8_t 			RxNComplated;
	
	uint8_t						TxEnable;
	uint8_t						RxEnable;
	
	uint8_t 					TxBitShift,TxBitConter;
	uint8_t 					RxBitShift,RxBitConter;
	
	uint8_t						TxIndex,TxSize;
	uint8_t						RxIndex;//,RxSize;
	
	SoftUartBuffer_S	*Buffer;
	
	GPIO_TypeDef  		*TxPort;
	uint16_t 					TxPin;
	
	GPIO_TypeDef  		*RxPort;
	uint16_t 					RxPin;
	
} SoftUart_S;

//Call Every 104.16666666uS or 9600Hz
//But in real 104 us work Fine & Beter of 104.1666us!
void 						SoftUartHandler(void);

void 						SoftUartWaitUntilTxComplate(uint8_t SoftUartNumber);
uint8_t 				SoftUartRxAlavailable(uint8_t SoftUartNumber);
SoftUartState_E SoftUartPuts(uint8_t SoftUartNumber,uint8_t *Str,uint8_t Len);
SoftUartState_E SoftUartEnableRx(uint8_t SoftUartNumber);
SoftUartState_E SoftUartDisableRx(uint8_t SoftUartNumber);
SoftUartState_E SoftUartInit(uint8_t SoftUartNumber,GPIO_TypeDef *TxPort,uint16_t TxPin,GPIO_TypeDef *RxPort,uint16_t RxPin);
SoftUartState_E SoftUartReadRxBuffer(uint8_t SoftUartNumber,uint8_t *Buffer,uint8_t Len);
