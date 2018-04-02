#pragma once

#include <list>

//////////////////////////////////////////////////////////////////////////
//�ṹ����
typedef struct _PER_IO_OPERATION_DATA
{
	OVERLAPPED   Overlapped;
	WSABUF       WSABuff;
	SOCKET       socket;
	int          cbOperationType;
	char         cbDataBuff[4096];
}PER_IO_OPERATION_DATA,* LPPER_IO_OPERATION_DATA;

//////////////////////////////////////////////////////////////////////////

typedef std::list<LPPER_IO_OPERATION_DATA> CIOOperationItemList;
class CTCPNetWorkEngine;

//////////////////////////////////////////////////////////////////////////
class CIOCP
{
public:
	SOCKET                          m_hServerSocket;                    //���Ӿ��
	HANDLE                          m_lIOCPHandle;                      //���Ӿ��
	WORD							m_wServicePort;						//�����˿�
	CIOOperationItemList            m_IOSendOperationItemList;
	CIOOperationItemList            m_IORecvOperationItemList;
	CIOOperationItemList            m_IdleIOOperationItemList;

private:
	CIOCP(void);
	~CIOCP(void);

public:
	static CIOCP* GetInstance();
	static void Release();

public:
	//��������
	bool StartService();
	//ֹͣ����
	bool StopService();
	//���÷���
	void SetServiceParameter(WORD wServicePort);
	//��ȡ����
	LPPER_IO_OPERATION_DATA GetIOOperationItem(SOCKET Socket, BYTE cbOperationType);
	//�ر�����
	bool CloseSocket(SOCKET Socket);
	//��������
	bool PostSend(SOCKET Socket, CHAR szBuf[]);
	//��������
	bool PostRecv(SOCKET Socket);

private:
	//�����߳�
	static void AcceptWork(LPVOID pParam);
	//��д�߳�
	static void ReadWriteWork(LPVOID pParam);

private:
	//��ѯ����
	LPPER_IO_OPERATION_DATA QueryIOOperationItem(SOCKET Socket, BYTE cbOperationType);

private:
	static CIOCP* m_sInstance;
	static int m_sInstanceCount;

	CTCPNetWorkEngine*	m_pTcpNetWorkEngine;

};
