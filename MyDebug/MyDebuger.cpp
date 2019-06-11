#include "stdafx.h"
#include "MyDebuger.h"
#include <string.h>
#include <math.h>

MyDebuger::MyDebuger(char* FileName)
{	
    m_pFileName = FileName;

	m_HInt3BPoint = 0;
	m_HWBPointValue = 0;
	m_CurCodeAddress = 0;	
	m_bSysProc = FALSE;
	m_HWHMFlag = FALSE;
	m_bExNOProc = FALSE;
	bSelIptFlag = FALSE;
	m_MemStepFlag = FALSE;	
	bSelectEptFlag = FALSE;
	m_bFirstLoadDll = TRUE;	
	m_EpnFlag.bGRunFlag = FALSE;
	m_EpnFlag.bSetInt3Flag = FALSE;
	m_EpnFlag.bHardWareFlag = FALSE;
	m_EpnFlag.bHardWareSetinFlag = FALSE;	
	m_GRunCode = 0;
	memset(&m_Context,0,sizeof(m_Context));
	memset(&m_HWflag,0,sizeof(HWBreakPoint));
	memset(m_CurCode.szCurLineCode,0,sizeof(m_CurCode.szCurLineCode));

	m_Context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
}

MyDebuger::~MyDebuger()
{
	//m_MemBPList 释放NEW出的空间
	if ( m_BPMemDataTmp.RWMemBPList != NULL)
	{
		delete m_BPMemDataTmp.RWMemBPList;
		m_BPMemDataTmp.RWMemBPList = NULL;
	}			
	//释放
	if ( m_BPMemData.RWMemBPList != NULL )
	{
		delete m_BPMemData.RWMemBPList;
		m_BPMemData.RWMemBPList = NULL;
	}

}

BOOL MyDebuger::CreateDebugProcess(char* FileName)
{
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };
    
    BOOL bRet = CreateProcessA(
        FileName,
        NULL,
        NULL,
        NULL,
        FALSE,
        DEBUG_ONLY_THIS_PROCESS,
        NULL,
        NULL,
        &si,
        &pi);

    if (!bRet)
    {
        printf("创建进程失败. \r\n");
        CloseHandle(pi.hProcess);
        return FALSE;
    }

    m_dwPID = pi.dwProcessId;

	//调试过程
	DebugLoop();
	
	return TRUE;
}


//        设置断点: m_code = SetBreakPoint( pFunAdd, 0xCC); 
//		  恢复断点: SetBreakPoint( pFunAdd, m_code);
BYTE MyDebuger::SetInt3BreakPoint(HANDLE hProcess,LPVOID pAdd, BYTE code,BOOL bFlag)
{
	BYTE b_read;
	BOOL bret;
	DWORD dwRead, dwOldFlg;
	// 0x80000000以上的地址为系统共有区域，不能够修改
	if( pAdd >= (LPVOID)0x80000000 || pAdd == (LPVOID)0)
		return code;
	// 取得需修改地址的一个字节代码
	bret = ReadProcessMemory(hProcess, pAdd, &b_read, sizeof(BYTE), &dwRead);
	// 如果原来的代码和准备修改的代码相同，没有必要再修改
	if(bret == 0 || b_read == code)
		return code;

	// 修改页码属性为可读可写并保存原属性
	VirtualProtectEx(hProcess, pAdd, sizeof(BYTE), PAGE_READWRITE, &dwOldFlg);
	// 修改目标代码为CC
	WriteProcessMemory(hProcess, pAdd, (LPVOID)&code, sizeof(BYTE), &dwRead);
	// 恢复原页码属性
	VirtualProtectEx(hProcess, pAdd, sizeof(BYTE), dwOldFlg, &dwOldFlg);
	return b_read;
}

//调试事件循环
void MyDebuger::DebugLoop()
{
	DEBUG_EVENT DebugEv;                   // debugging event information 
	DWORD dwContinueStatus = DBG_CONTINUE; // exception continuation 
	
	while (TRUE)
	{ 
		// Wait for a debugging event to occur. The second parameter indicates 
		// that the function does not return until a debugging event occurs. 

		WaitForDebugEvent(&DebugEv, INFINITE); 

		GetThreadInformation(DebugEv);
		// Process the debugging event code. 
		switch (DebugEv.dwDebugEventCode) 
		{ 
			case EXCEPTION_DEBUG_EVENT: 
				//处理异常前取得环境
				GetThreadContext(m_hThread,&m_Context);
				EXCEPTION_RECORD DebugRecordInfor = DebugEv.u.Exception.ExceptionRecord;
				switch (DebugRecordInfor.ExceptionCode) 
				{ 
					case EXCEPTION_ACCESS_VIOLATION: 
						// First chance: Pass this on to the system. 
						// Last chance: Display an appropriate error.
						AccessExceptionProc(DebugRecordInfor);
						if (m_bExNOProc == TRUE)
						{
							dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
							m_bExNOProc = FALSE;
						}
						break;
					case EXCEPTION_BREAKPOINT: 
						// First chance: Display the current 
						// instruction and register values.							
						m_bFirstLoadDll = FALSE;						
						BreakPointExceptionProc(DebugRecordInfor);
						break;		 
					case EXCEPTION_SINGLE_STEP: 
						// First chance: Update the display of the 
						// current instruction and register values.
						StepExceptionProc(DebugRecordInfor);
						break;				
				}
				SetThreadContext(m_hThread,&m_Context);
				break;
							
			case CREATE_THREAD_DEBUG_EVENT: 
				// As needed, examine or change the thread's registers 
				// with the GetThreadContext and SetThreadContext functions; 
				// and suspend and resume thread execution with the 
				// SuspendThread and ResumeThread functions. 				
				break;
				
			case CREATE_PROCESS_DEBUG_EVENT: 
				// As needed, examine or change the registers of the 
				// process's initial thread with the GetThreadContext and 
				// SetThreadContext functions; read from and write to the 
				// process's virtual memory with the ReadProcessMemory and 
				// WriteProcessMemory functions; and suspend and resume 
				// thread execution with the SuspendThread and ResumeThread 
				// functions. 
				{
					m_ProcessInfo = DebugEv.u.CreateProcessInfo;
					m_hProcess = m_ProcessInfo.hProcess;

					m_CurCode.pCurCodeEntryAddress =  (DWORD)m_ProcessInfo.lpStartAddress;
					m_CurCode.pStartAddress = (DWORD)m_ProcessInfo.lpStartAddress;
					//入口断点
					m_oepCode = SetInt3BreakPoint(m_ProcessInfo.hProcess, m_ProcessInfo.lpStartAddress, code);
					break;			
				}
				
			case EXIT_THREAD_DEBUG_EVENT: 
				// Display the thread's exit code. 
				break;
				
			case EXIT_PROCESS_DEBUG_EVENT: 
				// Display the process's exit code.			
				ExitProcess(NULL);
				break;
				
			case LOAD_DLL_DEBUG_EVENT: 
				// Read the debugging information included in the newly 
				// loaded DLL.
				if ( TRUE == m_bFirstLoadDll )
				{
					AnalyseLoadDLL(&DebugEv);
				}
				
				break;
				
			case UNLOAD_DLL_DEBUG_EVENT: 
				// Display a message that the DLL has been unloaded. 				
				break;
			case OUTPUT_DEBUG_STRING_EVENT: 
				// Display the output debugging string. 
				break;
		} 
		// Resume executing the thread that reported the debugging event.
		ContinueDebugEvent(DebugEv.dwProcessId, 
						   DebugEv.dwThreadId,
						   dwContinueStatus);
	}
}

void SetColor(int ForgC)
{
    WORD wColor;
    //We will need this handle to get the current background attribute
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    //We use csbi for the wAttributes word.
    if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
    {
        //Mask out all but the background attribute, and add in the forgournd color
        wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
        SetConsoleTextAttribute(hStdOut, wColor);
    }
}

//根据地址反查函数名
BOOL MyDebuger::FindFuncNameByAddr(DWORD addr)
{
    if (addr == 0)
    {
        return FALSE;
    }

    for (auto n : m_AnalysePE.m_ExportTableList)
    {
        if (n.szDllName != NULL)
        {
            for (auto i : n.ExportFuncList)
            {
                if (i.dwFuncAddress == addr)
                {
                    if (*i.szFuncName == NULL)
                        printf("\t\t\t\t%s.#:%d\r\n", n.szDllName, i.nExportOrdinal);
                    else
                        SetColor(14);
                        printf("\t\t\t\t%s.%s\r\n", n.szDllName, i.szFuncName);
                        SetColor(7);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

//用户输入的命令
void MyDebuger::InputCMD(char* szCurCmd)
{
	int i, ch;
	int j = 0, k = 0, n = 0;  //隔离输入指令空格的部分

	char strBuffer[24] = { 0 };           //输入的指令
	char strCmdCmp[12] = { 0 };			  //比较指令
	char strLenOrAddr[20] = { 0 };        //第2个空格后的数据 长度或地址
	
    char bufferAddr[18] = { 0 };
	char bufferEndAddr[18] = { 0 };
    char strCurCmd[24] = { 0 };

	char *pIntpuCMD[30] = {"t", "p", "g", "u", "d", "r", "e", 
                            "bp","bpl","bpc",//一般断点
						    "bhe","bhw","bha","bhl", "bhc",//硬件断点
						    "bmr","bmw", "bml", "bmpl", "bmc", //内存断点
						    "ls","es",//脚本
                            "q","ml","cls","trace","help",}; 

	unsigned int nBegAddress = 0; 
	unsigned int nEndAddress = 0;
	unsigned int nLenOrAddr = 0;
	
	printf("-");


	//判断是否从脚本中获取命令
	if ( TRUE == bSelIptFlag)
	{		
		for(i = 0; szCurCmd[i] != '\0' ; i++ )
		{
			if ( szCurCmd[i] == ' ' )
			{
				if ( j == 0 )
					j = i;				//求出输入的第二个空格指令开始位置
				else if ( k == 0 )
					k = i;				//求出输入的第二个空格指令开始位置
				else
				{
					printf("Input Error!\r\n");
					fflush(stdin);
					if ( FALSE == bSelIptFlag )
					{
						InputCMD();
					}
					return;
				}
			}
			strBuffer[i] = szCurCmd[i];
		}
		strBuffer[i] = '\0';
		printf("%s\r\n",szCurCmd);
	} 
	else
	{	
		for(i = 0; (i < 28) &&  ((ch = getchar()) != EOF) && (ch != '\n'); i++ )
		{
			if ( ch == ' ' )
			{
				if ( j == 0 )
					j = i;				//求出输入的第二个空格指令开始位置
				else if ( k == 0 )
					k = i;				//求出输入的第二个空格指令开始位置
				else
				{
					printf("Input Error!\r\n");
					fflush(stdin);
					if ( FALSE == bSelIptFlag )
					{
						InputCMD();
					}
					return;
				}
			}
			strBuffer[i] = (char)ch;
		}
		strBuffer[i] = '\0';
		
		memcpy((LPVOID)strCurCmd, strBuffer,sizeof(strBuffer));
	}
	
	if ( (j != 0) && ( k == 0 ) )
	{
		//分解第一个空格前的指令
		memcpy(strCmdCmp, strBuffer, j);
		//分解第一个空格后 ，第二个空格前的数据
		memcpy(bufferAddr,&strBuffer[j+1], i-j-1);
		//字符转换
		StrConvertDec(bufferAddr, nBegAddress);
	}
	else if (k != 0)
	{
		//分解第一个空格前的指令
		memcpy(strCmdCmp, strBuffer, j);
		//分解第一个空格后 ，第二个空格前的数据
		memcpy(bufferAddr,&strBuffer[j+1], k-j-1);		
		StrConvertDec(bufferAddr, nBegAddress);

		//分解第二个空格后的数据
		strncpy(strLenOrAddr, &strBuffer[k+1], i-k);
		StrConvertDec(strLenOrAddr, nLenOrAddr);
	}
	else
	{
		strcpy(strCmdCmp, strBuffer);
	}	
	
	
	for (i = 0; i < 28; i++)
	{
		if ( 0 == strcmp(strCmdCmp, pIntpuCMD[i]) )
		{
			n = i + 1;
			break;
		}
	}

	switch ( n )
	{
	//t
	case 1:
        StepInto();
		return;
	//p
	case 2:
        StepOver();
		return;
	//g
	case 3:
    {
        if (nBegAddress != 0)
        {
            m_MyCmdList.push_back(strCurCmd);
            RunInt3BreakPoint(nBegAddress);
            return;
        }
        else
        {
            return;
        }
    }
       
	//u
	case 4:
        m_MyCmdList.push_back(strCurCmd);
        ShowCurCode(nBegAddress);
		return;	
	//d
	case 5:
        m_MyCmdList.push_back(strCurCmd);
        ShowMemData(nBegAddress);
		return;
	//r
	case 6:
        m_MyCmdList.push_back(strCurCmd);
        ShowRegInfo();
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
	//e
	case 7:
        EditMem(nBegAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
		return;
	//bp
	case 8:
        if (nBegAddress == 0)
        {
            printf("\tInput Error!\r\n");
            if (FALSE == bSelIptFlag)
            {
                InputCMD();
            }
            return;
        }
        m_MyCmdList.push_back(strCurCmd);
        //如果硬件断点和int3断点重合,使int3断点变为无效
        if (nBegAddress == m_Context.Dr0 ||
            nBegAddress == m_Context.Dr1 ||
            nBegAddress == m_Context.Dr2 ||
            nBegAddress == m_Context.Dr3
            )
        {
            SetOneTimeBreakPoint(nBegAddress);
            DelInt3BreakPoint(nBegAddress);
        }
        else
        {
            SetOneTimeBreakPoint(nBegAddress);
        }
            

        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;

    //bpl 一般断点列表
	case 9:
    {
        m_MyCmdList.push_back(strCurCmd);
        printf("INT3断点列表:\r\n");
        printf("-----------------------------------------------------------------\r\n");
        printf("序号\t\t  地址\t\t  状态\r\n");

        int nBPNumber = 0;
        for (auto n : m_Int3BreakPointList)
        {
            if (n.BPAddress != NULL)
            {
                printf("%2d\t\t  0x%p\t\t  ", nBPNumber + 1, n.BPAddress);
                if (FALSE == n.DelFlag)
                {
                    printf("断点无效\r\n");
                }
                    
                else
                {
                    printf("断点可用\r\n");
                }
             nBPNumber++;
            }
        }
        printf("-----------------------------------------------------------------\r\n");

        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }
        
	//bpc 删除一般断点
	case 10:
    {
        m_MyCmdList.push_back(strCurCmd);
        if (TRUE == DelInt3BreakPoint(nBegAddress))
        {
            printf("\t\t删除成功!\r\n");
        }
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }
       
           
	//bhe 硬件执行断点
	case 11:
		m_MyCmdList.push_back(strCurCmd);
        DelInt3BreakPoint(nBegAddress);
        SetHWExecuteBreakPoint(nBegAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }

        return;
    //bhw 硬件写入断点
    case 12:
    {
        if ((strlen(bufferAddr) > 8) || !((nLenOrAddr == 1) || (nLenOrAddr == 2) || (nLenOrAddr == 4)))
        {
            printf("\tInput Error!\r\n");
            if (FALSE == bSelIptFlag)
            {
                InputCMD();
            }
            return;
        }
        m_MyCmdList.push_back(strCurCmd);
        DelInt3BreakPoint(nBegAddress);
        //硬件写入断点
        SetHWWriteBreakPoint(nBegAddress, nLenOrAddr);
        return;
    }
        
    //bha 硬件访问断点
    case 13:
    {
        if ((strlen(bufferAddr) > 8) || !((nLenOrAddr == 1) || (nLenOrAddr == 2) || (nLenOrAddr == 4)))
        {
            printf("\tInput Error!\r\n");
            if (FALSE == bSelIptFlag)
            {
                InputCMD();
            }
            return;
        }
        m_MyCmdList.push_back(strCurCmd);
        DelInt3BreakPoint(nBegAddress);
        //硬件访问
        SetHWAccessBreakPoint(nBegAddress, nLenOrAddr);
        return;
    }
        
	//bhl 硬件断点列表
	case 14:
    {
        m_MyCmdList.push_back(strCurCmd);
        printf("硬件断点列表:\r\n");
        printf("-----------------------------------------------------------------\r\n");
        printf(" 硬件断点寄存器\t\t      地址\t\t      状态\r\n");
        printf("    DR0\t\t\t  0x%p\t\t   ", m_Context.Dr0);
        PRINTFREGVAULE(m_Context.Dr0);
        printf("    DR1\t\t\t  0x%p\t\t   ", m_Context.Dr1);
        PRINTFREGVAULE(m_Context.Dr1);
        printf("    DR2\t\t\t  0x%p\t\t   ", m_Context.Dr2);
        PRINTFREGVAULE(m_Context.Dr2);
        printf("    DR3\t\t\t  0x%p\t\t   ", m_Context.Dr3);
        PRINTFREGVAULE(m_Context.Dr3);
        printf("-----------------------------------------------------------------\r\n");
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }
        
    //bhc 删除硬件断点
	case 15:
        m_MyCmdList.push_back(strCurCmd);
        DelHWBreakPointFlag(nBegAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
		
	//bmr 内存读断点
	case 16:
    {
        m_MyCmdList.push_back(strCurCmd);
        if (nBegAddress == 0)
        {
            char szTmp[18] = { 0 };
            int i = 0, j = 0;
            while (bufferAddr[i] != '\0')
            {
                if (bufferAddr[i] == '-')
                {
                    j = i;
                    break;
                }
                szTmp[i] = bufferAddr[i];
                i++;
            }
            memcpy(bufferEndAddr, &bufferAddr[j + 1], 18 - j - 1);
            StrConvertDec(bufferEndAddr, nEndAddress);
            StrConvertDec(szTmp, nBegAddress);
        }
        if (nEndAddress == 0)
        {
            nEndAddress = nBegAddress;
        }
        m_bRWBPointFlag.bMemRead = TRUE;
        DelInt3BreakPoint(nBegAddress);
        if (nBegAddress != 0 || nEndAddress != 0)
            SetMemReadWriteBPoint(nBegAddress, nEndAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }
        
    //bmw 内存写入断点
    case 17:
    {
        m_MyCmdList.push_back(strCurCmd);
        if (nBegAddress == 0)
        {
            char szTmp[18] = { 0 };
            int i = 0, j;
            while (bufferAddr[i] != '\0')
            {
                if (bufferAddr[i] == '-')
                {
                    j = i;
                    break;
                }
                szTmp[i] = bufferAddr[i];
                i++;
            }
            memcpy(bufferEndAddr, &bufferAddr[j + 1], 18 - j - 1);
            StrConvertDec(bufferEndAddr, nEndAddress);
            StrConvertDec(szTmp, nBegAddress);
        }
        m_bRWBPointFlag.bMemWrite = TRUE;
        DelInt3BreakPoint(nBegAddress);
        SetMemReadWriteBPoint(nBegAddress, nEndAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }

	//bml 内存断点列表
	case 18:
        printf("内存断点列表:\r\n");
        ShowMemBreakPoint();
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
		return;		

	//bmpl分页断点列表
	case 19:
		
		return;
	//bmc删除内存断点
	case 20:
    {
        m_MyCmdList.push_back(strCurCmd);
        int MemFlag = 0;
        if (nBegAddress == 0)
        {
            char szTmp[18] = { 0 };
            int i = 0;
            int j = 0;
            while (strLenOrAddr[i] != '\0')
            {
                if (strLenOrAddr[i] == '-')
                {
                    j = i;
                    break;
                }
                szTmp[i] = strLenOrAddr[i];
                i++;
            }
            memcpy(bufferEndAddr, &strLenOrAddr[j + 1], 18 - j - 1);
        }
        else
        {
            nEndAddress = nBegAddress;
        }
        DelMemBPoint(nBegAddress, (DWORD)nEndAddress, MemFlag);
        DelMemBreakPoint(nBegAddress, (DWORD)nEndAddress, MemFlag);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }
        
	//ls
	case 21:
        ImportScript();
        InputCMD();
        return;
		
	//es
	case 22:
        ExportScript();
        InputCMD();
        return;

	//q
	case 23:
        //DebugActiveProcessStop(m_dwPID);
        ExitProcess(0);
		return;

	//ml
	case 24:
        m_AnalysePE.ShowDllName();
		InputCMD();
		return;

    //cls
    case 25:
        system("cls");
        InputCMD();
        return;

    //trace
    case 26:
        if (nBegAddress != 0)
        {
            m_MyCmdList.push_back(strCurCmd);
            TraceProc(nBegAddress);
            return;
        }
        else
        {
            return;
        }

    //help
    case 27:
        ShowHelp();
        InputCMD();
        return;
	default:
		printf("Error：Please Input again!\r\n");
		InputCMD();
		return;
	}

	return;	
}

//获取线程信息
void MyDebuger::GetThreadInformation(DEBUG_EVENT &DebugEvent)
{

    HMODULE hModule = LoadLibrary("Kernel32.dll");
    
    pMyOpenThread pOpenThread = (pMyOpenThread)GetProcAddress(hModule,"OpenThread");
    
    m_hThread = pOpenThread(THREAD_ALL_ACCESS,NULL,DebugEvent.dwThreadId);  
    if(m_hThread==NULL)
    {
        return;   
    } 	
}

//单步步入
void MyDebuger::StepInto()
{
	m_Context.EFlags |= 0x100;
	m_EpnFlag.bSetInFlag = TRUE;
}

void MyDebuger::TraceProc(DWORD dwAddr)
{

}

//单步步过
void MyDebuger::StepOver()
{
	m_EpnFlag.bSetInFlag = FALSE;

	char szRcodeTmp[16] = {0};

	GetJumpInstruction((BYTE)m_CurCode.szCurLineCode[0],szRcodeTmp);
	if (szRcodeTmp[0] != NULL)
	{
		m_Context.EFlags |= 0x100;
		m_EpnFlag.bSetInFlag = TRUE;
		return;
	}


	// 	REP = (f3 f2) 和 CALL = (e8 9a) 指令解析 
	if ((unsigned char)m_CurCode.szCurLineCode[0] == 0xf3 ||
		(unsigned char)m_CurCode.szCurLineCode[0] == 0xf2 ||
		(unsigned char)m_CurCode.szCurLineCode[0] == 0xe8 ||
		(unsigned char)m_CurCode.szCurLineCode[0] == 0x9a ||		
		(unsigned char)m_CurCode.szCurLineCode[0] == 0xc3 ||
		(unsigned char)m_CurCode.szCurLineCode[0] == 0xff)
	{
		if ( m_CurCode.szCurLineCode[1] == 0x25 ||
			(unsigned char)m_CurCode.szCurLineCode[0] == 0xc3)
		{
			m_Context.EFlags |= 0x100;
			m_EpnFlag.bSetInFlag = TRUE;
			return;		
		}
		else
		{
			ReadProcessMemory(m_hProcess,(LPVOID)m_CurCode.pCurCodeEntryAddress,&m_GRunCode,sizeof(BYTE),NULL);
			WriteProcessMemory(m_hProcess,(LPVOID)m_CurCode.pCurCodeEntryAddress,(LPVOID)&code,sizeof(BYTE),NULL);
			m_CurCode.szSrcCode = m_GRunCode;
			m_EpnFlag.bGRunFlag = TRUE;
			return;
		}
	}
	else
	{
		m_Context.EFlags |= 0x100;
		m_EpnFlag.bSetJumpFlag = TRUE;
		return;
	}

		
}

void MyDebuger::StepIntoProc()
{
	char szCurCode[32] = {0};	
	m_CurCode.pCurCodeEntryAddress = (DWORD)m_Context.Eip;
	ReadProcessMemory(m_hProcess, (unsigned char *)m_CurCode.pCurCodeEntryAddress, &szCurCode, sizeof(szCurCode), NULL);
	m_CurCode.szCurLineCode[0] = szCurCode[0];
	m_CurCode.szCurLineCode[1] = szCurCode[1];

	ShowRegInfo();
	ShowAsmCode(szCurCode, m_CurCode.pCurCodeEntryAddress);
    m_EpnFlag.bSetInFlag = FALSE;
    if (FALSE == bSelIptFlag)
    {
        InputCMD();
    }
}

void MyDebuger::StepOverProc()
{
	//一次P处理
	m_EpnFlag.bSetJumpFlag = FALSE;

	char szCurCode[32] = {0};	
	ReadProcessMemory(m_hProcess, (unsigned char *)m_CurCode.pCurCodeEntryAddress, &szCurCode, sizeof(szCurCode), NULL);
	//保存当前代码
	m_CurCode.szCurLineCode[0] = szCurCode[0];
	m_CurCode.szCurLineCode[1] = szCurCode[1];

	ShowRegInfo();
	ShowAsmCode(szCurCode, m_CurCode.pCurCodeEntryAddress);
	if ( FALSE == bSelIptFlag )
	{
		InputCMD();
	}
}

//解析加载的DLL
void MyDebuger::AnalyseLoadDLL(LPDEBUG_EVENT DebugEv)
{
    LPLOAD_DLL_DEBUG_INFO pLoadDllDi = &DebugEv->u.LoadDll;
	char   szDllName[MAX_PATH] = {0};
	char   szDllBuffer[1024] = {0};
	DWORD  dwOldProto = 0;
  
	ReadProcessMemory(m_hProcess, (LPVOID)pLoadDllDi->lpBaseOfDll, szDllBuffer,sizeof(szDllBuffer),&dwOldProto);
	if (TRUE == m_AnalysePE.AnalysePeFile(szDllBuffer))
	{
        m_AnalysePE.GetDllName(m_hProcess, pLoadDllDi->lpBaseOfDll, szDllName);
        printf("解析加载的DLL文件:%s\r\n", szDllName);
		m_AnalysePE.GetExportTableInfo(m_hProcess, pLoadDllDi->lpBaseOfDll);
	}
}


