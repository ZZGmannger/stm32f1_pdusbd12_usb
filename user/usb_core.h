
#ifndef __USB_CORE_H
#define __USB_CORE_H

#include "pdiusbd12.h"

#define GET_STATUS         0
#define CLEAR_FEATURE      1
#define SET_FEATURE        3
#define SET_ADDRESS        5
#define GET_DESCRIPTOR     6
#define SET_DESCRIPTOR     7
#define GET_CONFIGURATION  8
#define SET_CONFIGURATION  9
#define GET_INTERFACE      10
#define SET_INTERFACE      11
#define SYNCH_FRAME        12

#define DEVICE_DESCRIPTOR         1
#define CONFIGURATION_DESCRIPTOR  2
#define STRING_DESCRIPTOR         3
#define INTERFACE_DESCRIPTOR      4
#define ENDPOINT_DESCRIPTOR       5
#define REPORT_DESCRIPTOR         0x22

#define SET_IDLE 0x0A

#define GET_LINE_CODING         0x21
#define SERIAL_STATE            0x20
#define SET_LINE_CODING         0x20
#define SET_CONTROL_LINE_STATE  0x22
#define SEND_BREAK              0x23

void UsbDisconnect(void); //USB�Ͽ�����
void UsbConnect(void);    //USB����
void UsbBusSuspend(void); //���߹����жϴ���
void UsbBusReset(void);   //���߸�λ�жϴ���
void UsbEp0Out(void);     //�˵�0����жϴ���
void UsbEp0In(void);      //�˵�0�����жϴ���
void UsbEp1Out(void);     //�˵�1����жϴ���
void UsbEp1In(void);      //�˵�1�����жϴ���
void UsbEp2Out(void);     //�˵�2����жϴ���
void UsbEp2In(void);      //�˵�2�����жϴ���

extern uint8_t ConfigValue;  //��ǰ����ֵ
extern uint8_t Ep1InIsBusy;  //�˵�1�����Ƿ�æ
extern uint8_t Ep2InIsBusy;  //�˵�2�����Ƿ�æ

#endif

