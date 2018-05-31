
#ifndef _LOGGER_H_  
#define _LOGGER_H_  

#pragma once

#include <Windows.h>  
#include <stdio.h>  
#include <string> 

namespace LOGGER
{
	//�ַ�ת��
	//C����ַ���
	int UnicodeToAnsi(LPSTR szAnsi, LPCWSTR wstrUnicode);
	int AnsiToUnicode(LPWSTR wstrUnicode, LPCSTR szAnsi);
	int UTF8ToUnicode(LPWSTR wstrUnicoe, LPCSTR szUTF8);
	int UnicodeToUTF8(LPSTR szUTF8, LPCWSTR wszUnicode);
	int UTF8ToAnsi(LPSTR szAnsi, LPCSTR szUTF8);
	int AnsiToUTF8(LPSTR szUTF8, LPCSTR szAnsi);

	//c++����ַ���
	std::string		UnicodeToAnsi(const std::wstring wstrUnicode);
	std::wstring	AnsiToUnicode(const std::string szAnsi);
	std::wstring	UTF8ToUnicode(const std::string szUTF8);
	std::string		UnicodeToUTF8(const std::wstring wszUnicode);
	std::string		UTF8ToAnsi(const std::string szUTF8);
	std::string		AnsiToUTF8(const std::string szAnsi);

	//��־�������ʾ��Ϣ  
	static const std::string strFatalPrefix = "Fatal\t";
	static const std::string strErrorPrefix = "Error\t";
	static const std::string strWarningPrefix = "Warning\t";
	static const std::string strInfoPrefix = "Info\t";

	//��־����ö��  
	typedef enum EnumLogLevel
	{
		LogLevel_Stop = 0,  //ʲô������¼  
		LogLevel_Fatal,     //ֻ��¼���ش���  
		LogLevel_Error,     //��¼���ش�����ͨ����  
		LogLevel_Warning,   //��¼���ش�����ͨ���󣬾���  
		LogLevel_Info       //��¼���ش�����ͨ���󣬾��棬��ʾ��Ϣ(Ҳ����ȫ����¼)  
	};

	class CLogger
	{
	public:
		//nLogLevel����־��¼�ĵȼ����ɿ�  
		//strLogPath����־Ŀ¼���ɿ�  
		//strLogName����־���ƣ��ɿ�  
		CLogger(const std::string strLogName = "", const std::string strLogPath = "", EnumLogLevel nLogLevel = LogLevel_Info);
		//��������  
		virtual ~CLogger();
	public:
		//д���ش�����Ϣ  
		void TraceFatal(const char *lpcszFormat, ...);
		void TraceFatal(const wchar_t *lpcszFormat, ...);
		//д������Ϣ  
		void TraceError(const char *lpcszFormat, ...);
		void TraceError(const wchar_t *lpcszFormat, ...);
		//д������Ϣ  
		void TraceWarning(const char *lpcszFormat, ...);
		void TraceWarning(const wchar_t *lpcszFormat, ...);
		//д��ʾ��Ϣ  
		void TraceInfo(const char *lpcszFormat, ...);
		void TraceInfo(const wchar_t *lpcszFormat, ...);
		

		//�ı�д��־����  
		void ChangeLogLevel(EnumLogLevel nLevel);
		//��ȡ��������·��  
		static std::string GetAppPathA();
		//��ʽ���ַ���  
		static std::string FormatString(const char *lpcszFormat, ...);
		static std::wstring FormatString(const wchar_t *lpcszFormat, ...);
	private:
		//д�ļ�����  
		void Trace(const std::string &strLog);
		void Trace(const std::wstring& strLog);
		//��ȡ��ǰϵͳʱ��  
		std::string GetTime();
		//�ļ�ȫ·���õ��ļ���  
		const char *path_file(const char *path, char splitter);

	private:
		//д��־�ļ���  
		FILE * m_pFileStream;
		//д��־����  
		EnumLogLevel m_nLogLevel;
		//��־Ŀ¼  
		std::string m_strLogPath;
		//��־������  
		std::string m_strLogName;
		//��־�ļ�ȫ·��  
		std::string m_strLogFilePath;
		//���ʻ�
		std::string m_oldLocale;
		//�߳�ͬ�����ٽ�������  
		CRITICAL_SECTION m_cs;
	};
}

#endif  