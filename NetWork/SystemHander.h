#pragma once
#include <map>
#include <string>
#include "ScopeLock.h"
#include "HttpHander.h"
using namespace std;

struct VtnCodeData
{
	wstring mobilePhone;	//�ֻ���
	wstring vtnCode;		//�ֻ���֤��
	DWORD expireTime;		//����ʱ�䣬��Ϊ��λ
};

class CSystemHander
{
public:
	static CSystemHander& GetInstance()											//Ψһʵ��
	{
		static CSystemHander s_instance;
		return s_instance;
	}

public:
	CSystemHander(void);
	~CSystemHander(void);

public:
	void InitData(CString szFilePath);											//ʹ��ǰ���ȵ��ó�ʼ��

	//�ֻ���֤�����
	bool SendVerificationCode(wstring mobilePhone);								//�����ֻ���֤��
	bool VerifyVerificationCode(wstring mobilePhone, wstring vtnCode);			//�ж���֤���Ƿ���Ч
	void RemoveVerificationCode(wstring mobilePhone);							//�����֤��


	//�������
	wstring CreateOrderId();													//��������ID
	std::string NoticePhpCreateOrder(wstring orderId);							//֪ͨPHP�����ɶ���

	//��ȡ�������б�
	bool GetBusinessesList(wstring &content);									//��ȡ�������б�

private:
	int GetRandNumber(int maxNumber = 10000, int minNumber = 1000);
	bool SendSMS(wstring mobilePhone, wstring vtnCode);							// ���ܰ����ƶ���
	string CreateSMSUrl(wstring mobilePhone, wstring vtnCode);					//�������Ͷ��ŵ�url	
	string specialUrlEncode(string value);
	string StringSign(string accessSecret, string stringToSign) ;
	std::string  GetErrorMsg(std::string strSrc);								//���������������ȡ������Ϣ

private:
	map<wstring, VtnCodeData> m_VtnCodeMap;										//������֤�롢�������ݿ�

	static MyMutex s_instance_mutex;

private:
	int m_VerificationCodeValidity;												//��λΪ��
	string m_AliyunSmsSignName;
	string m_AliyunSmsTemplateCode;
	string m_AliyunSmsTemplateParam;
	string m_AliyunSmsAccessKeyId;
	string m_AliyunSmsAccessSecret;

	string m_OrderNoticeUrl;													//�ϱ�����URL
	string m_BusinessesUrl;														//��ȡ�������б�URL

};
