#pragma once
#include "AnalysePE.h"
#include "Decode2Asm.h"
#include "Disasm.h"

//INT3断点
struct Int3BreakPoint 
{
	BOOL  DelFlag;
	BYTE  b_code;
	DWORD BPAddress;
};

//硬件断点
struct HWBreakPoint
{	
	BOOL DR0;        //当前寄存器使用状态
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

//内存断点处理标记
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
	BOOL bDelFlag;	      // 删除标记	
	BOOL bRWFlag;		  // READ   or  WRITE
	DWORD dwBeginAddr;   //断点开始地址
	DWORD dwEndAddr;     //断点终止地址
};

//添加用户断点表 <当前页>
struct MemPageBreakPoint
{	
	DWORD  BeginPageAddress;			//分页开始地址
	DWORD  EndPageAddress;              //设置内存断点当前分页的最后终止地址
	DWORD  OldPageProtect;				//原来页的保护属性	
	CList<MemBreakPointInfo,MemBreakPointInfo> *RWMemBPList;  //保存当前分页中所有读地址断点 <段+偏移 （保存偏移）>   
};

//保存当前程序运行中的一条代码  主要用于单步和步过
struct SaveCurrentCode 
{
	char szSrcCode;                     //保存原来的单个字节  CALL设置当前一个字节CC
	char szCurLineCode[4];              //保存当前机器码主要用来分析CALL指令 步过
	UINT nCodeSize;						//当前代码长度
	DWORD pStartAddress;                //程序入口地址
	DWORD pCurCodeEntryAddress;			//当前地址，执行完后保存下一条的地址首地址
	char szCurAsmBuf[64];				//当前程序指令
};

struct ExceptionFlag 
{
	BOOL bGRunFlag;						//G命令处理标志
	BOOL bSetInFlag;					//单步进入处理标志
	BOOL bSetJumpFlag;					//单步跳过处理标志
	BOOL bHardWareSetinFlag;            //硬件断点引起的单步重设标记
	BOOL bHardWareFlag;                 //硬件标记
	BOOL bSetInt3Flag;                  //int3断点重置标记
	ExceptionFlag()
	{
		bGRunFlag = FALSE;
		bSetInFlag = FALSE;
		bSetJumpFlag= FALSE;
	}
};

const BYTE code = 0xCC;
#define PRINTFREGVAULE(def)   if (0 != def)				\
								printf("断点可用\r\n");	\
							else					\
								printf("断点无效\r\n");  \

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
//DR7与宏定义得出一个值 来判断DR0 DR1 DR2 DR3执行 读或写  写断点
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
    //显示帮助信息
	void ShowHelp();
	//获取命令
	void InputCMD(char* szCurCmd = NULL);	
	//显示寄存器信息(R命令)
	void ShowRegInfo();

    //显示内存断点
	void ShowMemBreakPoint();
	//显示指定内存数据
	void ShowMemData(DWORD ShowAddressData);
    unsigned char AscToHex(unsigned char aChar);
    char ConvertCode(char& code1, char code2 = 0);
    unsigned char HexToAsc(unsigned char aHex);
    void EditMem(DWORD dwAddr);
	//显示当前代码
	void ShowCurCode(DWORD CurCodeAddr);
	//显示反汇编代码
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


//硬件断点
	//硬件断点设置
    void SetHWBreakPoint(DWORD SetAddress, int DR, int PointType, int nLen = 1);
	//硬件DR0
	void SetDR0HWBreakPoint(int PointType, int nLen);
	//硬件DR1
	void SetDR1HWBreakPoint(int PointType, int nLen);
	//硬件DR2
	void SetDR2HWBreakPoint(int PointType, int nLen);
	//硬件DR3
	void SetDR3HWBreakPoint(int PointType, int nLen);
	//硬件写
	void SetHWWriteBreakPoint(DWORD WriteAddress, int nLen);
	//硬件访问
	void SetHWAccessBreakPoint(DWORD ReadAddress,int nLen);
	//硬件执行
	void SetHWExecuteBreakPoint(DWORD ExecuteAddress);
    //硬件断点处理
	void HWBreakPointProc();
	//删除硬件断点
	void DelHWBreakPointFlag(int DRindex);


//int3断点
	//设置一次性INT3断点
	void SetOneTimeBreakPoint(DWORD BreakAddress);
	//设置INT3断点
	BYTE SetInt3BreakPoint(HANDLE hProcess,LPVOID pAdd, BYTE code, BOOL bFlag = TRUE);
	//查找该地址是否设置过INT3断点
	BOOL FindInt3BreakPoint(DWORD BreakAddress, char *Code = NULL);
    //重设INT3断点
	BOOL SetInt3BPointAgain(DWORD BreakAddress);
	//删除INT3断点
	BOOL DelInt3BreakPoint(DWORD BreakAddress);

//内存断点
	//设置内存读写断点
	void SetMemReadWriteBPoint(DWORD StartAddress,DWORD EndAddress);
	//查找指定的当内存页
	BOOL FindMemPage(DWORD PageAddress, DWORD EndAddress);
	//删除指定内存断点数据               <仅仅用于显示内存链表信息个数>
	void DelMemBreakPoint(DWORD StartAddress, DWORD EndAddress, int MemFlag);
	//删除已下的内存断点
	BOOL DelMemBPoint(DWORD StartAddress, DWORD EndAddress, int MemFlag);


	//调试循环
	void DebugLoop();
	//创建调试进程
	BOOL CreateDebugProcess(char*);
	//获取线程句柄OpenThread
	void GetThreadInformation(DEBUG_EVENT &Debugevent);
	//单步异常处理
	void StepExceptionProc(EXCEPTION_RECORD RecordTmp);
	//断点异常处理
	void BreakPointExceptionProc(EXCEPTION_RECORD RecordTmp);
	//访问异常处理
	void AccessExceptionProc(EXCEPTION_RECORD RecordTmp);

	void ImportScript();
	void ExportScript();
	//解析DLL
	void AnalyseLoadDLL(LPDEBUG_EVENT DebugEv);
    //根据地址查API
	BOOL FindFuncNameByAddr(DWORD addr);
	//显示函数名
	void ShowFuncName(char *szBuf);

private:
	//ExportTableInfor m_EptInfor;
	MemPageBreakPoint m_BPMemData;
	MemPageBreakPoint m_BPMemDataTmp; 
	AnalysePE m_AnalysePE;
	CREATE_PROCESS_DEBUG_INFO m_ProcessInfo;
	unsigned int m_StartMemAddr;          //一个分页中保存内存断点触发的异常地址(读or写)

//判断成员变量 TRUE or FLASE
private:
    BOOL m_RWMemFlag;
	BOOL m_bExNOProc;                     // 异常给操作系统
	BOOL m_bSysProc;
	BOOL m_MemStepFlag;                   //内存断点标志
	BOOL m_bFirstLoadDll;
	BOOL m_HWHMFlag;                      //内存断点与硬件断点组合下断点时存在对应关系
    MemBreakPoint m_bRWBPointFlag;        //内存处理读写标志
	ExceptionFlag m_EpnFlag;
	HWBreakPoint  m_HWflag;               //硬件断点标记位


private:
	DWORD m_CurCodeAddress;               //U指令记录地址	
	DWORD m_CurCodeMemAddress;			  //D指令记录地址
	DWORD m_HWBPointValue;                //保存当前硬件断点的DR7
	DWORD m_HInt3BPoint;                  //保存int3和硬件断点重合的地址

	HANDLE m_hThread;
	HANDLE m_hProcess;

	BYTE m_oepCode;                       //入口地址的临时保存 不加入INT3链表
	BYTE m_GRunCode;                      //g 命令的单独处理 不加入INT3链表

	SaveCurrentCode m_CurCode;                //保存当前代码等信息
	CONTEXT m_Context;					  //寄存器记录
    Int3BreakPoint m_BpNode;                  //保存当前int3地址信息 重置后继续设置CC
	BOOL bSelectEptFlag;                  //判断手动导
	BOOL bSelIptFlag;                     //触发脚本导入命令标记
    BOOL bTarce = FALSE;
	
//链表
private:
	list<Int3BreakPoint> m_Int3BreakPointList;
	CList<MemPageBreakPoint,MemPageBreakPoint> m_MemBPList;
	//保存内存链表用于显示   <仅仅用于显示内存链表信息个数>
	CList<MemBreakPointInfo, MemBreakPointInfo> m_ShowMemList;
	//脚本保存命令
	list<char*> m_MyCmdList;
    char* m_pFileName;
    DWORD m_dwPID;
};
