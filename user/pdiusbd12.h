
#ifndef __PDIUSBD12_H
#define __PDIUSBD12_H

#include "stm32f1xx_hal.h"
#include <stdio.h>

#define LOG   printf
#define DEBUG

//D12的读ID命令
#define Read_ID  0xFD

//D12的设置模式命令
#define D12_SET_MODE  0xF3

//D12的读中断寄存器命令
#define READ_INTERRUPT_REGISTER  0xF4

//D12读端点缓冲区的命令
#define D12_READ_BUFFER 0xF0

//D12写端点缓冲区的命令
#define D12_WRITE_BUFFER 0xF0

//D12清除接收端点缓冲区的命令
#define D12_CLEAR_BUFFER    0xF2

//D12使能发送端点缓冲区的命令
#define D12_VALIDATE_BUFFER 0xFA

//D12的应答设置包命令
#define D12_ACKNOWLEDGE_SETUP 0xF1

//D12的设置地址/使能命令
#define D12_SET_ADDRESS_ENABLE 0xD0

//D12的使能端点命令
#define D12_SET_ENDPOINT_ENABLE 0xD8


#define D12SetCommandAddr()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET)
#define D12SetDataAddr()     HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET)

#define D12SetWr()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET)
#define D12ClrWr()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET)

#define D12SetRd()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET)
#define D12ClrRd()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET)

#define D12GetIntPin()  HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)

#define D12SetPortIn()  D12_data_dir(1)
#define D12SetPortOut() D12_data_dir(0)

uint8_t D12GetData(void);
void D12SetData(uint8_t data);

void D12WriteCommand(uint8_t cmd);
uint8_t D12ReadByte(void);
void D12WriteByte(uint8_t Value);


uint16_t D12ReadID(void);
void D12SetEndpointEnable(uint8_t Enable);
void D12SetAddress(uint8_t Addr);
uint8_t D12WriteEndpointBuffer(uint8_t Endp,uint8_t Len,uint8_t * Buf);
uint8_t D12ReadEndpointStatus(uint8_t Endp);
uint8_t D12ReadEndpointLastStatus(uint8_t Endp);
uint8_t D12ReadEndpointBuffer(uint8_t Endp, uint8_t len, uint8_t *Buf);
void D12AcknowledgeSetup(void);
void D12ClearBuffer(void);

#endif

