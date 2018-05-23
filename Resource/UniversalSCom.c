/**
  ******************************************************************************
  * @file    /UniversalSCom.c 
  * @author  MMY
  * @version V1.0.0
  * @date    2017-4-5
  * @brief   ͨ�ô���
  * 
@verbatim  

@endverbatim        
  *
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2017 CQT Quartz. Co., Ltd.
  *
  ******************************************************************************
  */
	
	
	#include "UniversalSCom.h"
	
/** @brief  ����id��ȡ���ں�
  * @param  ID
  * @retval ���ں�
  * @note   
  */
CDV_INT08U GetComNoFromID(CDV_INT08U id)   
{ 
	CDV_INT08U no = 0;
	
	switch (id)
	{
		case 1:
			no = 6;
			break;
		case 2:
			no = 1;
		  break;
		default:
			break;
	}

 	return no;
}
	
struct COM_PARAM {
	u32 bound;
	u32 wordLength;
	u32 stopBits;
	u32 parity;
};
/** @brief  У���������õ�stm32ר������
  * @param  
  * @retval OPT_FAILURE ����������
  * @note   ��������stm32ר�õģ��ο�
λ��	��ż����	ֹͣλ
 7  	���ż  	0.5��1��1.5��2
 8  	�޼���  	0.5��1��1.5��2
 8  	���ż  	1
 9  	�޼���  	1
UART4��UART5��֧��0.5��1.5λ��ֹͣλ
  */
RET_STATUS CorrectComParam(struct COM_PARAM* param, u8 uartNo)
{ 
	RET_STATUS ret = OPT_SUCCESS;
	
	ASSERT(param);
	
  if(!IS_USART_BAUDRATE(param->bound))
	{
		param->bound = 9600;
	}
	
	#define EVEN_AND_ODD(PARITY) (((PARITY) == USART_Parity_Even) || \
                                 ((PARITY) == USART_Parity_Odd))

	switch (param->wordLength)
	{
		case 7:
			param->wordLength = EVEN_AND_ODD(param->parity) ? USART_WordLength_8b : 0x00;
			break;
		case 8:
			param->wordLength = EVEN_AND_ODD(param->parity) ? USART_WordLength_9b : USART_WordLength_8b;
			break;
		case 9:
			param->wordLength = EVEN_AND_ODD(param->parity) ? 0x00 : USART_WordLength_9b;
			break;
		default:
			ret = OPT_FAILURE;
			break;
	}
	
	switch (param->wordLength)
	{
		case 0x00:
			ret = OPT_FAILURE;
			break;
		case USART_WordLength_9b:
			if (param->stopBits != USART_StopBits_1)
			{
				ret = OPT_FAILURE;
			}
			break;
		default:
			break;
	}
	
	if (4 == uartNo || 5 == uartNo)
	{
		if (param->stopBits == USART_StopBits_0_5 
			|| param->stopBits == USART_StopBits_1_5)
		{
			ret = OPT_FAILURE;
		}
	}
	
	return ret;
}
	

/** @brief  ��������ͨѶ��������
  * @param  rxBuf  �����ַ���
	*         rxLen  ����
	*         uartNo ���������ĸ�����
  * @retval 
  * @note   
  */
RET_STATUS UniSerialComSetCmd(CDV_INT08U* rxBuf, CDV_INT08U rxLen, CMD_ARG *arg)   
{ 
	RET_STATUS ret       = OPT_FAILURE;
	CDV_INT08U no = 0;
	CDV_INT08U type = rxBuf[0];
	CDV_INT08U id = rxBuf[1];
	struct COM_PARAM param;
	param.bound = *(CDV_INT32U*)(rxBuf + 2);
	param.wordLength = *(CDV_INT32U*)(rxBuf + 6);
	param.stopBits = *(CDV_INT32U*)(rxBuf + 10);
	param.parity = *(CDV_INT32U*)(rxBuf + 14);
	
	switch (type)
	{
		case 0://����485
			no = GetComNoFromID(id);
		
		  if (OPT_SUCCESS == CorrectComParam(&param, no))
			  USARTSet(param.bound, param.wordLength, param.stopBits, param.parity, no);
			
			break;
		
		default:
			break;
	}
	
 	return ret;
}

/**
  * @brief  CSB ���� ת��
  *  
  * @param  ID     Ҫ������CSB�豸��
  *   
  * @retval void
  *
  * @note   
  */
RET_STATUS UniSerialSendCRC(const u8* txBuf, const u8 txLen,u8* rxBuf ,u8 rxbufLen,u8* rxLen , const CDV_INT08U uart)
{
	OS_ERR err;
	CDV_INT32S val = 0;
	u16 data;
	u8* sendBuf = NULL;
	
//	ASSERT(NULL != txBuf);
//	ASSERT(0 != txLen);
//	ASSERT(NULL != rxBuf);
//	ASSERT(0 != rxbufLen);
//	ASSERT(NULL != rxLen);
	
	if((NULL == txBuf) || (0 == txLen) || (NULL == rxBuf) || (0 == rxbufLen) || (NULL == rxLen))
		return OPT_FAILURE;
	
	NEW08U(sendBuf, txLen + 2);
	memcpy(sendBuf, txBuf, txLen);
	
	data=getCRC16(sendBuf,txLen);
	sendBuf[txLen]=data & 0x00ff;
  sendBuf[txLen + 1]=(data >> 8) & 0x00ff;

	USARTTR(sendBuf ,txLen + 2 ,rxBuf ,rxbufLen , rxLen , uart);
	
	DELETE(sendBuf);

	if(*rxLen > 2) {
		data=getCRC16(rxBuf,*rxLen-2);
		if((rxBuf[*rxLen-2]==(data & 0x00ff))
			&& (rxBuf[*rxLen-1]==((data >> 8) & 0x00ff))) //crc
		{
			return OPT_SUCCESS;
		}
		else
		{
			return OPT_FAILURE;
		}
	}
	else
	{
		return OPT_FAILURE;
	}
}


/** @brief  ͨ�ô���modbus����
  * @param  buf     �������ַ���ָ��
  *         len     �������ַ����ĳ���
  * @retval 
  * @note   ���ֶζ�����ο���ͨ������.docx��
  */
RET_STATUS UniSerialModbusParse(CDV_INT08U* buf, CDV_INT08U len, CDV_INT08U uart, CMD_ARG *arg)
{
	CDV_INT08U* cmdBuf = NULL;
	CDV_INT08U  cmdLen = 0;
	
	RET_STATUS ret       = OPT_FAILURE;	
	//CDV_INT08U native    = CascadeGetNativeNo(); //������
	CDV_INT08U devAddr   = buf[1];//����id
	CDV_INT08U res       = buf[2];//�˿ں�
	/*�����ֶ�*/
	CDV_INT08U ftmp1     = buf[3];
	CDV_INT08U ftmp2     = buf[4];
	
	CDV_INT08U fc        = buf[5];            /*������*/
	CDV_INT08U regPos    = buf[6];            /*�Ĵ���λ��*/
	CDV_INT08U datLen    = buf[7];            /*����λ����*/
	CDV_INT08U* p_dat    = buf + 8;           /*��������*/
	/*�����ֶ�*/
	CDV_INT08U btmp1     = buf[8 + (datLen?(len - 11):4)];//(datLen?datLen<<ftmp1:2)];//datLen:0����4B��1��ֵ��len-11��B
	CDV_INT08U prot      = buf[9 + (datLen?(len - 11):4)];   /*Э��*/
	CDV_INT08U* p_pp     = buf + 10 + (datLen?(len - 11):4); /*Э�����*/
	
	u8 recv_buf[100];
	u8 recv_len=0;

	CDV_INT08U num     = 0;            /*�������ݳ���*/
	CDV_INT08U* p_val  = NULL;              /*��������*/
	CDV_INT08U tmp[4] = {0};
	CDV_INT32S tmpVar = 0;
	CDV_INT08U uartNo;
	
	ASSERT(NULL != buf);
	ASSERT(0 != len);
	
	if(0x00 != prot)
		return ret;
	
	//ѡ��˿ں�
//	switch(res)
//	{
//		case 1:
//			uartNo = 6;
//			break;
//		case 2:
//			uartNo = 6;
//			break;
//		default:
//			uartNo = 6;
//			break;
//		
//	}
	uartNo = GetComNoFromID(res);
	
	if(!datLen) {                   /*0x00����*/
		num = ftmp1;
		tmpVar = VarGet(*(CDV_INT32U*)p_dat);
		tmp[0] = tmpVar >> 8;
		tmp[1] = tmpVar;
		tmp[2] = tmpVar >> 24;
		tmp[3] = tmpVar >> 16;
		p_val = tmp;
	} else {
		num = (len - 11)>>1;//ftmp1;//datLen>>1;
		p_val = p_dat;
	}
	
	switch(fc) {
		case 0x10:                        /*write reg*/
			WriteRegisterCmd(devAddr, regPos, num, p_val, &cmdBuf, &cmdLen);
			ret = UniSerialSendCRC(cmdBuf, cmdLen, recv_buf, 100, &recv_len, uartNo);
			break;
		case 0x03:                        /*read reg*/
			ReadRegisterCmd(devAddr, regPos, num, &cmdBuf, &cmdLen);
			ret = UniSerialSendCRC(cmdBuf, cmdLen, recv_buf, 100, &recv_len, uartNo);
			//���������ҽ��ճɹ�
			if(!datLen && OPT_SUCCESS == ret && devAddr == recv_buf[0] && 0x03 == recv_buf[1]) {
				ret = ReadRegReqToVar(recv_buf, recv_len, 0, *(CDV_INT32U*)p_dat);
			}
			break;
		case 0x04:                        /*read inreg*/
			ReadInRegisterCmd(devAddr, regPos, num, &cmdBuf, &cmdLen);
			ret = UniSerialSendCRC(cmdBuf, cmdLen, recv_buf, 100, &recv_len, uartNo);
			//���������ҽ��ճɹ�
			if(!datLen && OPT_SUCCESS == ret && devAddr == recv_buf[0] && 0x04 == recv_buf[1]) {
				ret = ReadInRegReqToVar(recv_buf, recv_len, 0, *(CDV_INT32U*)p_dat);
			}
			break;
		case 0x05:                        /*write coil*/
			WriteCoilCmd(devAddr, regPos, *p_val, &cmdBuf, &cmdLen);
			ret = UniSerialSendCRC(cmdBuf, cmdLen, recv_buf, 100, &recv_len, uartNo);
			break;
		case 0x02:                        /*read incoil*/
			ReadInCoilCmd(devAddr, regPos, num, &cmdBuf, &cmdLen);
			ret = UniSerialSendCRC(cmdBuf, cmdLen, recv_buf, 100, &recv_len, uartNo);
			//���������ҽ��ճɹ�
			if(!datLen && OPT_SUCCESS == ret && devAddr == recv_buf[0] && 0x02 == recv_buf[1]) {
				ret = ReadInCoilReqToVar(recv_buf, recv_len, 0, *(CDV_INT32U*)p_dat);
			}
			break;
		case 0x01:                        /*read coil*/
			ReadCoilCmd(devAddr, regPos, num, &cmdBuf, &cmdLen);
			ret = UniSerialSendCRC(cmdBuf, cmdLen, recv_buf, 100, &recv_len, uartNo);
			//���������ҽ��ճɹ�
			if(!datLen && OPT_SUCCESS == ret && devAddr == recv_buf[0] && 0x01 == recv_buf[1]) {
				ret = ReadCoilReqToVar(recv_buf, recv_len, 0, *(CDV_INT32U*)p_dat);
			}
			break;
		default:
			break;
	}
	//�ظ����������ֵ
	if(!datLen)
	{
		tmpVar = VarGet(*(CDV_INT32U*)p_dat);
		ResRequest(arg->buf, arg->len, (CDV_INT08U*)&tmpVar, 4, arg);
	}
	else
	{
		ResRequest(arg->buf, arg->len, 0, 0, arg);
	}
	DELETE(cmdBuf);
	return ret;
}