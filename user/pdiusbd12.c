
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
�������ܣ�D12д���
��ڲ�����Command��һ�ֽ����
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void D12WriteCommand(uint8_t Command)
{
    D12SetCommandAddr(); //����Ϊ�����ַ
    D12ClrWr();          //WR�õ�
    D12SetPortOut();     //�����ݿ�����Ϊ���״̬��ע������Ϊ�պ꣬��ֲʱ�������ã�
    D12SetData(Command); //���������ݿ���
    D12SetWr();          //WR�ø�
    D12SetPortIn();      //�����ݿ�����Ϊ����״̬���Ա���������ʹ��
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ���һ�ֽ�D12���ݡ�
��ڲ������ޡ�
��    �أ����ص�һ�ֽڡ�
��    ע���ޡ�
********************************************************************/
uint8_t D12ReadByte(void)
{
    uint8_t temp;
    D12SetDataAddr();    //����Ϊ���ݵ�ַ
    D12ClrRd();          //RD�õ�
    temp = D12GetData(); //��������
    D12SetRd();          //RD�ø�
    return temp;         //���ض�������
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ���D12��ID��
��ڲ������ޡ�
��    �أ�D12��ID��
��    ע���ޡ�
********************************************************************/
uint16_t D12ReadID(void)
{
    uint16_t id;
    D12WriteCommand(Read_ID);             //д��ID����
    id = D12ReadByte();                   //����ID�ŵ��ֽ�
    id |= ((uint16_t)D12ReadByte()) << 8; //����ID�Ÿ��ֽ�
    return id;
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�дһ�ֽ�D12���ݡ�
��ڲ�����Value��Ҫд��һ�ֽ����ݡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void D12WriteByte(uint8_t Value)
{
    D12SetDataAddr();  //����Ϊ���ݵ�ַ
    D12ClrWr();        //WR�õ�
    D12SetPortOut();   //�����ݿ�����Ϊ���״̬��ע������Ϊ�պ꣬��ֲʱ�������ã�
    D12SetData(Value); //д������
    D12SetWr();        //WR�ø�
    D12SetPortIn();    //�����ݿ�����Ϊ����״̬���Ա���������ʹ��
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ���ȡD12�����״̬�Ĵ����ĺ�����
��ڲ�����Endp���˵�š�
��    �أ��˵�������״̬��
��    ע���ò���������ö˵���жϱ�־λ��
********************************************************************/
uint8_t D12ReadEndpointLastStatus(uint8_t Endp)
{
    D12WriteCommand(0x40 + Endp); //��ȡ�˵����״̬������
    return D12ReadByte();
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�ѡ��˵�ĺ�����ѡ��һ���˵����ܶ����������ݲ�����
��ڲ�����Endp���˵�š�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void D12SelectEndpoint(uint8_t Endp)
{
    D12WriteCommand(0x00 + Endp); //ѡ��˵������
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�������ն˵㻺�����ĺ�����
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��ֻ��ʹ�øú�������˵㻺��󣬸ý��ն˵���ܽ����µ����ݰ���
********************************************************************/
void D12ClearBuffer(void)
{
    D12WriteCommand(D12_CLEAR_BUFFER);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�Ӧ�������ĺ�����
��ڲ������ޡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void D12AcknowledgeSetup(void)
{
    D12SelectEndpoint(1);                   //ѡ��˵�0����
    D12WriteCommand(D12_ACKNOWLEDGE_SETUP); //����Ӧ�����õ��˵�0����
    D12SelectEndpoint(0);                   //ѡ��˵�0���
    D12WriteCommand(D12_ACKNOWLEDGE_SETUP); //����Ӧ�����õ��˵�0���
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ���ȡ�˵㻺����������
��ڲ�����Endp���˵�ţ�Len����Ҫ��ȡ�ĳ��ȣ�Buf���������ݵĻ�������
��    �أ�ʵ�ʶ��������ݳ��ȡ�
��    ע���ޡ�
********************************************************************/
uint8_t D12ReadEndpointBuffer(uint8_t Endp, uint8_t Len, uint8_t *Buf)
{
    uint8_t i, j;
    D12SelectEndpoint(Endp);          //ѡ��Ҫ�����Ķ˵㻺��
    D12WriteCommand(D12_READ_BUFFER); //���Ͷ�������������
    D12ReadByte();                    //���ֽ������Ǳ����ģ����á�
    j = D12ReadByte();                //�������ʵ�ʵĽ��յ������ݳ���
    if (j > Len)                      //���Ҫ�����ֽ�����ʵ�ʽ��յ������ݳ�
    {
        j = Len; //��ֻ��ָ���ĳ�������
    }
#ifdef DEBUG //���������DEBUG������Ҫ��ʾ������Ϣ
    printf("���˵�");
    printf("%d ", Endp / 2); //�˵�š�����D12����Ķ˵���֯��ʽ��
                             //�����0��1�ֱ��ʾ�˵�0����������룻
                             //��2��3�ֱ��ʾ�˵�1����������룻
                             //4��5�ֱ��ʾ�˵�2����������롣
                             //���Ҫ����2����ʾ��Ӧ�Ķ˵㡣
    printf("������");
    printf("%d ", j); //ʵ�ʶ�ȡ���ֽ���
    printf("�ֽڡ�\r\n");
#endif
    for (i = 0; i < j; i++)
    {
        //���ﲻֱ�ӵ��ö�һ�ֽڵĺ�������ֱ��������ģ��ʱ�򣬿��Խ�ʡʱ��
        D12ClrRd();                //RD�õ�
        *(Buf + i) = D12GetData(); //��һ�ֽ�����
        D12SetRd();                //RD�ø�
#ifdef DEBUG
        printf("0x%.2x ", *(Buf + i)); //�����Ҫ��ʾ������Ϣ������ʾ����������
        if (((i + 1) % 16) == 0)
            printf("\r\n"); //ÿ16�ֽڻ���һ��
#endif
    }
#ifdef DEBUG
    if ((j % 16) != 0)
        printf("\r\n"); //���С�
#endif
    return j; //����ʵ�ʶ�ȡ���ֽ�����
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�ʹ�ܷ��Ͷ˵㻺����������Ч�ĺ�����
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��ֻ��ʹ�øú���ʹ�ܷ��Ͷ˵�������Ч֮�����ݲ��ܷ��ͳ�ȥ��
********************************************************************/
void D12ValidateBuffer(void)
{
    D12WriteCommand(D12_VALIDATE_BUFFER);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�������д��˵㻺����������
��ڲ�����Endp���˵�ţ�Len����Ҫ���͵ĳ��ȣ�Buf���������ݵĻ�������
��    �أ�Len��ֵ��
��    ע���ޡ�
********************************************************************/
uint8_t D12WriteEndpointBuffer(uint8_t Endp, uint8_t Len, uint8_t *Buf)
{
    uint8_t i;
    D12SelectEndpoint(Endp);           //ѡ��˵�
    D12WriteCommand(D12_WRITE_BUFFER); //дWrite Buffer����
    D12WriteByte(0);                   //���ֽڱ���д0
    D12WriteByte(Len);                 //д��Ҫ�������ݵĳ���

#ifdef DEBUG //���������DEBUG������Ҫ��ʾ������Ϣ
    printf("д�˵�");
    printf("%d ", Endp / 2); //�˵�š�����D12����Ķ˵���֯��ʽ��
                             //�����0��1�ֱ��ʾ�˵�0����������룻
                             //��2��3�ֱ��ʾ�˵�1����������룻
                             //4��5�ֱ��ʾ�˵�2����������롣
                             //���Ҫ����2����ʾ��Ӧ�Ķ˵㡣
    printf("������");
    printf("%d ", Len); //д����ֽ���
    printf("�ֽڡ�\r\n");
#endif
    D12SetPortOut(); //�����ݿ�����Ϊ���״̬��ע������Ϊ�պ꣬��ֲʱ�������ã�
    for (i = 0; i < Len; i++)
    {
        //���ﲻֱ�ӵ���дһ�ֽڵĺ�������ֱ��������ģ��ʱ�򣬿��Խ�ʡʱ��
        D12ClrWr();             //WR�õ�
        D12SetData(*(Buf + i)); //�����ݷŵ���������
        D12SetWr();             //WR�øߣ����һ�ֽ�д
#ifdef DEBUG
        printf("0x%.2x ", *(Buf + i)); //�����Ҫ��ʾ������Ϣ������ʾ���͵�����
        if ((i + 1) % 16 == 0)
            printf("\r\n"); //ÿ16�ֽڻ���һ��
#endif
    }
#ifdef DEBUG
    if ((Len % 16) != 0)
        printf("\r\n"); //����
#endif
    D12SetPortIn();      //���ݿ��л�������״̬
    D12ValidateBuffer(); //ʹ�˵�������Ч
    return Len;          //����Len
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ����õ�ַ������
��ڲ�����Addr��Ҫ���õĵ�ֵַ��
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void D12SetAddress(uint8_t Addr)
{
    D12WriteCommand(D12_SET_ADDRESS_ENABLE); //д���õ�ַ����
    D12WriteByte(0x80 | Addr);               //дһ�ֽ����ݣ�ʹ�ܼ���ַ
}
////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ�ʹ�ܶ˵㺯����
��ڲ�����Enable: �Ƿ�ʹ�ܡ�0ֵΪ��ʹ�ܣ���0ֵΪʹ�ܡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void D12SetEndpointEnable(uint8_t Enable)
{
    D12WriteCommand(D12_SET_ENDPOINT_ENABLE);
    if (Enable != 0)
    {
        D12WriteByte(0x01); //D0Ϊ1ʹ�ܶ˵�
    }
    else
    {
        D12WriteByte(0x00); //��ʹ�ܶ˵�
    }
}
/********************************************************************
�������ܣ���ȡD12�˵�״̬������
��ڲ�����Endp���˵�š�
��    �أ��˵�״̬�Ĵ�����ֵ��
��    ע���ޡ�
********************************************************************/
uint8_t D12ReadEndpointStatus(uint8_t Endp)
{
	D12WriteCommand(0x80+Endp); //��ȡ�˵�״̬����
	return D12ReadByte();
}


