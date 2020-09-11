#include "usb_core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG_CORE(...)


/*uart=================================================*/

uint8_t UartBuffer[128];
uint8_t UsbEp2Buffer[128];

uint8_t UartBufferOutputPoint;
uint8_t UartBufferInputPoint;
uint8_t UsbEp2BufferOutputPoint;

uint8_t UartByteCount;
uint8_t UsbEp2ByteCount;

volatile uint8_t Sending;

/*=================================================*/

uint8_t DeviceDescriptor[0x12] = //18�ֽ��豸������
{
    0x12, //bLength  �ֶΡ��豸�������ĳ���Ϊ18 �ֽ�
    0x01, //bDescriptorType �ֶΣ��豸�������ı��Ϊ0x01

    0x10, //bcd USB �汾��С��ģʽ
    0x01,
//bDeviceClass�ֶΡ����豸�������豸��������ָ���豸�����ͣ�
//�������������ü������������ӿڣ��ͻᱻϵͳ��Ϊ��һ��USB
//�����豸���Ӷ������豸������������0x02Ϊͨ���豸�������롣
    0x02, //bDeviceClass �豸��
    0x00, //bDeviceSubClass
    0x00, //bDeviceProtocol
    0x10, //bMaxPacketSize0 �ֶ� PD12USBD12 �Ķ˵�0��СΪ16�ֽ�

    0x88, //����ID ,��Ҫ����
    0x88,
	
//idProduct�ֶΡ���ƷID�ţ������ǵ��߸�ʵ�飬��������ȡ0x0007��
//ע��С��ģʽ�����ֽ�Ӧ����ǰ��
    0x06, //idProduct �ֶ� ��ƷID ����һ��ʵ�飬����Ϊ0x0001
    0x00,

    0x00, //bcd device�ֶ� USB���v1.0�汾 0x0100
    0x01,

    0x01, //iManufacturer �����ַ���������ֵ��Ϊ�˷����������ַ��������ʹ�1��ʼ
    0x02, //iProduct ��Ʒ�ַ���������ֵ
    0x03, //iSerialNumber �豸���к��ַ�������
    0x01  //bNumConfigurations
};


//USB�����������Ķ���
uint8_t ReportDescriptor[] =
{
    //ÿ�п�ʼ�ĵ�һ�ֽ�Ϊ����Ŀ��ǰ׺��ǰ׺�ĸ�ʽΪ��
    //D7~D4��bTag��D3~D2��bType��D1~D0��bSize�����·ֱ��ÿ����Ŀע�͡�

    //����һ��ȫ�֣�bTypeΪ1����Ŀ��ѡ����;ҳΪ��ͨ����Generic Desktop Page(0x01)
    //�����һ�ֽ����ݣ�bSizeΪ1����������ֽ����Ͳ�ע���ˣ�
    //�Լ�����bSize���жϡ�
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)

    //����һ���ֲ���bTypeΪ2����Ŀ��˵����������Ӧ�ü�����;�������
    0x09, 0x02, // USAGE (Mouse)

    //����һ������Ŀ��bTypeΪ0����Ŀ�������ϣ������������0x01��ʾ
    //�ü�����һ��Ӧ�ü��ϡ�����������ǰ������;ҳ����;����Ϊ
    //��ͨ�����õ���ꡣ
    0xa1, 0x01, // COLLECTION (Application)

    //����һ���ֲ���Ŀ��˵����;Ϊָ�뼯��
    0x09, 0x01, //   USAGE (Pointer)

    //����һ������Ŀ�������ϣ������������0x00��ʾ�ü�����һ��
    //�����ϣ���;��ǰ��ľֲ���Ŀ����Ϊָ�뼯�ϡ�
    0xa1, 0x00, //   COLLECTION (Physical)

    //����һ��ȫ����Ŀ��ѡ����;ҳΪ������Button Page(0x09)��
    0x05, 0x09, //     USAGE_PAGE (Button)

    //����һ���ֲ���Ŀ��˵����;����СֵΪ1��ʵ��������������
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)

    //����һ���ֲ���Ŀ��˵����;�����ֵΪ3��ʵ����������м���
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)

    //����һ��ȫ����Ŀ��˵�����ص����ݵ��߼�ֵ���������Ƿ��ص��������ֵ����
    //��СΪ0����Ϊ����������Bit����ʾһ�������������СΪ0�����Ϊ1��
    0x15, 0x00, //     LOGICAL_MINIMUM (0)

    //����һ��ȫ����Ŀ��˵���߼�ֵ���Ϊ1��
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)

    //����һ��ȫ����Ŀ��˵�������������Ϊ������
    0x95, 0x03, //     REPORT_COUNT (3)

    //����һ��ȫ����Ŀ��˵��ÿ��������ĳ���Ϊ1��bit��
    0x75, 0x01, //     REPORT_SIZE (1)

    //����һ������Ŀ��˵����3������Ϊ1bit�������������ͳ���
    //��ǰ�������ȫ����Ŀ�����壩������Ϊ���룬
    //����Ϊ��Data,Var,Abs��Data��ʾ��Щ���ݿ��Ա䶯��Var��ʾ
    //��Щ�������Ƕ����ģ�ÿ�����ʾһ����˼��Abs��ʾ����ֵ��
    //��������Ľ�����ǣ���һ��������bit0��ʾ����1��������Ƿ��£�
    //�ڶ���������bit1��ʾ����2���Ҽ����Ƿ��£�������������bit2��ʾ
    //����3���м����Ƿ��¡�
    0x81, 0x02, //     INPUT (Data,Var,Abs)

    //����һ��ȫ����Ŀ��˵������������Ϊ1��
    0x95, 0x01, //     REPORT_COUNT (1)

    //����һ��ȫ����Ŀ��˵��ÿ��������ĳ���Ϊ5bit��
    0x75, 0x05, //     REPORT_SIZE (5)

    //����һ������Ŀ�������ã���ǰ������ȫ����Ŀ��֪������Ϊ5bit��
    //����Ϊ1������������Ϊ�����������ص�����һֱ��0����
    //���ֻ��Ϊ�˴���һ���ֽڣ�ǰ������3��bit��������һЩ����
    //���ѣ���������û��ʵ����;�ġ�
    0x81, 0x03, //     INPUT (Cnst,Var,Abs)

    //����һ��ȫ����Ŀ��ѡ����;ҳΪ��ͨ����Generic Desktop Page(0x01)
    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)

    //����һ���ֲ���Ŀ��˵����;ΪX��
    0x09, 0x30, //     USAGE (X)

    //����һ���ֲ���Ŀ��˵����;ΪY��
    0x09, 0x31, //     USAGE (Y)

    //����һ���ֲ���Ŀ��˵����;Ϊ����
    0x09, 0x38, //     USAGE (Wheel)

    //��������Ϊȫ����Ŀ��˵�����ص��߼���С�����ֵ��
    //��Ϊ���ָ���ƶ�ʱ��ͨ���������ֵ����ʾ�ģ�
    //���ֵ����˼���ǣ���ָ���ƶ�ʱ��ֻ�����ƶ�����
    //�����ƶ�ʱ��XֵΪ���������ƶ�ʱ��YֵΪ����
    //���ڹ��֣����������Ϲ�ʱ��ֵΪ����
    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f, //     LOGICAL_MAXIMUM (127)

    //����һ��ȫ����Ŀ��˵��������ĳ���Ϊ8bit��
    0x75, 0x08, //     REPORT_SIZE (8)

    //����һ��ȫ����Ŀ��˵��������ĸ���Ϊ3����
    0x95, 0x03, //     REPORT_COUNT (3)

    //����һ������Ŀ����˵��������8bit���������������õģ�
    //����Ϊ��Data,Var,Rel��Data˵�������ǿ��Ա�ģ�Var˵��
    //��Щ�������Ƕ����ģ�����һ��8bit��ʾX�ᣬ�ڶ���8bit��ʾ
    //Y�ᣬ������8bit��ʾ���֡�Rel��ʾ��Щֵ�����ֵ��
    0x81, 0x06, //     INPUT (Data,Var,Rel)

    //��������������Ŀ�����ر�ǰ��ļ����á�
    //���ǿ����������ϣ�����Ҫ�����Ρ�bSizeΪ0�����Ժ���û���ݡ�
    0xc0, //   END_COLLECTION
    0xc0  // END_COLLECTION
};
//USB�������������ϵĶ���
//�����������ܳ���Ϊ9+9+9+7�ֽ�
uint8_t ConfigurationDescriptor[9+9+5+5+4+5+7+9+7+7] =
{
   /***************����������***********************/
 //bLength�ֶΡ������������ĳ���Ϊ9�ֽڡ�
 0x09,
 
 //bDescriptorType�ֶΡ��������������Ϊ0x02��
 0x02,
 
 //wTotalLength�ֶΡ��������������ϵ��ܳ��ȣ�
 //�������������������ӿ��������������������˵��������ȡ�
 sizeof(ConfigurationDescriptor)&0xFF, //���ֽ�
 (sizeof(ConfigurationDescriptor)>>8)&0xFF, //���ֽ�
 
 //bNumInterfaces�ֶΡ������ð����Ľӿ������������ӿڡ�
 0x02,
 
 //bConfiguration�ֶΡ������õ�ֵΪ1��
 0x01,
 
 //iConfigurationz�ֶΣ������õ��ַ�������������û�У�Ϊ0��
 0x00,
 
 //bmAttributes�ֶΣ����豸�����ԡ��������ǵİ��������߹���ģ�
 //�������ǲ���ʵ��Զ�̻��ѵĹ��ܣ����Ը��ֶε�ֵΪ0x80��
 0x80,
 
 //bMaxPower�ֶΣ����豸��Ҫ�������������������ǵİ���
 //��Ҫ�ĵ�������100mA�����������������Ϊ100mA������ÿ��λ
 //����Ϊ2mA��������������Ϊ50(0x32)��
 0x32,
 
 /*******************CDC��ӿ�������*********************/
 //bLength�ֶΡ��ӿ��������ĳ���Ϊ9�ֽڡ�
 0x09,
 
 //bDescriptorType�ֶΡ��ӿ��������ı��Ϊ0x04��
 0x04,
 
 //bInterfaceNumber�ֶΡ��ýӿڵı�ţ���һ���ӿڣ����Ϊ0��
 0x00,
 
 //bAlternateSetting�ֶΡ��ýӿڵı��ñ�ţ�Ϊ0��
 0x00,
 
 //bNumEndpoints�ֶΡ���0�˵����Ŀ��CDC�ӿ�ֻʹ��һ���ж�
 //����˵㡣
 0x01,
 
 //bInterfaceClass�ֶΡ��ýӿ���ʹ�õ��ࡣCDC��������Ϊ0x02��
 0x02,
 
 //bInterfaceSubClass�ֶΡ��ýӿ���ʹ�õ����ࡣҪʵ��USBת���ڣ�
 //�ͱ���ʹ��Abstract Control Model���������ģ�ͣ����ࡣ����
 //���Ϊ0x02��
 0x02,
 
 //bInterfaceProtocol�ֶΡ�ʹ��Common AT Commands��ͨ��AT���
 //Э�顣��Э��ı��Ϊ0x01��
 0x01,
 
 //iConfiguration�ֶΡ��ýӿڵ��ַ�������ֵ������û�У�Ϊ0��
 0x00,
 
 /***************����Ϊ����������****************/
 /********* Header Functional Descriptor ********/
 //bFunctionLength�ֶΡ�������������Ϊ5�ֽ�
 0x05,
 
 //bDescriptorType�ֶΡ�����������Ϊ������ӿڣ�CS_INTERFACE��
 //���Ϊ0x24��
 0x24,
 
 //bDescriptorSubtype�ֶΡ�����������ΪHeader Functional Descriptor
 //���Ϊ0x00��
 0x00,
 
 //bcdCDC�ֶΡ�CDC�汾�ţ�Ϊ0x0110�����ֽ����ȣ�
 0x10,
 0x01,
 
 /**** Call Management Functional Descriptor ****/
 //bFunctionLength�ֶΡ�������������Ϊ5�ֽ�
 0x05,
 
 //bDescriptorType�ֶΡ�����������Ϊ������ӿڣ�CS_INTERFACE��
 //���Ϊ0x24��
 0x24,
 
 //bDescriptorSubtype�ֶΡ�����������ΪCall Management 
 //functional descriptor�����Ϊ0x01��
 0x01,
 
 //bmCapabilities�ֶΡ��豸�Լ�������call management
 0x00,
 
 //bDataInterface�ֶΡ�û��������ӿ�����call management
 0x00,

 /*** Abstract Control Management Functional Descriptor ***/
 //bFunctionLength�ֶΡ�������������Ϊ4�ֽ�
 0x04,
 
 //bDescriptorType�ֶΡ�����������Ϊ������ӿڣ�CS_INTERFACE��
 //���Ϊ0x24��
 0x24,
 
 //bDescriptorSubtype�ֶΡ�����������ΪAbstract Control 
 //Management functional descriptor�����Ϊ0x02��
 0x02,

 //bmCapabilities�ֶΡ�֧��Set_Line_Coding��Set_Control_Line_State��
 //Get_Line_Coding�����Serial_State֪ͨ
 0x02,

 /***  Union Functional Descriptor  **/
 //bFunctionLength�ֶΡ�������������Ϊ5�ֽڡ� 
 0x05,

 //bDescriptorType�ֶΡ�����������Ϊ������ӿڣ�CS_INTERFACE��
 //���Ϊ0x24��
 0x24,
 
 //bDescriptorSubtype�ֶΡ�����������Ϊ
 //Union functional descriptor�����Ϊ0x06��
 0x06,
 
 //MasterInterface�ֶΡ�����Ϊǰ����Ϊ0��CDC�ӿڡ�
 0x00,
 
 //SlaveInterface�ֶΣ�����Ϊ���������Ϊ1��������ӿڡ�
 0x01,

 /***********  ����Ϊ�ӿ�0�Ķ˵�������  *******/
 //bLength�ֶΡ��˵�����������Ϊ7�ֽڡ�
 0x07,
 
 //bDescriptorType�ֶΡ��˵����������Ϊ0x05��
 0x05,
 
 //bEndpointAddress�ֶΡ��˵�ĵ�ַ������ʹ��D12������˵�1��
 //D7λ��ʾ���ݷ�������˵�D7Ϊ1����������˵�1�ĵ�ַΪ0x81��
 0x81,
 
 //bmAttributes�ֶΡ�D1~D0Ϊ�˵㴫������ѡ��
 //�ö˵�Ϊ�ж϶˵㡣�ж϶˵�ı��Ϊ3������λ����Ϊ0��
 0x03,
 
 //wMaxPacketSize�ֶΡ��ö˵�����������˵�1��������Ϊ16�ֽڡ�
 //ע����ֽ����ȡ�
 0x10,
 0x00,
 
 //bInterval�ֶΡ��˵��ѯ��ʱ�䣬��������Ϊ10��֡ʱ�䣬��10ms��
 0x0A,
 
 /*********  ����Ϊ�ӿ�1�����ݽӿڣ��Ľӿ�������  *********/
 //bLength�ֶΡ��ӿ��������ĳ���Ϊ9�ֽڡ�
 0x09,
 
 //bDescriptorType�ֶΡ��ӿ��������ı��Ϊ0x04��
 0x04,
 
 //bInterfaceNumber�ֶΡ��ýӿڵı�ţ��ڶ����ӿڣ����Ϊ1��
 0x01,
 
 //bAlternateSetting�ֶΡ��ýӿڵı��ñ�ţ�Ϊ0��
 0x00,
 
 //bNumEndpoints�ֶΡ���0�˵����Ŀ�����豸��Ҫʹ��һ�������˵㣬����Ϊ2��
 0x02,
 
 //bInterfaceClass�ֶΡ��ýӿ���ʹ�õ��ࡣ������ӿڵĴ���Ϊ0x0A��
 0x0A,
 
 //bInterfaceSubClass�ֶΡ��ýӿ���ʹ�õ�����Ϊ0��
 0x00,
 
 //bInterfaceProtocol�ֶΡ��ýӿ���ʹ�õ�Э��Ϊ0��
 0x00,
 
 //iConfiguration�ֶΡ��ýӿڵ��ַ�������ֵ������û�У�Ϊ0��
 0x00,
 
 /*****  ����Ϊ�ӿ�1��������ӿڣ��Ķ˵�������  *****/
 /*************** ��������˵�2������ ******************/
 //bLength�ֶΡ��˵�����������Ϊ7�ֽڡ�
 0x07,
 
 //bDescriptorType�ֶΡ��˵����������Ϊ0x05��
 0x05,
 
 //bEndpointAddress�ֶΡ��˵�ĵ�ַ������ʹ��D12������˵�2��
 //D7λ��ʾ���ݷ�������˵�D7Ϊ1����������˵�2�ĵ�ַΪ0x82��
 0x82,
 
 //bmAttributes�ֶΡ�D1~D0Ϊ�˵㴫������ѡ��
 //�ö˵�Ϊ�����˵㣬�����˵�ı��Ϊ0x02������λ����Ϊ0��
 0x02,
 
 //wMaxPacketSize�ֶΡ��ö˵�����������˵�2��������Ϊ64�ֽڡ�
 //ע����ֽ����ȡ�
 0x40,
 0x00,
 
 //bInterval�ֶΡ��˵��ѯ��ʱ�䣬����������˵���Ч��
 0x00,
 
 /*************** ��������˵�2������ ******************/
 //bLength�ֶΡ��˵�����������Ϊ7�ֽڡ�
 0x07,
 
 //bDescriptorType�ֶΡ��˵����������Ϊ0x05��
 0x05,
 
 //bEndpointAddress�ֶΡ��˵�ĵ�ַ������ʹ��D12������˵�2��
 //D7λ��ʾ���ݷ�������˵�D7Ϊ0����������˵�2�ĵ�ַΪ0x02��
 0x02,
 
 //bmAttributes�ֶΡ�D1~D0Ϊ�˵㴫������ѡ��
 //�ö˵�Ϊ�����˵㣬�����˵�ı��Ϊ0x02������λ����Ϊ0��
 0x02,
 
 //wMaxPacketSize�ֶΡ��ö˵�����������˵�2��������Ϊ64�ֽڡ�
 //ע����ֽ����ȡ�
 0x40,
 0x00,
 
 //bInterval�ֶΡ��˵��ѯ��ʱ�䣬����������˵���Ч��
 0x00
};




/************************����ID�Ķ���********************/
uint8_t LanguageId[4]=
{
 0x04, //���������ĳ���
 0x03, //�ַ���������
 //0x0409Ϊ��ʽӢ���ID
 0x09,
 0x04
};
////////////////////////����ID���//////////////////////////////////

/**************************************************/
/*********        ��ת���������         **********/
/********* Http://computer00.21ic.org    **********/
/*********        ����: ����ȦȦ         **********/
/*********         ��ӭ���ʹ��          **********/
/*********    ��Ȩ���У�������д������   **********/
/**************************************************/

//http://computer00.21ic.org/user1/2198/archives/2007/42769.html
//�ַ���������ȦȦ��USBר�� Http://group.ednchina.com/93/����Unicode����
//8λС�˸�ʽ
uint8_t ManufacturerStringDescriptor[82]={
82,         //���������ĳ���Ϊ82�ֽ�
0x03,       //�ַ��������������ͱ���Ϊ0x03
0x35, 0x75, //��
0x11, 0x81, //��
0x08, 0x57, //Ȧ
0x08, 0x57, //Ȧ
0x84, 0x76, //��
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x13, 0x4e, //ר
0x3a, 0x53, //��
0x20, 0x00, // 
0x48, 0x00, //H
0x74, 0x00, //t
0x74, 0x00, //t
0x70, 0x00, //p
0x3a, 0x00, //:
0x2f, 0x00, ///
0x2f, 0x00, ///
0x67, 0x00, //g
0x72, 0x00, //r
0x6f, 0x00, //o
0x75, 0x00, //u
0x70, 0x00, //p
0x2e, 0x00, //.
0x65, 0x00, //e
0x64, 0x00, //d
0x6e, 0x00, //n
0x63, 0x00, //c
0x68, 0x00, //h
0x69, 0x00, //i
0x6e, 0x00, //n
0x61, 0x00, //a
0x2e, 0x00, //.
0x63, 0x00, //c
0x6f, 0x00, //o
0x6d, 0x00, //m
0x2f, 0x00, ///
0x39, 0x00, //9
0x33, 0x00, //3
0x2f, 0x00  ///
};
/////////////////////////�����ַ�������/////////////////////////////

//�ַ�������ȦȦ������USB��֮USB��ꡱ��Unicode����
//8λС�˸�ʽ
uint8_t ProductStringDescriptor[34]={
34,         //���������ĳ���Ϊ34�ֽ�
0x03,       //�ַ��������������ͱ���Ϊ0x03
0x0a, 0x30, //��
0x08, 0x57, //Ȧ
0x08, 0x57, //Ȧ
0x59, 0x65, //��
0x60, 0x4f, //��
0xa9, 0x73, //��
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x0b, 0x30, //��
0x4b, 0x4e, //֮
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x20, 0x9f, //��
0x07, 0x68  //��
};
////////////////////////��Ʒ�ַ�������////////////////////////////

//�ַ�����2008-07-07����Unicode����
//8λС�˸�ʽ
uint8_t SerialNumberStringDescriptor[22]={
22,         //���������ĳ���Ϊ22�ֽ�
0x03,       //�ַ��������������ͱ���Ϊ0x03
0x32, 0x00, //2
0x30, 0x00, //0
0x30, 0x00, //0
0x38, 0x00, //8
0x2d, 0x00, //-
0x30, 0x00, //0
0x37, 0x00, //7
0x2d, 0x00, //-
0x30, 0x00, //0
0x38, 0x00  //7
};
//////////////////////��Ʒ���к��ַ�������/////////////////////////



uint8_t Buffer[16]; //���˵�0�õĻ�����
//USB�豸����ĸ��ֶ�
uint8_t bmRequestType;
uint8_t bRequest;
uint16_t wValue;
uint16_t wIndex;
uint16_t wLength;
//��ǰ�������ݵ�λ��
uint8_t *pSendData;
//��Ҫ�������ݵĳ���
uint16_t SendLength;
//�Ƿ���Ҫ����0���ݰ��ı�־����USB���ƴ�������ݹ����У�
//�����ص����ݰ��ֽ�������������ʱ������Ϊ���ݹ��̽�����
//��������ֽ�����ʵ����Ҫ���ص��ֽ���������ʵ�ʷ��ص��ֽ�
//���ָպ��Ƕ˵�0��С��������ʱ������Ҫ����һ��0���ȵ����ݰ�
//���������ݹ��̡������������һ����־������������Ƿ���Ҫ����
//һ��0���ȵ����ݰ���
uint8_t NeedZeroPacket;

uint8_t ConfigValue;
uint8_t Ep1InIsBusy;


//�˵�2�����Ƿ�æ�ı�־������������������ʱ���ñ�־Ϊ�档
//���������п���ʱ���ñ�־Ϊ�١�
uint8_t Ep2InIsBusy;

//LineCoding���飬�������沨���ʡ�ֹͣλ�ȴ������ԡ�
//��ʼ��������Ϊ9600��1ֹͣλ����У�飬8����λ��
uint8_t LineCoding[7]={0x80,0x25,0x00,0x00,0x00,0x00,0x08};


void UsbDisconnect(void)
{
#ifdef DEBUG
    LOG("USB disconnected\r\n");
#endif
    D12WriteCommand(D12_SET_MODE); //set mode
    D12WriteByte(0x06);
    D12WriteByte(0x47);
    HAL_Delay(1000);
}

void UsbConnect(void)
{
#ifdef DEBUG
    LOG("USB connected\r\n");
#endif
    D12WriteCommand(D12_SET_MODE); //set mode
    D12WriteByte(0x16);
    D12WriteByte(0x47);
}

void UsbBusSuspend(void)
{
#ifdef DEBUG
    LOG("UsbBusSuspend\r\n");
#endif
}

void UsbBusReset(void)
{
#ifdef DEBUG
    LOG("UsbBusReset\r\n");
#endif
	Ep1InIsBusy=0; //��λ��˵�1���뻺�������С�
	Ep2InIsBusy=0; //��λ��˵�2���뻺�������С�
	UartBufferOutputPoint=0;
	UartBufferInputPoint=0;
	UartByteCount=0;
	UsbEp2ByteCount=0;
	UsbEp2BufferOutputPoint=0;
}


 
void UsbEp0Out(void);
void UsbEp0In(void);
void UsbEp1Out(void);
void UsbEp1In(void);
void UsbEp2Out(void);
void UsbEp2In(void);


void SendReport(void);

void UsbEp0SendData(void)
{
    //������д���˵���ȥ׼������
    //д֮ǰҪ���ж�һ����Ҫ���͵������Ƿ�ȶ˵�0
    //��󳤶ȴ���������˵��С����һ��ֻ�ܷ���
    //�����������ݡ��˵�0����������DeviceDescriptor[7]
    if (SendLength > DeviceDescriptor[7])
    {
        //���������ȷ���
        D12WriteEndpointBuffer(1, DeviceDescriptor[7], pSendData);
        //���ͺ�ʣ���ֽ�������������
        SendLength -= DeviceDescriptor[7];
        //����һ�κ�ָ��λ��Ҫ����
        pSendData += DeviceDescriptor[7];
    }
    else
    {
        if (SendLength != 0)
        {
            //����������������ֱ�ӷ���
            D12WriteEndpointBuffer(1, SendLength, pSendData);
            //������Ϻ�SendLength���ȱ�Ϊ0
            SendLength = 0;
        }
        else //���Ҫ���͵����ݰ�����Ϊ0
        {
            if (NeedZeroPacket == 1) //�����Ҫ����0��������
            {
                D12WriteEndpointBuffer(1, 0, pSendData); //����0�������ݰ�
                NeedZeroPacket = 0;                      //����Ҫ����0�������ݰ���־
            }
        }
    }
}

/********************************************************************
�������ܣ�USB�˵�0���ݹ������ݴ�������
��ڲ������ޡ�
��    �أ��ޡ�
��    ע���ú�����������0�˵���ƴ�������ݻ�״̬���̡�
********************************************************************/
void UsbEp0DataOut(void)
{
	//���ڱ�������ֻ��һ������������ݣ����Կ���ֱ��ʹ��if����ж�������
	//����кܶ�����Ļ���ʹ��if���Ͳ������ˣ���Ӧ��ʹ��switch���ɢת��
	if((bmRequestType==0x21)&&(bRequest==SET_LINE_CODING))
	{
		uint32_t BitRate;
		uint8_t Length;

		//����7�ֽڵ�LineCodingֵ
		Length=D12ReadEndpointBuffer(0,7,LineCoding);
		D12ClearBuffer(); //���������

		if(Length==7) //���������ȷ
		{
			//��LineCoding�������õĲ�����
			BitRate=LineCoding[3];
			BitRate=(BitRate<<8)+LineCoding[2];
			BitRate=(BitRate<<8)+LineCoding[1];
			BitRate=(BitRate<<8)+LineCoding[0];

			LOG("����������Ϊ��");
			LOG("%d",BitRate);
			LOG("bps\r\n");

			//���ô��ڵĲ�����
			BitRate=9600;

			//��LineCoding��ֵ����Ϊʵ�ʵ�����ֵ
			LineCoding[0]=BitRate&0xFF;
			LineCoding[1]=(BitRate>>8)&0xFF;
			LineCoding[2]=(BitRate>>16)&0xFF;
			LineCoding[3]=(BitRate>>24)&0xFF;

			//����ֻ֧��һֹͣλ����У�顢8λ����λ��
			//���Թ̶���Щ���ݡ�
			LineCoding[4]=0x00;
			LineCoding[5]=0x00;
			LineCoding[6]=0x08;
		}
		//����0���ȵ�״̬���ݰ���
		D12WriteEndpointBuffer(1,0,0);
	}
	else  //������������ݹ��̻���״̬����
	{
		D12ReadEndpointBuffer(0,16,Buffer);
		D12ClearBuffer();
	}
}



uint8_t x_point;
uint8_t y_point;
uint8_t sign;

void signed_create(void)
{
	static uint32_t tick;
	
	if(!tick)
	{
		tick = HAL_GetTick();
	}
	if((tick - HAL_GetTick())>1000)
	{
		tick = 0;
		sign = 1;
		if(x_point < 20)
		{
			x_point++;
		}
		if(y_point< 20)
		{
			y_point++;
		}
	}
}
/********************************************************************
�������ܣ����ݰ���������ر���ĺ�����
��ڲ������ޡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void SendReport(void)
{
	//��Ҫ���ص�4�ֽڱ���Ļ���
	//Buf[0]��D0���������D1�����Ҽ���D2�����м�������û�У�
	//Buf[1]ΪX�ᣬBuf[2]ΪY�ᣬBuf[3]Ϊ����
	uint8_t Buf[4]={0,0,0,0};
	
 	Buf[1] =	0;
 	Buf[2]=     0;
	
	//����׼�����ˣ�ͨ���˵�1���أ�����Ϊ4�ֽڡ�
	D12WriteEndpointBuffer(3,4,Buf);
	Ep1InIsBusy=1;  //���ö˵�æ��־��
}




void detect_intterrupt(void)
{
    uint8_t InterruptSource = 0;
    UsbDisconnect();
    UsbConnect();
    LOG("Device id : %.4x\r\n", D12ReadID());
    while (1)
    {
		//signed_create();
        if (D12GetIntPin() == 0)
        {
            D12WriteCommand(READ_INTERRUPT_REGISTER);
            InterruptSource = D12ReadByte();

            if (InterruptSource & 0x80)
            {
                UsbBusSuspend(); //���߹����ж�
            }
            else if (InterruptSource & 0x40)
            {
                UsbBusReset(); //���߸�λ�ж�
            }
            else if (InterruptSource & 0x01)
            {
                UsbEp0Out(); //�˵�0����ж�
            }
            else if (InterruptSource & 0x02)
            {
                UsbEp0In(); //�˵�0�����ж�
            }
            else if (InterruptSource & 0x04)
            {
                UsbEp1Out(); //�˵�0����ж�
            }
            else if (InterruptSource & 0x08)
            {
                UsbEp1In(); //�˵�0�����ж�
            }
            else if (InterruptSource & 0x10)
            {
                UsbEp2Out(); //�˵�0����ж�
            }
            else if (InterruptSource & 0x20)
            {
                UsbEp2In(); //�˵�0�����ж�
            }
		}
		if(ConfigValue!=0) //����Ѿ�����Ϊ��0�����ã�����Է��ر�������
		{
			if(!Ep2InIsBusy)  //����˵�2����û�д���æ״̬������Է�������
			{
				//������д�뵽�˵�2���뻺����
				D12WriteEndpointBuffer(5, 15,(uint8_t*)"this is test\r\n");
			
				//ֻ����������������ʱ�������ö˵�2����æ
				if ((D12ReadEndpointStatus(5) & 0x60) == 0x60)
				{
					Ep2InIsBusy = 1;
				} //���ú��������������ݷ��͵��˵�2
			}
			if(UsbEp2ByteCount != 0)
			{
				//����һ�ֽڵ�����
                printf("%d",UsbEp2Buffer[UsbEp2BufferOutputPoint]);
                UsbEp2BufferOutputPoint++; //����λ�ú���1
                UsbEp2ByteCount--;         //����ֵ��1
			}
		}
    }
}

void UsbEp0Out(void)
{
    LOG_CORE("USB�˵�0����жϡ�\r\n");
    //��ȡ�˵�0��������״̬���ò�������жϱ�־
    //���жϵ�5λ�Ƿ�Ϊ1������ǣ���˵���ǽ�����
    if (D12ReadEndpointLastStatus(0) & 0x20)
    {
        D12ReadEndpointBuffer(0, 16, Buffer); //��������������
        D12AcknowledgeSetup();                //Ӧ������
        D12ClearBuffer();                     //�建����
        //������������豸����ĸ��ֶ���
        bmRequestType = Buffer[0];
        bRequest = Buffer[1];
        wValue = Buffer[2] + (((uint16_t)Buffer[3]) << 8);
        wIndex = Buffer[4] + (((uint16_t)Buffer[5]) << 8);
        wLength = Buffer[6] + (((uint16_t)Buffer[7]) << 8);
        //����Ĵ����жϾ�������󣬲����ݲ�ͬ�����������ز���
        //���D7λΪ1����˵������������
        if ((bmRequestType & 0x80) == 0x80)
        {
            //����bmRequestType��D6~5λɢת��D6~5λ��ʾ���������
            //0Ϊ��׼����1Ϊ������2Ϊ��������
            switch ((bmRequestType >> 5) & 0x03)
            {
				case 0: //��׼����
				{
					LOG_CORE("USB��׼��������");
					//USBЭ�鶨���˼�����׼������������ʵ����Щ��׼���󼴿�
					//����Ĵ�����bRequest�У��Բ�ͬ������������ɢת
					//��ʵ�ϣ����ǻ���Ҫ�Խ����߽���ɢת����Ϊ��ͬ�����������
					//�ǲ�һ���ġ���������bmRequestType��D4~D0λ�ж��塣
					//��������Ϊ�˼򻯲�������Щ��ʡ���˶Խ����ߵ��жϡ�
					//�����ȡ������������ֻ����������������������
					switch (bRequest)
					{
						case GET_CONFIGURATION: //��ȡ����
						{
							LOG_CORE("��ȡ���á�\r\n");
							break;
						}
						case GET_DESCRIPTOR: //��ȡ������
						{
							LOG_CORE("��ȡ����������");
							//�����������ͽ���ɢת������ȫ���豸��
							//��׼����ֻ֧�ַ��͵��豸���豸�����á��ַ�������������
							switch ((wValue >> 8) & 0xFF)
							{
								case DEVICE_DESCRIPTOR: //�豸������
								{
									LOG_CORE("�豸��������\r\n");
									pSendData = DeviceDescriptor; //��Ҫ���͵�����
									//�ж�������ֽ����Ƿ��ʵ����Ҫ���͵��ֽ�����
									//������������豸��������������ݳ��Ⱦ���
									//DeviceDescriptor[0]���������ı�ʵ�ʵĳ���
									//��ôֻ����ʵ�ʳ��ȵ�����
									if (wLength > DeviceDescriptor[0])
									{
										SendLength = DeviceDescriptor[0];
										if (SendLength % DeviceDescriptor[7] == 0) //���Ҹպ������������ݰ�ʱ
										{
											NeedZeroPacket = 1; //��Ҫ����0���ȵ����ݰ�
										}
									}
									else
									{
										SendLength = wLength;
									}
									//������ͨ��EP0����
									UsbEp0SendData();
									break;
								}
								case CONFIGURATION_DESCRIPTOR: //����������
								{
									LOG("������������\r\n");
									pSendData = ConfigurationDescriptor; //��Ҫ���͵�����Ϊ����������
                                    //�ж�������ֽ����Ƿ��ʵ����Ҫ���͵��ֽ�����
                                    //������������������������ϣ�������ݳ��Ⱦ���
                                    //ConfigurationDescriptor[3]*256+ConfigurationDescriptor[2]��
                                    //�������ı�ʵ�ʵĳ�����ôֻ����ʵ�ʳ��ȵ�����
                                    SendLength = ConfigurationDescriptor[3];
                                    SendLength = SendLength * 256 + ConfigurationDescriptor[2];
                                    if (wLength > SendLength)
                                    {
                                        if (SendLength % DeviceDescriptor[7] == 0) //���Ҹպ������������ݰ�ʱ
                                        {
                                            NeedZeroPacket = 1; //��Ҫ����0���ȵ����ݰ�
                                        }
                                    }
                                    else
                                    {
                                        SendLength = wLength;
                                    }
                                    //������ͨ��EP0����
                                    UsbEp0SendData();
									break;
								}
								case STRING_DESCRIPTOR: //�ַ���������
								{
									LOG("�ַ���������");
									switch(wValue&0xFF)  //����wValue�ĵ��ֽڣ�����ֵ��ɢת
									{
										case 0:  //��ȡ����ID
											LOG("(����ID)��\r\n");
											pSendData=LanguageId;
											SendLength=LanguageId[0];
											break;

										case 1:  //�����ַ���������ֵΪ1����������Ϊ�����ַ���
											LOG("(��������)��\r\n");
											pSendData=ManufacturerStringDescriptor;
											SendLength=ManufacturerStringDescriptor[0];
											break;

										case 2:  //��Ʒ�ַ���������ֵΪ2����������Ϊ��Ʒ�ַ���
											LOG("(��Ʒ����)��\r\n");
											pSendData=ProductStringDescriptor;
											SendLength=ProductStringDescriptor[0];
											break;

										case 3:  //��Ʒ���кŵ�����ֵΪ3����������Ϊ���к�
											LOG("(��Ʒ���к�)��\r\n");
											pSendData=SerialNumberStringDescriptor;
											SendLength=SerialNumberStringDescriptor[0];
											break;

										default :
											LOG("(δ֪������ֵ)��\r\n");
											//����δ֪����ֵ�����󣬷���һ��0���ȵİ�
											SendLength=0;
											NeedZeroPacket=1;
											break;
									}
									//�ж�������ֽ����Ƿ��ʵ����Ҫ���͵��ֽ�����
									//�������ı�ʵ�ʵĳ�����ôֻ����ʵ�ʳ��ȵ�����
									if(wLength>SendLength)
									{
										if(SendLength%DeviceDescriptor[7]==0) //���Ҹպ������������ݰ�ʱ
										{
											NeedZeroPacket=1; //��Ҫ����0���ȵ����ݰ�
										}
									}
									else
									{
										SendLength=wLength;
									}
									//������ͨ��EP0����
									UsbEp0SendData();         
									break;
								}
								case REPORT_DESCRIPTOR: //����������
								{
									LOG("������������\r\n");
									pSendData = ReportDescriptor;
									SendLength = sizeof(ReportDescriptor);
									if(wLength > SendLength)
									{
										if(SendLength %DeviceDescriptor[7]==0)
										{
											NeedZeroPacket = 1;
										}
									}
									else
									{
										SendLength = wLength;
									}
									UsbEp0SendData();
									break;
								}
								default: //����������
								{
									LOG_CORE("���������������������룺");
									LOG_CORE("%d",(wValue >> 8) & 0xFF);
									LOG_CORE("\r\n");
									break;
								}
							}
							break;
						}

						case GET_INTERFACE: //��ȡ�ӿ�
						{
							LOG_CORE("��ȡ�ӿڡ�\r\n");
							break;
						}
						case GET_STATUS: //��ȡ״̬
						{
							LOG_CORE("��ȡ״̬��\r\n");
							break;
						}
						case SYNCH_FRAME: //ͬ��֡
						{
							LOG_CORE("ͬ��֡��\r\n");
							break;
						}
						default: //δ����ı�׼����
						{
							LOG_CORE("����δ����ı�׼��������\r\n");
							break;
						}
					}
					break;
				}
				case 1: //������
				{
					LOG_CORE("USB����������\r\n");
					switch(bRequest)
					{
						case GET_LINE_CODING: //GET_LINE_CODING����
							LOG("GET_LINE_CODING��\r\n");
						 
							SendLength=0x07; //7�ֽڵ�LineCoding
							pSendData=LineCoding;
							break;

						case SERIAL_STATE: //��ȡSERIAL_STATE����
							//�����������ǻ�ȡ����״̬�ģ�����ȦȦ��ʵ��ʹ���У�
							//������������δ���͹�������������ﲢ���������д���
							//ֻ�Ǽ򵥵ط���һ��0���ȵ����ݰ���
							LOG("SERIAL_STATE��\r\n");
							        
							SendLength=0;
							NeedZeroPacket=1;
							break;

						default:
							LOG("δ֪������\r\n");
							SendLength=0;
							NeedZeroPacket=1;
						break;
					}
					//�ж�������ֽ����Ƿ��ʵ����Ҫ���͵��ֽ�����
					//�������ı�ʵ�ʵĳ�����ôֻ����ʵ�ʳ��ȵ�����
					if(wLength>SendLength)
					{
						if(SendLength%DeviceDescriptor[7]==0) //���Ҹպ������������ݰ�ʱ
						{
							NeedZeroPacket=1; //��Ҫ����0���ȵ����ݰ�
						}
					}
					else
					{
						SendLength=wLength;
					}
					//������ͨ��EP0����
					UsbEp0SendData();
					break;
				}
				case 2: //��������
				{
					LOG_CORE("USB������������\r\n");
					break;
				}

				default: //δ�������������ֻ��ʾһ��������Ϣ��
				{
					LOG_CORE("����δ�������������\r\n");
					break;
				}
            }
        }
        //����˵�����������
        else //if(bmRequestType&0x80==0x80)֮else
        {
            //����bmRequestType��D6~5λɢת��D6~5λ��ʾ���������
            //0Ϊ��׼����1Ϊ������2Ϊ��������
            switch ((bmRequestType >> 5) & 0x03)
            {
				case 0: //��׼����
				{
					LOG_CORE("USB��׼�������");
					//USBЭ�鶨���˼�����׼�����������ʵ����Щ��׼���󼴿�
					//����Ĵ�����bRequest�У��Բ�ͬ������������ɢת
					switch (bRequest)
					{
						case CLEAR_FEATURE: //�������
						{
							LOG_CORE("������ԡ�\r\n");
							break;
						}
						case SET_ADDRESS: //���õ�ַ
						{
							LOG("���õ�ַ����ַΪ��");
							LOG("%d",wValue & 0xFF); //��ʾ�����õĵ�ַ
							LOG("\r\n");
							
							D12SetAddress(wValue&0xff);
							SendLength = 0;
							NeedZeroPacket = 1;
							UsbEp0SendData();
							break;
						}

						case SET_CONFIGURATION: //��������
						{
							LOG("�������á�\r\n");
							//ʹ�ܷ�0�˵㡣��0�˵�ֻ��������Ϊ��0�����ú����ʹ�ܡ�
						    //wValue�ĵ��ֽ�Ϊ���õ�ֵ�������ֵΪ��0������ʹ�ܷ�0�˵㡣
						    //���浱ǰ����ֵ
							ConfigValue=wValue&0xFF;
							D12SetEndpointEnable(ConfigValue);
							SendLength = 0;
							NeedZeroPacket = 1;
							//������ͨ��EP0����
							UsbEp0SendData();
							break;
						}

						case SET_DESCRIPTOR: //����������
						{
							LOG_CORE("������������\r\n");
							break;
						}

						case SET_FEATURE: //��������
						{
							LOG_CORE("�������ԡ�\r\n");
							break;
						}

						case SET_INTERFACE: //���ýӿ�
						{
							LOG_CORE("���ýӿڡ�\r\n");
							break;
						}

						default: //δ����ı�׼����
						{
							LOG_CORE("����δ����ı�׼�������\r\n");
							break;
						}
					}
					break;
				}

				case 1: //������
				{
					LOG("USB���������");
					switch (bRequest)
					{
//						case SET_IDLE:
//							LOG_CORE("���ÿ��С�\r\n");
//							//ֻ��Ҫ����һ��0���ȵ����ݰ�����
//							SendLength=0;
//							NeedZeroPacket=1;
//							//������ͨ��EP0����
//							UsbEp0SendData();
//							break;
						case SET_CONTROL_LINE_STATE:
							LOG("SET_CONTROL_LINE_STATE��\r\n");
							 
							//������û����������׶Σ�����wValue�ֶε�D0λ��ʾDTR��
							//D1λ��ʾRTS���������ǵİ��ϵĴ��ڲ�û���������ţ����
							//�Ը��������ǽ��Ǽ򵥵ط���һ��0���ȵ�״̬�������ݰ�����
							SendLength=0;
							NeedZeroPacket=1;
							//������ͨ��EP0����
							UsbEp0SendData();
							break;

						case SET_LINE_CODING:
							//���������ô��ڵ����ԣ�����ʵ�ʵ����ݲ��������ù��̷�����
							//������֮������ݹ��̷��������ﲻ�����κδ��������ݹ���
							//��ɺ󷵻�0���ȵ�״̬����
							LOG("SET_LINE_CODING��\r\n");
							break;
       
						default:
							LOG("δ֪����\r\n");
							break;
					}
					break;
				}

				case 2: //��������
				{
					LOG_CORE("USB�����������\r\n");
					break;
				}
				default: //δ�������������ֻ��ʾһ��������Ϣ��
				{
					LOG_CORE("����δ������������\r\n");
					break;
				}
            }
        }
    }
    //��ͨ�������
    else //if(D12ReadEndpointLastStatus(0)&0x20)֮else
    {
        UsbEp0DataOut();
    }
}
void UsbEp0In(void)
{
#ifdef DEBUG
    LOG("USB�˵�0�����жϡ�\r\n");
#endif
    //�������״̬���⽫����˵�0���жϱ�־λ
    D12ReadEndpointLastStatus(1);
    //����ʣ����ֽ���
    UsbEp0SendData();
}
void UsbEp1Out(void)
{
#ifdef DEBUG
    LOG("UsbEp1Out\r\n");
#endif
  //���˵����״̬���⽫����˵�1������жϱ�־λ
   D12ReadEndpointLastStatus(2);
   //����˵㻺����
   D12ClearBuffer();
}
void UsbEp1In(void)
{
#ifdef DEBUG
    LOG("UsbEp1In\r\n");
#endif
	 //�������״̬���⽫����˵�1������жϱ�־λ
	D12ReadEndpointLastStatus(3);
    //�˵�1���봦�ڿ���״̬
	Ep1InIsBusy=0;
}
void UsbEp2Out(void)
{
	#ifdef DEBUG
	LOG("USB�˵�2����жϡ�\r\n");
	#endif
	//����������е����ݻ�δͨ�����ڷ�����ϣ�����ʱ��������жϣ�ֱ�ӷ��ء�
	if(UsbEp2ByteCount!=0) return;

	/* �ɰ���룬��BUG���޸��ں�������ʱ����ж� 
	//��������״̬���⽫����˵�2������жϱ�־λ��
	//ע��˵�2�и�˫������ƣ�������ж�֮ǰ���ȼ���Ƿ�����������
	//�Ƿ�ȫ���ˣ��������������ȫ���Ļ����Ͳ�������жϱ�־��ֻ�е�
	//������������ȫ����ʱ�����Ҫ����жϱ�־��
	if((D12ReadEndpointStatus(4)&0x60)!=0x60)
	{
	D12ReadEndpointLastStatus(4);
	}
	*/

	//��ȡ�˵�2�����ݡ�����ֵΪʵ�ʶ����������ֽ���
	UsbEp2ByteCount=D12ReadEndpointBuffer(4,128,UsbEp2Buffer);
	//����˵㻺����
	D12ClearBuffer();

	//���λ����Ϊ0
	UsbEp2BufferOutputPoint=0;

	//�������������ж�û������ʱ����������жϱ�־
	if(!(D12ReadEndpointStatus(4)&0x60))
	{
		//�������״̬���⽫����˵�2������жϱ�־λ
		D12ReadEndpointLastStatus(4);
	}
}
void UsbEp2In(void)
{
#ifdef DEBUG
    LOG("UsbEp2In\r\n");
#endif
	//�������״̬���⽫����˵�2������жϱ�־λ
	D12ReadEndpointLastStatus(5);
	//�˵�2���봦�ڿ���״̬
	Ep2InIsBusy=0;
}



