
#pragma once

struct TCP_PacketHead
{
	WORD			wMainCmd;			//Э������
	WORD			wSubCmd;			//Э������
	DWORD			nDataPacketSize;	//Э�����ݰ���С
}; 

struct TCP_DataPacket
{

	TCP_PacketHead		TcpHead;		//Э��ͷ
	BYTE				cbBuff[4096];	//Э������
};