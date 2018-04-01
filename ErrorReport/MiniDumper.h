#pragma  once   
  
#include <windows.h>
#include <dbghelp.h> 
#include <vector>
  

const int MAX_ADDRESS_LENGTH = 32;
const int MAX_NAME_LENGTH = 1024;

// ������Ϣ  
//   
struct CrashInfo
{
	CHAR ErrorCode[MAX_ADDRESS_LENGTH];
	CHAR Address[MAX_ADDRESS_LENGTH];
	CHAR Flags[MAX_ADDRESS_LENGTH];
};

// CallStack��Ϣ  
//  
struct CallStackInfo
{
	CHAR ModuleName[MAX_NAME_LENGTH];
	CHAR MethodName[MAX_NAME_LENGTH];
	CHAR FileName[MAX_NAME_LENGTH];
	CHAR LineNumber[MAX_NAME_LENGTH];
};



class CMiniDumper  
{  
public:  
    static HRESULT  CreateInstance();  
    static HRESULT  ReleaseInstance();  
  
public:  
    LONG WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo);  
  
private:  
	//����dump�ļ�����
    void SetMiniDumpFileName(void);  
    BOOL GetImpersonationToken(HANDLE* phToken); 
	//
    BOOL EnablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld); 

    BOOL RestorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld); 
	//��ֹ�����쳣������
	void DisableSetUnhandledExceptionFilter();
	//��ȫ�����ַ���
	void SafeStrCpy(char* szDest, size_t nMaxDestSize, const char* szSrc);
	// �õ����������Ϣ   
	CrashInfo GetCrashInfo(const EXCEPTION_RECORD *pRecord);
	// �õ�CallStack��Ϣ  
	std::vector<CallStackInfo> GetCallStack(const CONTEXT *pContext);
	//д��������ջ��Ϣ
	LONG  CMiniDumper::WriteApplicationCrashInfo(EXCEPTION_POINTERS *pException);
	

private:
	//�쳣�ص�����
	static LONG WINAPI UnhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo);
	//дdump�ص�
	static BOOL WINAPI MiniDumpCallback(PVOID  pParam, const PMINIDUMP_CALLBACK_INPUT   pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput);
	//�ж�ģ���Ƿ����
	static BOOL IsDataSectionNeeded(const WCHAR* pModuleName);

private:  
    CMiniDumper();  
    virtual ~CMiniDumper(void);  
  
private:  
    TCHAR   m_szMiniDumpPath[MAX_PATH]; 
	//TCHAR	m_szCrashInfoPath[MAX_PATH];
    TCHAR   m_szAppPath[MAX_PATH]; 

	static CMiniDumper*			gs_pMiniDumper;
	static LPCRITICAL_SECTION	gs_pCriticalSection;
};  