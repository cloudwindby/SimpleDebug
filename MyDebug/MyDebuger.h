#pragma once
#include "AnalysePE.h"
#include "Decode2Asm.h"
#include "Disasm.h"

//INT3�ϵ�
struct Int3BreakPoint 
{
	BOOL  DelFlag;
	BYTE  b_code;
	DWORD BPAddress;
};

//Ӳ���ϵ�
struct HWBreakPoint
{	
	BOOL DR0;        //��ǰ�Ĵ���ʹ��״̬
	BOOL DR1;
	BOOL DR2;
	BOOL DR3;	
	HWBreakPoint()
	{
		DR0 = FALSE;
		DR1 = FALSE;
		DR2 = FALSE;
		DR3 = FALSE;	
	}
};

//�ڴ�ϵ㴦����
struct MemBreakPoint
{
	BOOL bMemRead;
	BOOL bMemWrite;
	MemBreakPoint()
	{
		bMemRead = FALSE;
		bMemWrite = FALSE;
	}

};

struct tagChar
{
    unsigned char m_a : 4;
    unsigned char m_b : 4;
};

#define READ  0
#define WRITE 1

struct MemBreakPointInfo 
{
	BOOL bDelFlag;	      // ɾ�����	
	BOOL bRWFlag;		  // READ   or  WRITE
	DWORD dwBeginAddr;   //�ϵ㿪ʼ��ַ
	DWORD dwEndAddr;     //�ϵ���ֹ��ַ
};

//����û��ϵ�� <��ǰҳ>
struct MemPageBreakPoint
{	
	DWORD  BeginPageAddress;			//��ҳ��ʼ��ַ
	DWORD  EndPageAddress;              //�����ڴ�ϵ㵱ǰ��ҳ�������ֹ��ַ
	DWORD  OldPageProtect;				//ԭ��ҳ�ı�������	
	CList<MemBreakPointInfo,MemBreakPointInfo> *RWMemBPList;  //���浱ǰ��ҳ�����ж���ַ�ϵ� <��+ƫ�� ������ƫ�ƣ�>   
};

//���浱ǰ���������е�һ������  ��Ҫ���ڵ����Ͳ���
struct SaveCurrentCode 
{
	char szSrcCode;                     //����ԭ���ĵ����ֽ�  CALL���õ�ǰһ���ֽ�CC
	char szCurLineCode[4];              //���浱ǰ��������Ҫ��������CALLָ�� ����
	UINT nCodeSize;						//��ǰ���볤��
	DWORD pStartAddress;                //������ڵ�ַ
	DWORD pCurCodeEntryAddress;			//��ǰ��ַ��ִ����󱣴���һ���ĵ�ַ�׵�ַ
	char szCurAsmBuf[64];				//��ǰ����ָ��
};

struct ExceptionFlag 
{
	BOOL bGRunFlag;						//G������־
	BOOL bSetInFlag;					//�������봦���־
	BOOL bSetJumpFlag;					//�������������־
	BOOL bHardWareSetinFlag;            //Ӳ���ϵ�����ĵ���������
	BOOL bHardWareFlag;                 //Ӳ�����
	BOOL bSetInt3Flag;                  //int3�ϵ����ñ��
	ExceptionFlag()
	{
		bGRunFlag = FALSE;
		bSetInFlag = FALSE;
		bSetJumpFlag= FALSE;
	}
};

const BYTE code = 0xCC;
#define PRINTFREGVAULE(def)   if (0 != def)				\
								printf("�ϵ����\r\n");	\
							else					\
								printf("�ϵ���Ч\r\n");  \

// OF IF TF SF ZF AF PF CF
// NV UP EI PL NZ NA PO NC
#define  EFLAG_OF 0x800
#define  EFLAG_IF 0x200
#define  EFLAG_TF 0x100
#define  EFLAG_SF 0x80
#define  EFLAG_ZF 0x40
#define  EFLAG_AF 0x10
#define  EFLAG_PF 0x4
#define  EFLAG_CF 0x1

///////////////////////////////////////////////////////////////////////
//DR7��궨��ó�һ��ֵ ���ж�DR0 DR1 DR2 DR3ִ�� ����д  д�ϵ�
//DR0
#define ONEDR0EVALUE 	0X301
#define ONEDR0WVALUE	0x10701
#define ONEDR0RWVALUE	0x30701

#define TWODR0EVALUE 	0X301
#define TWODR0WVALUE	0x50701
#define TWODR0RWVALUE	0x70701

#define FOURDR0EVALUE 	0X301
#define FOURDR0WVALUE	0x0D0701
#define FOURDR0RWVALUE	0x0F0701

//DR1
#define ONEDR1EVALUE 	0x304
#define ONEDR1WVALUE	0x100704
#define ONEDR1RWVALUE	0x300704

#define TWODR1EVALUE 	0x304
#define TWODR1WVALUE	0x500704
#define TWODR1RWVALUE	0x700704

#define FOURDR1EVALUE 	0x304
#define FOURDR1WVALUE	0x0D00704
#define FOURDR1RWVALUE	0x0F00704


//DR2
#define ONEDR2EVALUE 	0x310
#define ONEDR2WVALUE	0x1000710
#define ONEDR2RWVALUE	0x3000710

#define TWODR2EVALUE 	0x301
#define TWODR2WVALUE	0x5000710
#define TWODR2RWVALUE	0x7000710

#define FOURDR2EVALUE 	0x301
#define FOURDR2WVALUE	0x0D000710
#define FOURDR2RWVALUE	0x0F000710

//DR3
#define ONEDR3EVALUE 	0x340
#define ONEDR3WVALUE	0x10000740
#define ONEDR3RWVALUE	0x30000740

#define TWODR3EVALUE 	0x340
#define TWODR3WVALUE	0x50000740
#define TWODR3RWVALUE	0x70000740

#define FOURDR3EVALUE 	0x340
#define FOURDR3WVALUE	0x0D0000740
#define FOURDR3RWVALUE	0x0F0000740
///////////////////////////////////////////////////////////////////////



typedef HANDLE(__stdcall *pMyOpenThread)(DWORD dwDesiredAccess,  // access right
    BOOL bInheritHandle,    // handle inheritance option
    DWORD dwThreadId        // thread identifier
    );


class MyDebuger  
{
public:
	MyDebuger();
    MyDebuger(char*);
	virtual ~MyDebuger();

    inline BOOL StrConvertDec(char *pSrc, unsigned int &nDec)
    {
        int nLen = strlen(pSrc);
        unsigned int nSum = 0;

        if (nLen > 8)
        {
            nDec = NULL;
            return FALSE;
        }

        for (int i = nLen - 1, j = 0; i >= 0; i--, j++)
        {
            if (pSrc[i] >= '0'&&pSrc[i] <= '9')
            {
                nSum += (pSrc[i] - '0')*(unsigned int)pow(16, j);
                continue;
            }

            if (pSrc[i] >= 'a'&&pSrc[i] <= 'f')
            {
                nSum += (pSrc[i] - 'a' + 10)*(unsigned int)pow(16, j);
                continue;
            }

            if (pSrc[i] >= 'A'&&pSrc[i] <= 'F')
            {
                nSum += (pSrc[i] - 'A' + 10)*(unsigned int)pow(16, j);
                continue;
            }

            nDec = NULL;
            return FALSE;
        }

        nDec = nSum;
        return TRUE;

    }

public:
    //��ʾ������Ϣ
	void ShowHelp();
	//��ȡ����
	void InputCMD(char* szCurCmd = NULL);	
	//��ʾ�Ĵ�����Ϣ(R����)
	void ShowRegInfo();

    //��ʾ�ڴ�ϵ�
	void ShowMemBreakPoint();
	//��ʾָ���ڴ�����
	void ShowMemData(DWORD ShowAddressData);
    unsigned char AscToHex(unsigned char aChar);
    char ConvertCode(char& code1, char code2 = 0);
    unsigned char HexToAsc(unsigned char aHex);
    void EditMem(DWORD dwAddr);
	//��ʾ��ǰ����
	void ShowCurCode(DWORD CurCodeAddr);
	//��ʾ��������
	void ShowAsmCode(char* szPcode,DWORD CurAddress);

	//G
	void RunInt3BreakPoint(DWORD RunAddressData);
	//P
	void StepOverProc();
    void StepOver();
	//T
	void StepIntoProc();
	void StepInto();
    //trace
    void TraceProc(DWORD dwAddr);


//Ӳ���ϵ�
	//Ӳ���ϵ�����
    void SetHWBreakPoint(DWORD SetAddress, int DR, int PointType, int nLen = 1);
	//Ӳ��DR0
	void SetDR0HWBreakPoint(int PointType, int nLen);
	//Ӳ��DR1
	void SetDR1HWBreakPoint(int PointType, int nLen);
	//Ӳ��DR2
	void SetDR2HWBreakPoint(int PointType, int nLen);
	//Ӳ��DR3
	void SetDR3HWBreakPoint(int PointType, int nLen);
	//Ӳ��д
	void SetHWWriteBreakPoint(DWORD WriteAddress, int nLen);
	//Ӳ������
	void SetHWAccessBreakPoint(DWORD ReadAddress,int nLen);
	//Ӳ��ִ��
	void SetHWExecuteBreakPoint(DWORD ExecuteAddress);
    //Ӳ���ϵ㴦��
	void HWBreakPointProc();
	//ɾ��Ӳ���ϵ�
	void DelHWBreakPointFlag(int DRindex);


//int3�ϵ�
	//����һ����INT3�ϵ�
	void SetOneTimeBreakPoint(DWORD BreakAddress);
	//����INT3�ϵ�
	BYTE SetInt3BreakPoint(HANDLE hProcess,LPVOID pAdd, BYTE code, BOOL bFlag = TRUE);
	//���Ҹõ�ַ�Ƿ����ù�INT3�ϵ�
	BOOL FindInt3BreakPoint(DWORD BreakAddress, char *Code = NULL);
    //����INT3�ϵ�
	BOOL SetInt3BPointAgain(DWORD BreakAddress);
	//ɾ��INT3�ϵ�
	BOOL DelInt3BreakPoint(DWORD BreakAddress);

//�ڴ�ϵ�
	//�����ڴ��д�ϵ�
	void SetMemReadWriteBPoint(DWORD StartAddress,DWORD EndAddress);
	//����ָ���ĵ��ڴ�ҳ
	BOOL FindMemPage(DWORD PageAddress, DWORD EndAddress);
	//ɾ��ָ���ڴ�ϵ�����               <����������ʾ�ڴ�������Ϣ����>
	void DelMemBreakPoint(DWORD StartAddress, DWORD EndAddress, int MemFlag);
	//ɾ�����µ��ڴ�ϵ�
	BOOL DelMemBPoint(DWORD StartAddress, DWORD EndAddress, int MemFlag);


	//����ѭ��
	void DebugLoop();
	//�������Խ���
	BOOL CreateDebugProcess(char*);
	//��ȡ�߳̾��OpenThread
	void GetThreadInformation(DEBUG_EVENT &Debugevent);
	//�����쳣����
	void StepExceptionProc(EXCEPTION_RECORD RecordTmp);
	//�ϵ��쳣����
	void BreakPointExceptionProc(EXCEPTION_RECORD RecordTmp);
	//�����쳣����
	void AccessExceptionProc(EXCEPTION_RECORD RecordTmp);

	void ImportScript();
	void ExportScript();
	//����DLL
	void AnalyseLoadDLL(LPDEBUG_EVENT DebugEv);
    //���ݵ�ַ��API
	BOOL FindFuncNameByAddr(DWORD addr);
	//��ʾ������
	void ShowFuncName(char *szBuf);

private:
	//ExportTableInfor m_EptInfor;
	MemPageBreakPoint m_BPMemData;
	MemPageBreakPoint m_BPMemDataTmp; 
	AnalysePE m_AnalysePE;
	CREATE_PROCESS_DEBUG_INFO m_ProcessInfo;
	unsigned int m_StartMemAddr;          //һ����ҳ�б����ڴ�ϵ㴥�����쳣��ַ(��orд)

//�жϳ�Ա���� TRUE or FLASE
private:
    BOOL m_RWMemFlag;
	BOOL m_bExNOProc;                     // �쳣������ϵͳ
	BOOL m_bSysProc;
	BOOL m_MemStepFlag;                   //�ڴ�ϵ��־
	BOOL m_bFirstLoadDll;
	BOOL m_HWHMFlag;                      //�ڴ�ϵ���Ӳ���ϵ�����¶ϵ�ʱ���ڶ�Ӧ��ϵ
    MemBreakPoint m_bRWBPointFlag;        //�ڴ洦���д��־
	ExceptionFlag m_EpnFlag;
	HWBreakPoint  m_HWflag;               //Ӳ���ϵ���λ


private:
	DWORD m_CurCodeAddress;               //Uָ���¼��ַ	
	DWORD m_CurCodeMemAddress;			  //Dָ���¼��ַ
	DWORD m_HWBPointValue;                //���浱ǰӲ���ϵ��DR7
	DWORD m_HInt3BPoint;                  //����int3��Ӳ���ϵ��غϵĵ�ַ

	HANDLE m_hThread;
	HANDLE m_hProcess;

	BYTE m_oepCode;                       //��ڵ�ַ����ʱ���� ������INT3����
	BYTE m_GRunCode;                      //g ����ĵ������� ������INT3����

	SaveCurrentCode m_CurCode;                //���浱ǰ�������Ϣ
	CONTEXT m_Context;					  //�Ĵ�����¼
    Int3BreakPoint m_BpNode;                  //���浱ǰint3��ַ��Ϣ ���ú��������CC
	BOOL bSelectEptFlag;                  //�ж��ֶ���
	BOOL bSelIptFlag;                     //�����ű�����������
    BOOL bTarce = FALSE;
	
//����
private:
	list<Int3BreakPoint> m_Int3BreakPointList;
	CList<MemPageBreakPoint,MemPageBreakPoint> m_MemBPList;
	//�����ڴ�����������ʾ   <����������ʾ�ڴ�������Ϣ����>
	CList<MemBreakPointInfo, MemBreakPointInfo> m_ShowMemList;
	//�ű���������
	list<char*> m_MyCmdList;
    char* m_pFileName;
    DWORD m_dwPID;
};
