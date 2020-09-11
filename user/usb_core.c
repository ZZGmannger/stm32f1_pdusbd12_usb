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

uint8_t DeviceDescriptor[0x12] = //18字节设备描述符
{
    0x12, //bLength  字段。设备描述符的长度为18 字节
    0x01, //bDescriptorType 字段，设备描述符的编号为0x01

    0x10, //bcd USB 版本，小端模式
    0x01,
//bDeviceClass字段。本设备必须在设备描述符中指定设备的类型，
//否则，由于在配置集合中有两个接口，就会被系统认为是一个USB
//复合设备，从而导致设备工作不正常。0x02为通信设备类的类代码。
    0x02, //bDeviceClass 设备类
    0x00, //bDeviceSubClass
    0x00, //bDeviceProtocol
    0x10, //bMaxPacketSize0 字段 PD12USBD12 的端点0大小为16字节

    0x88, //厂商ID ,需要购买
    0x88,
	
//idProduct字段。产品ID号，由于是第七个实验，我们这里取0x0007。
//注意小端模式，低字节应该在前。
    0x06, //idProduct 字段 产品ID ，第一个实验，所以为0x0001
    0x00,

    0x00, //bcd device字段 USB鼠标v1.0版本 0x0100
    0x01,

    0x01, //iManufacturer 厂商字符串的索引值，为了方便记忆管理，字符串索引就从1开始
    0x02, //iProduct 产品字符串的索引值
    0x03, //iSerialNumber 设备序列号字符串索引
    0x01  //bNumConfigurations
};


//USB报告描述符的定义
uint8_t ReportDescriptor[] =
{
    //每行开始的第一字节为该条目的前缀，前缀的格式为：
    //D7~D4：bTag。D3~D2：bType；D1~D0：bSize。以下分别对每个条目注释。

    //这是一个全局（bType为1）条目，选择用途页为普通桌面Generic Desktop Page(0x01)
    //后面跟一字节数据（bSize为1），后面的字节数就不注释了，
    //自己根据bSize来判断。
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)

    //这是一个局部（bType为2）条目，说明接下来的应用集合用途用于鼠标
    0x09, 0x02, // USAGE (Mouse)

    //这是一个主条目（bType为0）条目，开集合，后面跟的数据0x01表示
    //该集合是一个应用集合。它的性质在前面由用途页和用途定义为
    //普通桌面用的鼠标。
    0xa1, 0x01, // COLLECTION (Application)

    //这是一个局部条目。说明用途为指针集合
    0x09, 0x01, //   USAGE (Pointer)

    //这是一个主条目，开集合，后面跟的数据0x00表示该集合是一个
    //物理集合，用途由前面的局部条目定义为指针集合。
    0xa1, 0x00, //   COLLECTION (Physical)

    //这是一个全局条目，选择用途页为按键（Button Page(0x09)）
    0x05, 0x09, //     USAGE_PAGE (Button)

    //这是一个局部条目，说明用途的最小值为1。实际上是鼠标左键。
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)

    //这是一个局部条目，说明用途的最大值为3。实际上是鼠标中键。
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)

    //这是一个全局条目，说明返回的数据的逻辑值（就是我们返回的数据域的值啦）
    //最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
    0x15, 0x00, //     LOGICAL_MINIMUM (0)

    //这是一个全局条目，说明逻辑值最大为1。
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)

    //这是一个全局条目，说明数据域的数量为三个。
    0x95, 0x03, //     REPORT_COUNT (3)

    //这是一个全局条目，说明每个数据域的长度为1个bit。
    0x75, 0x01, //     REPORT_SIZE (1)

    //这是一个主条目，说明有3个长度为1bit的数据域（数量和长度
    //由前面的两个全局条目所定义）用来做为输入，
    //属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
    //这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
    //这样定义的结果就是，第一个数据域bit0表示按键1（左键）是否按下，
    //第二个数据域bit1表示按键2（右键）是否按下，第三个数据域bit2表示
    //按键3（中键）是否按下。
    0x81, 0x02, //     INPUT (Data,Var,Abs)

    //这是一个全局条目，说明数据域数量为1个
    0x95, 0x01, //     REPORT_COUNT (1)

    //这是一个全局条目，说明每个数据域的长度为5bit。
    0x75, 0x05, //     REPORT_SIZE (5)

    //这是一个主条目，输入用，由前面两个全局条目可知，长度为5bit，
    //数量为1个。它的属性为常量（即返回的数据一直是0）。
    //这个只是为了凑齐一个字节（前面用了3个bit）而填充的一些数据
    //而已，所以它是没有实际用途的。
    0x81, 0x03, //     INPUT (Cnst,Var,Abs)

    //这是一个全局条目，选择用途页为普通桌面Generic Desktop Page(0x01)
    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)

    //这是一个局部条目，说明用途为X轴
    0x09, 0x30, //     USAGE (X)

    //这是一个局部条目，说明用途为Y轴
    0x09, 0x31, //     USAGE (Y)

    //这是一个局部条目，说明用途为滚轮
    0x09, 0x38, //     USAGE (Wheel)

    //下面两个为全局条目，说明返回的逻辑最小和最大值。
    //因为鼠标指针移动时，通常是用相对值来表示的，
    //相对值的意思就是，当指针移动时，只发送移动量。
    //往右移动时，X值为正；往下移动时，Y值为正。
    //对于滚轮，当滚轮往上滚时，值为正。
    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f, //     LOGICAL_MAXIMUM (127)

    //这是一个全局条目，说明数据域的长度为8bit。
    0x75, 0x08, //     REPORT_SIZE (8)

    //这是一个全局条目，说明数据域的个数为3个。
    0x95, 0x03, //     REPORT_COUNT (3)

    //这是一个主条目。它说明这三个8bit的数据域是输入用的，
    //属性为：Data,Var,Rel。Data说明数据是可以变的，Var说明
    //这些数据域是独立的，即第一个8bit表示X轴，第二个8bit表示
    //Y轴，第三个8bit表示滚轮。Rel表示这些值是相对值。
    0x81, 0x06, //     INPUT (Data,Var,Rel)

    //下面这两个主条目用来关闭前面的集合用。
    //我们开了两个集合，所以要关两次。bSize为0，所以后面没数据。
    0xc0, //   END_COLLECTION
    0xc0  // END_COLLECTION
};
//USB配置描述符集合的定义
//配置描述符总长度为9+9+9+7字节
uint8_t ConfigurationDescriptor[9+9+5+5+4+5+7+9+7+7] =
{
   /***************配置描述符***********************/
 //bLength字段。配置描述符的长度为9字节。
 0x09,
 
 //bDescriptorType字段。配置描述符编号为0x02。
 0x02,
 
 //wTotalLength字段。配置描述符集合的总长度，
 //包括配置描述符本身、接口描述符、类描述符、端点描述符等。
 sizeof(ConfigurationDescriptor)&0xFF, //低字节
 (sizeof(ConfigurationDescriptor)>>8)&0xFF, //高字节
 
 //bNumInterfaces字段。该配置包含的接口数，有两个接口。
 0x02,
 
 //bConfiguration字段。该配置的值为1。
 0x01,
 
 //iConfigurationz字段，该配置的字符串索引。这里没有，为0。
 0x00,
 
 //bmAttributes字段，该设备的属性。由于我们的板子是总线供电的，
 //并且我们不想实现远程唤醒的功能，所以该字段的值为0x80。
 0x80,
 
 //bMaxPower字段，该设备需要的最大电流量。由于我们的板子
 //需要的电流不到100mA，因此我们这里设置为100mA。由于每单位
 //电流为2mA，所以这里设置为50(0x32)。
 0x32,
 
 /*******************CDC类接口描述符*********************/
 //bLength字段。接口描述符的长度为9字节。
 0x09,
 
 //bDescriptorType字段。接口描述符的编号为0x04。
 0x04,
 
 //bInterfaceNumber字段。该接口的编号，第一个接口，编号为0。
 0x00,
 
 //bAlternateSetting字段。该接口的备用编号，为0。
 0x00,
 
 //bNumEndpoints字段。非0端点的数目。CDC接口只使用一个中断
 //输入端点。
 0x01,
 
 //bInterfaceClass字段。该接口所使用的类。CDC类的类代码为0x02。
 0x02,
 
 //bInterfaceSubClass字段。该接口所使用的子类。要实现USB转串口，
 //就必须使用Abstract Control Model（抽象控制模型）子类。它的
 //编号为0x02。
 0x02,
 
 //bInterfaceProtocol字段。使用Common AT Commands（通用AT命令）
 //协议。该协议的编号为0x01。
 0x01,
 
 //iConfiguration字段。该接口的字符串索引值。这里没有，为0。
 0x00,
 
 /***************以下为功能描述符****************/
 /********* Header Functional Descriptor ********/
 //bFunctionLength字段。该描述符长度为5字节
 0x05,
 
 //bDescriptorType字段。描述符类型为类特殊接口（CS_INTERFACE）
 //编号为0x24。
 0x24,
 
 //bDescriptorSubtype字段。描述符子类为Header Functional Descriptor
 //编号为0x00。
 0x00,
 
 //bcdCDC字段。CDC版本号，为0x0110（低字节在先）
 0x10,
 0x01,
 
 /**** Call Management Functional Descriptor ****/
 //bFunctionLength字段。该描述符长度为5字节
 0x05,
 
 //bDescriptorType字段。描述符类型为类特殊接口（CS_INTERFACE）
 //编号为0x24。
 0x24,
 
 //bDescriptorSubtype字段。描述符子类为Call Management 
 //functional descriptor，编号为0x01。
 0x01,
 
 //bmCapabilities字段。设备自己不管理call management
 0x00,
 
 //bDataInterface字段。没有数据类接口用作call management
 0x00,

 /*** Abstract Control Management Functional Descriptor ***/
 //bFunctionLength字段。该描述符长度为4字节
 0x04,
 
 //bDescriptorType字段。描述符类型为类特殊接口（CS_INTERFACE）
 //编号为0x24。
 0x24,
 
 //bDescriptorSubtype字段。描述符子类为Abstract Control 
 //Management functional descriptor，编号为0x02。
 0x02,

 //bmCapabilities字段。支持Set_Line_Coding、Set_Control_Line_State、
 //Get_Line_Coding请求和Serial_State通知
 0x02,

 /***  Union Functional Descriptor  **/
 //bFunctionLength字段。该描述符长度为5字节。 
 0x05,

 //bDescriptorType字段。描述符类型为类特殊接口（CS_INTERFACE）
 //编号为0x24。
 0x24,
 
 //bDescriptorSubtype字段。描述符子类为
 //Union functional descriptor，编号为0x06。
 0x06,
 
 //MasterInterface字段。这里为前面编号为0的CDC接口。
 0x00,
 
 //SlaveInterface字段，这里为接下来编号为1的数据类接口。
 0x01,

 /***********  以下为接口0的端点描述符  *******/
 //bLength字段。端点描述符长度为7字节。
 0x07,
 
 //bDescriptorType字段。端点描述符编号为0x05。
 0x05,
 
 //bEndpointAddress字段。端点的地址。这里使用D12的输入端点1。
 //D7位表示数据方向，输入端点D7为1。所以输入端点1的地址为0x81。
 0x81,
 
 //bmAttributes字段。D1~D0为端点传输类型选择。
 //该端点为中断端点。中断端点的编号为3。其它位保留为0。
 0x03,
 
 //wMaxPacketSize字段。该端点的最大包长。端点1的最大包长为16字节。
 //注意低字节在先。
 0x10,
 0x00,
 
 //bInterval字段。端点查询的时间，这里设置为10个帧时间，即10ms。
 0x0A,
 
 /*********  以下为接口1（数据接口）的接口描述符  *********/
 //bLength字段。接口描述符的长度为9字节。
 0x09,
 
 //bDescriptorType字段。接口描述符的编号为0x04。
 0x04,
 
 //bInterfaceNumber字段。该接口的编号，第二个接口，编号为1。
 0x01,
 
 //bAlternateSetting字段。该接口的备用编号，为0。
 0x00,
 
 //bNumEndpoints字段。非0端点的数目。该设备需要使用一对批量端点，设置为2。
 0x02,
 
 //bInterfaceClass字段。该接口所使用的类。数据类接口的代码为0x0A。
 0x0A,
 
 //bInterfaceSubClass字段。该接口所使用的子类为0。
 0x00,
 
 //bInterfaceProtocol字段。该接口所使用的协议为0。
 0x00,
 
 //iConfiguration字段。该接口的字符串索引值。这里没有，为0。
 0x00,
 
 /*****  以下为接口1（数据类接口）的端点描述符  *****/
 /*************** 批量输入端点2描述符 ******************/
 //bLength字段。端点描述符长度为7字节。
 0x07,
 
 //bDescriptorType字段。端点描述符编号为0x05。
 0x05,
 
 //bEndpointAddress字段。端点的地址。我们使用D12的输入端点2。
 //D7位表示数据方向，输入端点D7为1。所以输入端点2的地址为0x82。
 0x82,
 
 //bmAttributes字段。D1~D0为端点传输类型选择。
 //该端点为批量端点，批量端点的编号为0x02。其它位保留为0。
 0x02,
 
 //wMaxPacketSize字段。该端点的最大包长。端点2的最大包长为64字节。
 //注意低字节在先。
 0x40,
 0x00,
 
 //bInterval字段。端点查询的时间，这里对批量端点无效。
 0x00,
 
 /*************** 批量输出端点2描述符 ******************/
 //bLength字段。端点描述符长度为7字节。
 0x07,
 
 //bDescriptorType字段。端点描述符编号为0x05。
 0x05,
 
 //bEndpointAddress字段。端点的地址。我们使用D12的输出端点2。
 //D7位表示数据方向，输出端点D7为0。所以输出端点2的地址为0x02。
 0x02,
 
 //bmAttributes字段。D1~D0为端点传输类型选择。
 //该端点为批量端点，批量端点的编号为0x02。其它位保留为0。
 0x02,
 
 //wMaxPacketSize字段。该端点的最大包长。端点2的最大包长为64字节。
 //注意低字节在先。
 0x40,
 0x00,
 
 //bInterval字段。端点查询的时间，这里对批量端点无效。
 0x00
};




/************************语言ID的定义********************/
uint8_t LanguageId[4]=
{
 0x04, //本描述符的长度
 0x03, //字符串描述符
 //0x0409为美式英语的ID
 0x09,
 0x04
};
////////////////////////语言ID完毕//////////////////////////////////

/**************************************************/
/*********        本转换结果来自         **********/
/********* Http://computer00.21ic.org    **********/
/*********        作者: 电脑圈圈         **********/
/*********         欢迎大家使用          **********/
/*********    版权所有，盗版请写明出处   **********/
/**************************************************/

//http://computer00.21ic.org/user1/2198/archives/2007/42769.html
//字符串“电脑圈圈的USB专区 Http://group.ednchina.com/93/”的Unicode编码
//8位小端格式
uint8_t ManufacturerStringDescriptor[82]={
82,         //该描述符的长度为82字节
0x03,       //字符串描述符的类型编码为0x03
0x35, 0x75, //电
0x11, 0x81, //脑
0x08, 0x57, //圈
0x08, 0x57, //圈
0x84, 0x76, //的
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x13, 0x4e, //专
0x3a, 0x53, //区
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
/////////////////////////厂商字符串结束/////////////////////////////

//字符串“《圈圈教你玩USB》之USB鼠标”的Unicode编码
//8位小端格式
uint8_t ProductStringDescriptor[34]={
34,         //该描述符的长度为34字节
0x03,       //字符串描述符的类型编码为0x03
0x0a, 0x30, //《
0x08, 0x57, //圈
0x08, 0x57, //圈
0x59, 0x65, //教
0x60, 0x4f, //你
0xa9, 0x73, //玩
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x0b, 0x30, //》
0x4b, 0x4e, //之
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x20, 0x9f, //鼠
0x07, 0x68  //标
};
////////////////////////产品字符串结束////////////////////////////

//字符串“2008-07-07”的Unicode编码
//8位小端格式
uint8_t SerialNumberStringDescriptor[22]={
22,         //该描述符的长度为22字节
0x03,       //字符串描述符的类型编码为0x03
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
//////////////////////产品序列号字符串结束/////////////////////////



uint8_t Buffer[16]; //读端点0用的缓冲区
//USB设备请求的个字段
uint8_t bmRequestType;
uint8_t bRequest;
uint16_t wValue;
uint16_t wIndex;
uint16_t wLength;
//当前发送数据的位置
uint8_t *pSendData;
//需要发送数据的长度
uint16_t SendLength;
//是否需要发送0数据包的标志。在USB控制传输的数据过程中，
//当返回的数据包字节数少于最大包长时，会认为数据过程结束。
//当请求的字节数比实际需要返回的字节数长，而实际返回的字节
//数又刚好是端点0大小的整数倍时，就需要返回一个0长度的数据包
//来结束数据过程。因此这里增加一个标志，供程序决定是否需要返回
//一个0长度的数据包。
uint8_t NeedZeroPacket;

uint8_t ConfigValue;
uint8_t Ep1InIsBusy;


//端点2缓冲是否忙的标志。当缓冲区中有数据时，该标志为真。
//当缓冲区中空闲时，该标志为假。
uint8_t Ep2InIsBusy;

//LineCoding数组，用来保存波特率、停止位等串口属性。
//初始化波特率为9600，1停止位，无校验，8数据位。
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
	Ep1InIsBusy=0; //复位后端点1输入缓冲区空闲。
	Ep2InIsBusy=0; //复位后端点2输入缓冲区空闲。
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
    //将数据写到端点中去准备发送
    //写之前要先判断一下需要发送的数据是否比端点0
    //最大长度大，如果超过端点大小，则一次只能发送
    //最大包长的数据。端点0的最大包长在DeviceDescriptor[7]
    if (SendLength > DeviceDescriptor[7])
    {
        //按最大包长度发送
        D12WriteEndpointBuffer(1, DeviceDescriptor[7], pSendData);
        //发送后剩余字节数减少最大包长
        SendLength -= DeviceDescriptor[7];
        //发送一次后指针位置要调整
        pSendData += DeviceDescriptor[7];
    }
    else
    {
        if (SendLength != 0)
        {
            //不够最大包长，可以直接发送
            D12WriteEndpointBuffer(1, SendLength, pSendData);
            //发送完毕后，SendLength长度变为0
            SendLength = 0;
        }
        else //如果要发送的数据包长度为0
        {
            if (NeedZeroPacket == 1) //如果需要发送0长度数据
            {
                D12WriteEndpointBuffer(1, 0, pSendData); //发送0长度数据包
                NeedZeroPacket = 0;                      //清需要发送0长度数据包标志
            }
        }
    }
}

/********************************************************************
函数功能：USB端点0数据过程数据处理函数。
入口参数：无。
返    回：无。
备    注：该函数用来处理0端点控制传输的数据或状态过程。
********************************************************************/
void UsbEp0DataOut(void)
{
	//由于本程序中只有一个请求输出数据，所以可以直接使用if语句判断条件，
	//如果有很多请求的话，使用if语句就不方便了，而应该使用switch语句散转。
	if((bmRequestType==0x21)&&(bRequest==SET_LINE_CODING))
	{
		uint32_t BitRate;
		uint8_t Length;

		//读回7字节的LineCoding值
		Length=D12ReadEndpointBuffer(0,7,LineCoding);
		D12ClearBuffer(); //清除缓冲区

		if(Length==7) //如果长度正确
		{
			//从LineCoding计算设置的波特率
			BitRate=LineCoding[3];
			BitRate=(BitRate<<8)+LineCoding[2];
			BitRate=(BitRate<<8)+LineCoding[1];
			BitRate=(BitRate<<8)+LineCoding[0];

			LOG("波特率设置为：");
			LOG("%d",BitRate);
			LOG("bps\r\n");

			//设置串口的波特率
			BitRate=9600;

			//将LineCoding的值设置为实际的设置值
			LineCoding[0]=BitRate&0xFF;
			LineCoding[1]=(BitRate>>8)&0xFF;
			LineCoding[2]=(BitRate>>16)&0xFF;
			LineCoding[3]=(BitRate>>24)&0xFF;

			//由于只支持一停止位、无校验、8位数据位，
			//所以固定这些数据。
			LineCoding[4]=0x00;
			LineCoding[5]=0x00;
			LineCoding[6]=0x08;
		}
		//返回0长度的状态数据包。
		D12WriteEndpointBuffer(1,0,0);
	}
	else  //其它请求的数据过程或者状态过程
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
函数功能：根据按键情况返回报告的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendReport(void)
{
	//需要返回的4字节报告的缓冲
	//Buf[0]的D0就是左键，D1就是右键，D2就是中键（这里没有）
	//Buf[1]为X轴，Buf[2]为Y轴，Buf[3]为滚轮
	uint8_t Buf[4]={0,0,0,0};
	
 	Buf[1] =	0;
 	Buf[2]=     0;
	
	//报告准备好了，通过端点1返回，长度为4字节。
	D12WriteEndpointBuffer(3,4,Buf);
	Ep1InIsBusy=1;  //设置端点忙标志。
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
                UsbBusSuspend(); //总线挂起中断
            }
            else if (InterruptSource & 0x40)
            {
                UsbBusReset(); //总线复位中断
            }
            else if (InterruptSource & 0x01)
            {
                UsbEp0Out(); //端点0输出中断
            }
            else if (InterruptSource & 0x02)
            {
                UsbEp0In(); //端点0输入中断
            }
            else if (InterruptSource & 0x04)
            {
                UsbEp1Out(); //端点0输出中断
            }
            else if (InterruptSource & 0x08)
            {
                UsbEp1In(); //端点0输入中断
            }
            else if (InterruptSource & 0x10)
            {
                UsbEp2Out(); //端点0输出中断
            }
            else if (InterruptSource & 0x20)
            {
                UsbEp2In(); //端点0输入中断
            }
		}
		if(ConfigValue!=0) //如果已经设置为非0的配置，则可以返回报告数据
		{
			if(!Ep2InIsBusy)  //如果端点2输入没有处于忙状态，则可以发送数据
			{
				//将数据写入到端点2输入缓冲区
				D12WriteEndpointBuffer(5, 15,(uint8_t*)"this is test\r\n");
			
				//只有两个缓冲区都满时，才设置端点2输入忙
				if ((D12ReadEndpointStatus(5) & 0x60) == 0x60)
				{
					Ep2InIsBusy = 1;
				} //调用函数将缓冲区数据发送到端点2
			}
			if(UsbEp2ByteCount != 0)
			{
				//发送一字节到串口
                printf("%d",UsbEp2Buffer[UsbEp2BufferOutputPoint]);
                UsbEp2BufferOutputPoint++; //发送位置后移1
                UsbEp2ByteCount--;         //计数值减1
			}
		}
    }
}

void UsbEp0Out(void)
{
    LOG_CORE("USB端点0输出中断。\r\n");
    //读取端点0输出最后传输状态，该操作清除中断标志
    //并判断第5位是否为1，如果是，则说明是建立包
    if (D12ReadEndpointLastStatus(0) & 0x20)
    {
        D12ReadEndpointBuffer(0, 16, Buffer); //读建立过程数据
        D12AcknowledgeSetup();                //应答建立包
        D12ClearBuffer();                     //清缓冲区
        //将缓冲数据填到设备请求的各字段中
        bmRequestType = Buffer[0];
        bRequest = Buffer[1];
        wValue = Buffer[2] + (((uint16_t)Buffer[3]) << 8);
        wIndex = Buffer[4] + (((uint16_t)Buffer[5]) << 8);
        wLength = Buffer[6] + (((uint16_t)Buffer[7]) << 8);
        //下面的代码判断具体的请求，并根据不同的请求进行相关操作
        //如果D7位为1，则说明是输入请求
        if ((bmRequestType & 0x80) == 0x80)
        {
            //根据bmRequestType的D6~5位散转，D6~5位表示请求的类型
            //0为标准请求，1为类请求，2为厂商请求。
            switch ((bmRequestType >> 5) & 0x03)
            {
				case 0: //标准请求
				{
					LOG_CORE("USB标准输入请求：");
					//USB协议定义了几个标准输入请求，我们实现这些标准请求即可
					//请求的代码在bRequest中，对不同的请求代码进行散转
					//事实上，我们还需要对接收者进行散转，因为不同的请求接收者
					//是不一样的。接收者在bmRequestType的D4~D0位中定义。
					//我们这里为了简化操作，有些就省略了对接收者的判断。
					//例如获取描述符的请求，只根据描述符的类型来区别。
					switch (bRequest)
					{
						case GET_CONFIGURATION: //获取配置
						{
							LOG_CORE("获取配置。\r\n");
							break;
						}
						case GET_DESCRIPTOR: //获取描述符
						{
							LOG_CORE("获取描述符——");
							//对描述符类型进行散转，对于全速设备，
							//标准请求只支持发送到设备的设备、配置、字符串三种描述符
							switch ((wValue >> 8) & 0xFF)
							{
								case DEVICE_DESCRIPTOR: //设备描述符
								{
									LOG_CORE("设备描述符。\r\n");
									pSendData = DeviceDescriptor; //需要发送的数据
									//判断请求的字节数是否比实际需要发送的字节数多
									//这里请求的是设备描述符，因此数据长度就是
									//DeviceDescriptor[0]。如果请求的比实际的长，
									//那么只返回实际长度的数据
									if (wLength > DeviceDescriptor[0])
									{
										SendLength = DeviceDescriptor[0];
										if (SendLength % DeviceDescriptor[7] == 0) //并且刚好是整数个数据包时
										{
											NeedZeroPacket = 1; //需要返回0长度的数据包
										}
									}
									else
									{
										SendLength = wLength;
									}
									//将数据通过EP0返回
									UsbEp0SendData();
									break;
								}
								case CONFIGURATION_DESCRIPTOR: //配置描述符
								{
									LOG("配置描述符。\r\n");
									pSendData = ConfigurationDescriptor; //需要发送的数据为配置描述符
                                    //判断请求的字节数是否比实际需要发送的字节数多
                                    //这里请求的是配置描述符集合，因此数据长度就是
                                    //ConfigurationDescriptor[3]*256+ConfigurationDescriptor[2]。
                                    //如果请求的比实际的长，那么只返回实际长度的数据
                                    SendLength = ConfigurationDescriptor[3];
                                    SendLength = SendLength * 256 + ConfigurationDescriptor[2];
                                    if (wLength > SendLength)
                                    {
                                        if (SendLength % DeviceDescriptor[7] == 0) //并且刚好是整数个数据包时
                                        {
                                            NeedZeroPacket = 1; //需要返回0长度的数据包
                                        }
                                    }
                                    else
                                    {
                                        SendLength = wLength;
                                    }
                                    //将数据通过EP0返回
                                    UsbEp0SendData();
									break;
								}
								case STRING_DESCRIPTOR: //字符串描述符
								{
									LOG("字符串描述符");
									switch(wValue&0xFF)  //根据wValue的低字节（索引值）散转
									{
										case 0:  //获取语言ID
											LOG("(语言ID)。\r\n");
											pSendData=LanguageId;
											SendLength=LanguageId[0];
											break;

										case 1:  //厂商字符串的索引值为1，所以这里为厂商字符串
											LOG("(厂商描述)。\r\n");
											pSendData=ManufacturerStringDescriptor;
											SendLength=ManufacturerStringDescriptor[0];
											break;

										case 2:  //产品字符串的索引值为2，所以这里为产品字符串
											LOG("(产品描述)。\r\n");
											pSendData=ProductStringDescriptor;
											SendLength=ProductStringDescriptor[0];
											break;

										case 3:  //产品序列号的索引值为3，所以这里为序列号
											LOG("(产品序列号)。\r\n");
											pSendData=SerialNumberStringDescriptor;
											SendLength=SerialNumberStringDescriptor[0];
											break;

										default :
											LOG("(未知的索引值)。\r\n");
											//对于未知索引值的请求，返回一个0长度的包
											SendLength=0;
											NeedZeroPacket=1;
											break;
									}
									//判断请求的字节数是否比实际需要发送的字节数多
									//如果请求的比实际的长，那么只返回实际长度的数据
									if(wLength>SendLength)
									{
										if(SendLength%DeviceDescriptor[7]==0) //并且刚好是整数个数据包时
										{
											NeedZeroPacket=1; //需要返回0长度的数据包
										}
									}
									else
									{
										SendLength=wLength;
									}
									//将数据通过EP0返回
									UsbEp0SendData();         
									break;
								}
								case REPORT_DESCRIPTOR: //报告描述符
								{
									LOG("报告描述符。\r\n");
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
								default: //其它描述符
								{
									LOG_CORE("其他描述符，描述符代码：");
									LOG_CORE("%d",(wValue >> 8) & 0xFF);
									LOG_CORE("\r\n");
									break;
								}
							}
							break;
						}

						case GET_INTERFACE: //获取接口
						{
							LOG_CORE("获取接口。\r\n");
							break;
						}
						case GET_STATUS: //获取状态
						{
							LOG_CORE("获取状态。\r\n");
							break;
						}
						case SYNCH_FRAME: //同步帧
						{
							LOG_CORE("同步帧。\r\n");
							break;
						}
						default: //未定义的标准请求
						{
							LOG_CORE("错误：未定义的标准输入请求。\r\n");
							break;
						}
					}
					break;
				}
				case 1: //类请求
				{
					LOG_CORE("USB类输入请求：\r\n");
					switch(bRequest)
					{
						case GET_LINE_CODING: //GET_LINE_CODING请求
							LOG("GET_LINE_CODING。\r\n");
						 
							SendLength=0x07; //7字节的LineCoding
							pSendData=LineCoding;
							break;

						case SERIAL_STATE: //获取SERIAL_STATE请求
							//本来该请求是获取串口状态的，但是圈圈在实际使用中，
							//发现主机从来未发送过该请求，因而这里并不对它进行处理，
							//只是简单地发送一个0长度的数据包。
							LOG("SERIAL_STATE。\r\n");
							        
							SendLength=0;
							NeedZeroPacket=1;
							break;

						default:
							LOG("未知类请求。\r\n");
							SendLength=0;
							NeedZeroPacket=1;
						break;
					}
					//判断请求的字节数是否比实际需要发送的字节数多
					//如果请求的比实际的长，那么只返回实际长度的数据
					if(wLength>SendLength)
					{
						if(SendLength%DeviceDescriptor[7]==0) //并且刚好是整数个数据包时
						{
							NeedZeroPacket=1; //需要返回0长度的数据包
						}
					}
					else
					{
						SendLength=wLength;
					}
					//将数据通过EP0返回
					UsbEp0SendData();
					break;
				}
				case 2: //厂商请求
				{
					LOG_CORE("USB厂商输入请求：\r\n");
					break;
				}

				default: //未定义的请求。这里只显示一个报错信息。
				{
					LOG_CORE("错误：未定义的输入请求。\r\n");
					break;
				}
            }
        }
        //否则说明是输出请求
        else //if(bmRequestType&0x80==0x80)之else
        {
            //根据bmRequestType的D6~5位散转，D6~5位表示请求的类型
            //0为标准请求，1为类请求，2为厂商请求。
            switch ((bmRequestType >> 5) & 0x03)
            {
				case 0: //标准请求
				{
					LOG_CORE("USB标准输出请求：");
					//USB协议定义了几个标准输出请求，我们实现这些标准请求即可
					//请求的代码在bRequest中，对不同的请求代码进行散转
					switch (bRequest)
					{
						case CLEAR_FEATURE: //清除特性
						{
							LOG_CORE("清除特性。\r\n");
							break;
						}
						case SET_ADDRESS: //设置地址
						{
							LOG("设置地址。地址为：");
							LOG("%d",wValue & 0xFF); //显示所设置的地址
							LOG("\r\n");
							
							D12SetAddress(wValue&0xff);
							SendLength = 0;
							NeedZeroPacket = 1;
							UsbEp0SendData();
							break;
						}

						case SET_CONFIGURATION: //设置配置
						{
							LOG("设置配置。\r\n");
							//使能非0端点。非0端点只有在设置为非0的配置后才能使能。
						    //wValue的低字节为配置的值，如果该值为非0，才能使能非0端点。
						    //保存当前配置值
							ConfigValue=wValue&0xFF;
							D12SetEndpointEnable(ConfigValue);
							SendLength = 0;
							NeedZeroPacket = 1;
							//将数据通过EP0返回
							UsbEp0SendData();
							break;
						}

						case SET_DESCRIPTOR: //设置描述符
						{
							LOG_CORE("设置描述符。\r\n");
							break;
						}

						case SET_FEATURE: //设置特性
						{
							LOG_CORE("设置特性。\r\n");
							break;
						}

						case SET_INTERFACE: //设置接口
						{
							LOG_CORE("设置接口。\r\n");
							break;
						}

						default: //未定义的标准请求
						{
							LOG_CORE("错误：未定义的标准输出请求。\r\n");
							break;
						}
					}
					break;
				}

				case 1: //类请求
				{
					LOG("USB类输出请求：");
					switch (bRequest)
					{
//						case SET_IDLE:
//							LOG_CORE("设置空闲。\r\n");
//							//只需要返回一个0长度的数据包即可
//							SendLength=0;
//							NeedZeroPacket=1;
//							//将数据通过EP0返回
//							UsbEp0SendData();
//							break;
						case SET_CONTROL_LINE_STATE:
							LOG("SET_CONTROL_LINE_STATE。\r\n");
							 
							//该请求没有数据输出阶段，其中wValue字段的D0位表示DTR，
							//D1位表示RTS。但是我们的板上的串口并没有这两引脚，因而
							//对该请求我们仅是简单地返回一个0长度的状态过程数据包即可
							SendLength=0;
							NeedZeroPacket=1;
							//将数据通过EP0返回
							UsbEp0SendData();
							break;

						case SET_LINE_CODING:
							//该请求设置串口的属性，但是实际的数据并不在设置过程发出，
							//而是在之后的数据过程发出。这里不用做任何处理，在数据过程
							//完成后返回0长度的状态包。
							LOG("SET_LINE_CODING。\r\n");
							break;
       
						default:
							LOG("未知请求。\r\n");
							break;
					}
					break;
				}

				case 2: //厂商请求
				{
					LOG_CORE("USB厂商输出请求：\r\n");
					break;
				}
				default: //未定义的请求。这里只显示一个报错信息。
				{
					LOG_CORE("错误：未定义的输出请求。\r\n");
					break;
				}
            }
        }
    }
    //普通数据输出
    else //if(D12ReadEndpointLastStatus(0)&0x20)之else
    {
        UsbEp0DataOut();
    }
}
void UsbEp0In(void)
{
#ifdef DEBUG
    LOG("USB端点0输入中断。\r\n");
#endif
    //读最后发送状态，这将清除端点0的中断标志位
    D12ReadEndpointLastStatus(1);
    //发送剩余的字节数
    UsbEp0SendData();
}
void UsbEp1Out(void)
{
#ifdef DEBUG
    LOG("UsbEp1Out\r\n");
#endif
  //读端点最后状态，这将清除端点1输出的中断标志位
   D12ReadEndpointLastStatus(2);
   //清除端点缓冲区
   D12ClearBuffer();
}
void UsbEp1In(void)
{
#ifdef DEBUG
    LOG("UsbEp1In\r\n");
#endif
	 //读最后发送状态，这将清除端点1输入的中断标志位
	D12ReadEndpointLastStatus(3);
    //端点1输入处于空闲状态
	Ep1InIsBusy=0;
}
void UsbEp2Out(void)
{
	#ifdef DEBUG
	LOG("USB端点2输出中断。\r\n");
	#endif
	//如果缓冲区中的数据还未通过串口发送完毕，则暂时不处理该中断，直接返回。
	if(UsbEp2ByteCount!=0) return;

	/* 旧版代码，有BUG。修改在函数返回时清除中断 
	//读最后接收状态，这将清除端点2输出的中断标志位。
	//注意端点2有个双缓冲机制，在清除中断之前，先检查是否两个缓冲区
	//是否全满了，如果两个缓冲区全满的话，就不用清除中断标志。只有当
	//两个缓冲区不全满的时候才需要清除中断标志。
	if((D12ReadEndpointStatus(4)&0x60)!=0x60)
	{
	D12ReadEndpointLastStatus(4);
	}
	*/

	//读取端点2的数据。返回值为实际读到的数据字节数
	UsbEp2ByteCount=D12ReadEndpointBuffer(4,128,UsbEp2Buffer);
	//清除端点缓冲区
	D12ClearBuffer();

	//输出位置设为0
	UsbEp2BufferOutputPoint=0;

	//当两个缓冲区中都没有数据时，才能清除中断标志
	if(!(D12ReadEndpointStatus(4)&0x60))
	{
		//读最后发送状态，这将清除端点2输入的中断标志位
		D12ReadEndpointLastStatus(4);
	}
}
void UsbEp2In(void)
{
#ifdef DEBUG
    LOG("UsbEp2In\r\n");
#endif
	//读最后发送状态，这将清除端点2输入的中断标志位
	D12ReadEndpointLastStatus(5);
	//端点2输入处于空闲状态
	Ep2InIsBusy=0;
}



