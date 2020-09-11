
#include "pdiusbd12.h"

enum
{
    D12_DATA_OUT,
    D12_DATA_IN
};

void D12_data_dir(uint8_t sta)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    if (sta)
    {
        GPIO_InitStruct.Pin = 0xFF;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else
    {
        GPIO_InitStruct.Pin = 0xFF;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void D12_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*A0 ---->PB1*/
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*WR----->PB11*/
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*RD----->PB10*/
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*INT---->PB0*/
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*DATA0-7----->PA0-7*/
    GPIO_InitStruct.Pin = 0xFF;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void D12SetData(uint8_t data)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, (GPIO_PinState)(data & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (GPIO_PinState)(data >> 1 & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, (GPIO_PinState)(data >> 2 & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, (GPIO_PinState)(data >> 3 & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (GPIO_PinState)(data >> 4 & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (GPIO_PinState)(data >> 5 & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, (GPIO_PinState)(data >> 6 & 0x01));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (GPIO_PinState)(data >> 7 & 0x01));
}
uint8_t D12GetData(void)
{
    uint8_t temp = 0;

    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) << 1;
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) << 2;
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) << 3;
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) << 4;
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) << 5;
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) << 6;
    temp |= HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) << 7;

    return temp;
}

/*--------------------------------------------------------------------*/

/********************************************************************
函数功能：D12写命令。
入口参数：Command：一字节命令。
返    回：无。
备    注：无。
********************************************************************/
void D12WriteCommand(uint8_t Command)
{
    D12SetCommandAddr(); //设置为命令地址
    D12ClrWr();          //WR置低
    D12SetPortOut();     //将数据口设置为输出状态（注意这里为空宏，移植时可能有用）
    D12SetData(Command); //输出命令到数据口上
    D12SetWr();          //WR置高
    D12SetPortIn();      //将数据口设置为输入状态，以备后面输入使用
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：读一字节D12数据。
入口参数：无。
返    回：读回的一字节。
备    注：无。
********************************************************************/
uint8_t D12ReadByte(void)
{
    uint8_t temp;
    D12SetDataAddr();    //设置为数据地址
    D12ClrRd();          //RD置低
    temp = D12GetData(); //读回数据
    D12SetRd();          //RD置高
    return temp;         //返回读到数据
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：读D12的ID。
入口参数：无。
返    回：D12的ID。
备    注：无。
********************************************************************/
uint16_t D12ReadID(void)
{
    uint16_t id;
    D12WriteCommand(Read_ID);             //写读ID命令
    id = D12ReadByte();                   //读回ID号低字节
    id |= ((uint16_t)D12ReadByte()) << 8; //读回ID号高字节
    return id;
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：写一字节D12数据。
入口参数：Value：要写的一字节数据。
返    回：无。
备    注：无。
********************************************************************/
void D12WriteByte(uint8_t Value)
{
    D12SetDataAddr();  //设置为数据地址
    D12ClrWr();        //WR置低
    D12SetPortOut();   //将数据口设置为输出状态（注意这里为空宏，移植时可能有用）
    D12SetData(Value); //写出数据
    D12SetWr();        //WR置高
    D12SetPortIn();    //将数据口设置为输入状态，以备后面输入使用
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：读取D12最后传输状态寄存器的函数。
入口参数：Endp：端点号。
返    回：端点的最后传输状态。
备    注：该操作将清除该端点的中断标志位。
********************************************************************/
uint8_t D12ReadEndpointLastStatus(uint8_t Endp)
{
    D12WriteCommand(0x40 + Endp); //读取端点最后状态的命令
    return D12ReadByte();
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：选择端点的函数，选择一个端点后才能对它进行数据操作。
入口参数：Endp：端点号。
返    回：无。
备    注：无。
********************************************************************/
void D12SelectEndpoint(uint8_t Endp)
{
    D12WriteCommand(0x00 + Endp); //选择端点的命令
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：清除接收端点缓冲区的函数。
入口参数：无。
返    回：无。
备    注：只有使用该函数清除端点缓冲后，该接收端点才能接收新的数据包。
********************************************************************/
void D12ClearBuffer(void)
{
    D12WriteCommand(D12_CLEAR_BUFFER);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：应答建立包的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void D12AcknowledgeSetup(void)
{
    D12SelectEndpoint(1);                   //选择端点0输入
    D12WriteCommand(D12_ACKNOWLEDGE_SETUP); //发送应答设置到端点0输入
    D12SelectEndpoint(0);                   //选择端点0输出
    D12WriteCommand(D12_ACKNOWLEDGE_SETUP); //发送应答设置到端点0输出
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：读取端点缓冲区函数。
入口参数：Endp：端点号；Len：需要读取的长度；Buf：保存数据的缓冲区。
返    回：实际读到的数据长度。
备    注：无。
********************************************************************/
uint8_t D12ReadEndpointBuffer(uint8_t Endp, uint8_t Len, uint8_t *Buf)
{
    uint8_t i, j;
    D12SelectEndpoint(Endp);          //选择要操作的端点缓冲
    D12WriteCommand(D12_READ_BUFFER); //发送读缓冲区的命令
    D12ReadByte();                    //该字节数据是保留的，不用。
    j = D12ReadByte();                //这里才是实际的接收到的数据长度
    if (j > Len)                      //如果要读的字节数比实际接收到的数据长
    {
        j = Len; //则只读指定的长度数据
    }
#ifdef DEBUG //如果定义了DEBUG，则需要显示调试信息
    printf("读端点");
    printf("%d ", Endp / 2); //端点号。由于D12特殊的端点组织形式，
                             //这里的0和1分别表示端点0的输出和输入；
                             //而2、3分别表示端点1的输出和输入；
                             //4、5分别表示端点2的输出和输入。
                             //因此要除以2才显示对应的端点。
    printf("缓冲区");
    printf("%d ", j); //实际读取的字节数
    printf("字节。\r\n");
#endif
    for (i = 0; i < j; i++)
    {
        //这里不直接调用读一字节的函数，而直接在这里模拟时序，可以节省时间
        D12ClrRd();                //RD置低
        *(Buf + i) = D12GetData(); //读一字节数据
        D12SetRd();                //RD置高
#ifdef DEBUG
        printf("0x%.2x ", *(Buf + i)); //如果需要显示调试信息，则显示读到的数据
        if (((i + 1) % 16) == 0)
            printf("\r\n"); //每16字节换行一次
#endif
    }
#ifdef DEBUG
    if ((j % 16) != 0)
        printf("\r\n"); //换行。
#endif
    return j; //返回实际读取的字节数。
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：使能发送端点缓冲区数据有效的函数。
入口参数：无。
返    回：无。
备    注：只有使用该函数使能发送端点数据有效之后，数据才能发送出去。
********************************************************************/
void D12ValidateBuffer(void)
{
    D12WriteCommand(D12_VALIDATE_BUFFER);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：将数据写入端点缓冲区函数。
入口参数：Endp：端点号；Len：需要发送的长度；Buf：保存数据的缓冲区。
返    回：Len的值。
备    注：无。
********************************************************************/
uint8_t D12WriteEndpointBuffer(uint8_t Endp, uint8_t Len, uint8_t *Buf)
{
    uint8_t i;
    D12SelectEndpoint(Endp);           //选择端点
    D12WriteCommand(D12_WRITE_BUFFER); //写Write Buffer命令
    D12WriteByte(0);                   //该字节必须写0
    D12WriteByte(Len);                 //写需要发送数据的长度

#ifdef DEBUG //如果定义了DEBUG，则需要显示调试信息
    printf("写端点");
    printf("%d ", Endp / 2); //端点号。由于D12特殊的端点组织形式，
                             //这里的0和1分别表示端点0的输出和输入；
                             //而2、3分别表示端点1的输出和输入；
                             //4、5分别表示端点2的输出和输入。
                             //因此要除以2才显示对应的端点。
    printf("缓冲区");
    printf("%d ", Len); //写入的字节数
    printf("字节。\r\n");
#endif
    D12SetPortOut(); //将数据口设置为输出状态（注意这里为空宏，移植时可能有用）
    for (i = 0; i < Len; i++)
    {
        //这里不直接调用写一字节的函数，而直接在这里模拟时序，可以节省时间
        D12ClrWr();             //WR置低
        D12SetData(*(Buf + i)); //将数据放到数据线上
        D12SetWr();             //WR置高，完成一字节写
#ifdef DEBUG
        printf("0x%.2x ", *(Buf + i)); //如果需要显示调试信息，则显示发送的数据
        if ((i + 1) % 16 == 0)
            printf("\r\n"); //每16字节换行一次
#endif
    }
#ifdef DEBUG
    if ((Len % 16) != 0)
        printf("\r\n"); //换行
#endif
    D12SetPortIn();      //数据口切换到输入状态
    D12ValidateBuffer(); //使端点数据有效
    return Len;          //返回Len
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：设置地址函数。
入口参数：Addr：要设置的地址值。
返    回：无。
备    注：无。
********************************************************************/
void D12SetAddress(uint8_t Addr)
{
    D12WriteCommand(D12_SET_ADDRESS_ENABLE); //写设置地址命令
    D12WriteByte(0x80 | Addr);               //写一字节数据：使能及地址
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：使能端点函数。
入口参数：Enable: 是否使能。0值为不使能，非0值为使能。
返    回：无。
备    注：无。
********************************************************************/
void D12SetEndpointEnable(uint8_t Enable)
{
    D12WriteCommand(D12_SET_ENDPOINT_ENABLE);
    if (Enable != 0)
    {
        D12WriteByte(0x01); //D0为1使能端点
    }
    else
    {
        D12WriteByte(0x00); //不使能端点
    }
}
/********************************************************************
函数功能：读取D12端点状态函数。
入口参数：Endp：端点号。
返    回：端点状态寄存器的值。
备    注：无。
********************************************************************/
uint8_t D12ReadEndpointStatus(uint8_t Endp)
{
	D12WriteCommand(0x80+Endp); //读取端点状态命令
	return D12ReadByte();
}


