#include "StdAfx.h"
#include "flashaccredit.h"
#include "..\Include\InitSocket.h"
#include <assert.h>
#include "TCPNetWorkEngine.h"

//////////////////////////////////////////////////////////////////////////
//�궨��
#define RECV_POSTED      101
#define SEND_POSTED      102


CInitSocket g_InitSocket;

CIOCP* CIOCP::m_sInstance = NULL;
int CIOCP::m_sInstanceCount = 0;

//////////////////////////////////////////////////////////////////////////
CIOCP::CIOCP(void)
{
	m_hServerSocket = INVALID_SOCKET;
	m_lIOCPHandle = INVALID_HANDLE_VALUE;
	m_wServicePort = 843;
	m_pTcpNetWorkEngine = NULL;
	m_IOSendOperationItemList.clear();
	m_IORecvOperationItemList.clear();
	m_IdleIOOperationItemList.clear();
}

CIOCP::~CIOCP(void)
{
	StopService();
	WSACleanup();
}

//����ģʽ
CIOCP* CIOCP::GetInstance()
{
	if (m_sInstanceCount == 0)
	{
		m_sInstance = new CIOCP;

		++m_sInstanceCount;
	}

	return m_sInstance;
}

void CIOCP::Release()
{
	if (m_sInstanceCount != 0)
	{
		delete m_sInstance;
	}
}

//��������
bool CIOCP::StartService()
{
	//��������
	SOCKADDR_IN SocketAddr;
	ZeroMemory(&SocketAddr,sizeof(SocketAddr));

	m_pTcpNetWorkEngine = CTCPNetWorkEngine::GetInstance();

	//��������
	SocketAddr.sin_family=AF_INET;
	SocketAddr.sin_addr.s_addr=INADDR_ANY;
	SocketAddr.sin_port=htons(m_wServicePort);
	m_hServerSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);

	//�����ж�
	if (m_hServerSocket==INVALID_SOCKET) 
	{
		LPCTSTR pszString=TEXT("ϵͳ��Դ������� TCP/IP Э��û�а�װ����������ʧ��");
		printf(pszString);
		return false;
	}

	//������
	if (bind(m_hServerSocket,(SOCKADDR*)&SocketAddr,sizeof(SocketAddr))==SOCKET_ERROR)
	{
		LPCTSTR pszString=TEXT("����󶨷���������������ʧ��");
		printf(pszString);
		return false;
	}

	//�����˿�
	if (listen(m_hServerSocket,200)==SOCKET_ERROR)
	{
		TCHAR szString[512]=TEXT("");
		_sntprintf(szString,CountArray(szString),TEXT("�˿�������������ռ�ã����� %d �˿�ʧ��"), m_wServicePort);
		printf(szString);
		return false;
	}

	//ϵͳ��Ϣ
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	//��ɶ˿�
	m_lIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
	if(m_lIOCPHandle == NULL)
	{
		LPCTSTR pszString=TEXT("����������Դʧ�ܣ���������ʧ��");
		printf(pszString);
		return false;
	}

	//��SOCKET
	if(NULL == CreateIoCompletionPort((HANDLE)m_hServerSocket, m_lIOCPHandle, 0, 0))
	{
		LPCTSTR pszString=TEXT("�󶨼���SOCKETʧ�ܣ���������ʧ��");
		printf(pszString);
		return false;
	}

	//���������߳�
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AcceptWork, (LPVOID)this, 0, 0);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadWriteWork, (LPVOID)this, 0, 0);

	return true;
}

//ֹͣ����
bool CIOCP::StopService()
{
	//�����¼
	
	for (auto pos = m_IORecvOperationItemList.begin(); pos != m_IORecvOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata= (*pos);
		if(pIOOperdata->socket)
		{
			//�ر�����
			closesocket(pIOOperdata->socket);
			delete pIOOperdata;
			pIOOperdata = NULL;
		}
	}
	m_IORecvOperationItemList.clear();

	//�����¼
	
	for (auto pos = m_IOSendOperationItemList.begin(); pos != m_IOSendOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata=(*pos);
		if(pIOOperdata->socket)
		{
			//�ر�����
			closesocket(pIOOperdata->socket);
			delete pIOOperdata;
			pIOOperdata = NULL;
		}
	}
	m_IOSendOperationItemList.clear();

	//�����¼
	for (auto pos = m_IdleIOOperationItemList.begin(); pos != m_IdleIOOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata=(*pos);
		if(pIOOperdata->socket)
		{
			//�ر�����
			closesocket(pIOOperdata->socket);
			delete pIOOperdata;
			pIOOperdata = NULL;
		}
	}
	m_IdleIOOperationItemList.clear();

	return true;
}

//���÷���
void CIOCP::SetServiceParameter(WORD wServicePort)
{
	m_wServicePort=wServicePort;
}

//��ѯ����
LPPER_IO_OPERATION_DATA CIOCP::QueryIOOperationItem(SOCKET Socket, BYTE cbOperationType)
{
	//����Ч��
	assert(cbOperationType == SEND_POSTED || cbOperationType == RECV_POSTED);
	if (cbOperationType != SEND_POSTED && cbOperationType != RECV_POSTED) return NULL;

	CIOOperationItemList QueryList = (RECV_POSTED == cbOperationType ? m_IORecvOperationItemList : m_IOSendOperationItemList);

	for (auto pos = QueryList.begin(); pos != QueryList.end(); ++pos)
	{
		if ((*pos)->socket == Socket)
		{
			return *pos;
		}
	}

	return NULL;

}

//��ȡ����
LPPER_IO_OPERATION_DATA CIOCP::GetIOOperationItem(SOCKET Socket, BYTE cbOperationType)
{
	//����Ч��
	assert(cbOperationType==SEND_POSTED || cbOperationType==RECV_POSTED);
	if(cbOperationType!=SEND_POSTED && cbOperationType!=RECV_POSTED) return NULL;

	//�������
	LPPER_IO_OPERATION_DATA pIOOperdata = NULL;


	//��ѯbuff
	if (RECV_POSTED == cbOperationType || SEND_POSTED == cbOperationType)
	{
		LPPER_IO_OPERATION_DATA pIOData = QueryIOOperationItem(Socket, cbOperationType);
		if (NULL != pIOData)
		{
			return pIOData;
		}
	}

	//����buff
	if(m_IdleIOOperationItemList.size() > 0)
	{
		pIOOperdata = (*m_IdleIOOperationItemList.begin());

		m_IdleIOOperationItemList.pop_front();
	}
	else //���仺��
	{
		pIOOperdata = new PER_IO_OPERATION_DATA;
		if( pIOOperdata == NULL) return NULL;
	}

	//���û���
	pIOOperdata->socket = Socket;
	memset(&(pIOOperdata->Overlapped),0,sizeof(OVERLAPPED));
	memset(pIOOperdata->cbDataBuff,0,sizeof(pIOOperdata->cbDataBuff));
	pIOOperdata->WSABuff.len = sizeof(pIOOperdata->cbDataBuff);
	pIOOperdata->WSABuff.buf = pIOOperdata->cbDataBuff;
	pIOOperdata->cbOperationType = cbOperationType;

	//��¼����
	if(cbOperationType == RECV_POSTED)
	{
		m_IORecvOperationItemList.push_back(pIOOperdata);
	}
	else if(cbOperationType == SEND_POSTED)
	{
		m_IOSendOperationItemList.push_back(pIOOperdata);
	}

	return pIOOperdata;
}

//�ر�����
bool CIOCP::CloseSocket(SOCKET Socket)
{
	//�ر�����
	closesocket(Socket);

	//�����¼
	for (auto pos = m_IORecvOperationItemList.begin();
		pos != m_IORecvOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA tempPos = (*pos);
		if(tempPos->socket == Socket)
		{
			m_IORecvOperationItemList.erase(pos);
			m_IdleIOOperationItemList.push_back(tempPos);
			break;
		}
	}

	//�����¼
	//pos=m_IOSendOperationItemList.GetHeadPosition();
	for (auto pos = m_IOSendOperationItemList.begin();
		pos != m_IOSendOperationItemList.end(); ++pos)
	{
		LPPER_IO_OPERATION_DATA pIOOperdata=(*pos);
		if(pIOOperdata->socket == Socket)
		{
			m_IOSendOperationItemList.erase(pos);
			m_IdleIOOperationItemList.push_back(pIOOperdata);
			break;
		}
	}

	return true;
}

//�����߳�
void CIOCP::AcceptWork(LPVOID pParam)
{
	//��������
	CIOCP * pFlashAccredit = (CIOCP *)pParam;

	//�߳�ѭ��
	while(true)
	{
		//�����û����ӣ�������ɶ˿ڹ���
		SOCKET sockAccept = WSAAccept(pFlashAccredit->m_hServerSocket,NULL,NULL,NULL,0);
		if (INVALID_SOCKET == sockAccept)
		{
			printf("��Чsocket \n");
			continue;
		}

		//���仺��
		LPPER_IO_OPERATION_DATA pIOOperdata = pFlashAccredit->GetIOOperationItem(sockAccept, RECV_POSTED);
		if (pIOOperdata == NULL)
		{
			printf("�ڴ��ȡʧ��\n");
			continue;
		}

		//����SOCKET����ɶ˿�
		CreateIoCompletionPort((HANDLE)sockAccept,pFlashAccredit->m_lIOCPHandle,NULL,1);

		//Ͷ�ݽ��ղ���
		/*DWORD dwRecvBytes=0;
		DWORD dwFlags=0;
		INT nRet = WSARecv(pIOOperdata->socket,&(pIOOperdata->WSABuff),1,&dwRecvBytes,&dwFlags,&(pIOOperdata->Overlapped),NULL);
		if(nRet==SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			pFlashAccredit->CloseSocket(sockAccept);
			continue;
		}*/

		bool bRet = pFlashAccredit->PostRecv(sockAccept);
		if (!bRet)
		{
			printf("[%s--%d] Ͷ�ݽ�������ʧ��\n", __FUNCTION__, __LINE__);
			continue;
		}
	}
}

//��д�߳�
void CIOCP::ReadWriteWork(LPVOID pParam)
{
	//��������
	CIOCP * pFlashAccredit = (CIOCP *)pParam;

	//�߳�ѭ��
	while(true)
	{
		//�ȴ���ɶ˿���SOCKET�����
		bool bRet = false;
		DWORD dwThancferred=0;
		DWORD  dwCompletionKey=0;
		LPPER_IO_OPERATION_DATA pIOOperdata=NULL;
		GetQueuedCompletionStatus(pFlashAccredit->m_lIOCPHandle,&dwThancferred,&dwCompletionKey,(LPOVERLAPPED *)&pIOOperdata,INFINITE);
		if(pIOOperdata==NULL) continue;

		//����Ƿ��д������
		if(dwThancferred == 0 && (pIOOperdata->cbOperationType == RECV_POSTED || pIOOperdata->cbOperationType == SEND_POSTED))
		{
			//�ر�SOCKET
			pFlashAccredit->CloseSocket(pIOOperdata->socket);
			continue;
		}

		//Ϊ�������
		if(pIOOperdata->cbOperationType == RECV_POSTED)
		{
			//���ݰ�����
			
			pFlashAccredit->m_pTcpNetWorkEngine->ParseTCPPacket(pIOOperdata->socket, pIOOperdata->cbDataBuff);

			//
			bRet = pFlashAccredit->PostRecv(pIOOperdata->socket);
			if (!bRet)
			{
				printf("[%s--%d] Ͷ�ݽ�������ʧ��\n", __FUNCTION__, __LINE__);
				continue;
			}
		}
		else if(pIOOperdata->cbOperationType == SEND_POSTED)
		{
			bRet = pFlashAccredit->PostRecv(pIOOperdata->socket);
			if (!bRet)
			{
				printf("[%s--%d] Ͷ�ݽ�������ʧ��\n", __FUNCTION__, __LINE__);
				continue;
			}
		}
	}
}

//��������
bool CIOCP::PostSend(SOCKET Socket, CHAR szBuf[])
{

	LPPER_IO_OPERATION_DATA pIOOperdata = GetIOOperationItem(Socket, SEND_POSTED);

	//Ͷ�ݷ����������
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	CopyMemory(pIOOperdata->WSABuff.buf, szBuf, sizeof(CHAR)*strlen(szBuf));

	INT nRet = WSASend(pIOOperdata->socket, &(pIOOperdata->WSABuff), 1, &dwRecvBytes, dwFlags, &(pIOOperdata->Overlapped), NULL);
	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		CloseSocket(pIOOperdata->socket);
		return false;
	}
	return true;
}

//��������
bool CIOCP::PostRecv(SOCKET Socket)
{
	LPPER_IO_OPERATION_DATA pIOOperdata = GetIOOperationItem(Socket, RECV_POSTED);

	//Ͷ�ݷ����������
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	INT nRet = WSARecv(pIOOperdata->socket, &(pIOOperdata->WSABuff), 1, &dwRecvBytes, &dwFlags, &(pIOOperdata->Overlapped), NULL);
	if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		CloseSocket(pIOOperdata->socket);

		return false;
	}

	return true;
}
