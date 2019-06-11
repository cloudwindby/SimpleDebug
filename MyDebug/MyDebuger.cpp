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
	//m_MemBPList �ͷ�NEW���Ŀռ�
	if ( m_BPMemDataTmp.RWMemBPList != NULL)
	{
		delete m_BPMemDataTmp.RWMemBPList;
		m_BPMemDataTmp.RWMemBPList = NULL;
	}			
	//�ͷ�
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
        printf("��������ʧ��. \r\n");
        CloseHandle(pi.hProcess);
        return FALSE;
    }

    m_dwPID = pi.dwProcessId;

	//���Թ���
	DebugLoop();
	
	return TRUE;
}


//        ���öϵ�: m_code = SetBreakPoint( pFunAdd, 0xCC); 
//		  �ָ��ϵ�: SetBreakPoint( pFunAdd, m_code);
BYTE MyDebuger::SetInt3BreakPoint(HANDLE hProcess,LPVOID pAdd, BYTE code,BOOL bFlag)
{
	BYTE b_read;
	BOOL bret;
	DWORD dwRead, dwOldFlg;
	// 0x80000000���ϵĵ�ַΪϵͳ�������򣬲��ܹ��޸�
	if( pAdd >= (LPVOID)0x80000000 || pAdd == (LPVOID)0)
		return code;
	// ȡ�����޸ĵ�ַ��һ���ֽڴ���
	bret = ReadProcessMemory(hProcess, pAdd, &b_read, sizeof(BYTE), &dwRead);
	// ���ԭ���Ĵ����׼���޸ĵĴ�����ͬ��û�б�Ҫ���޸�
	if(bret == 0 || b_read == code)
		return code;

	// �޸�ҳ������Ϊ�ɶ���д������ԭ����
	VirtualProtectEx(hProcess, pAdd, sizeof(BYTE), PAGE_READWRITE, &dwOldFlg);
	// �޸�Ŀ�����ΪCC
	WriteProcessMemory(hProcess, pAdd, (LPVOID)&code, sizeof(BYTE), &dwRead);
	// �ָ�ԭҳ������
	VirtualProtectEx(hProcess, pAdd, sizeof(BYTE), dwOldFlg, &dwOldFlg);
	return b_read;
}

//�����¼�ѭ��
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
				//�����쳣ǰȡ�û���
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
					//��ڶϵ�
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

//���ݵ�ַ���麯����
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

//�û����������
void MyDebuger::InputCMD(char* szCurCmd)
{
	int i, ch;
	int j = 0, k = 0, n = 0;  //��������ָ��ո�Ĳ���

	char strBuffer[24] = { 0 };           //�����ָ��
	char strCmdCmp[12] = { 0 };			  //�Ƚ�ָ��
	char strLenOrAddr[20] = { 0 };        //��2���ո������� ���Ȼ��ַ
	
    char bufferAddr[18] = { 0 };
	char bufferEndAddr[18] = { 0 };
    char strCurCmd[24] = { 0 };

	char *pIntpuCMD[30] = {"t", "p", "g", "u", "d", "r", "e", 
                            "bp","bpl","bpc",//һ��ϵ�
						    "bhe","bhw","bha","bhl", "bhc",//Ӳ���ϵ�
						    "bmr","bmw", "bml", "bmpl", "bmc", //�ڴ�ϵ�
						    "ls","es",//�ű�
                            "q","ml","cls","trace","help",}; 

	unsigned int nBegAddress = 0; 
	unsigned int nEndAddress = 0;
	unsigned int nLenOrAddr = 0;
	
	printf("-");


	//�ж��Ƿ�ӽű��л�ȡ����
	if ( TRUE == bSelIptFlag)
	{		
		for(i = 0; szCurCmd[i] != '\0' ; i++ )
		{
			if ( szCurCmd[i] == ' ' )
			{
				if ( j == 0 )
					j = i;				//�������ĵڶ����ո�ָ�ʼλ��
				else if ( k == 0 )
					k = i;				//�������ĵڶ����ո�ָ�ʼλ��
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
					j = i;				//�������ĵڶ����ո�ָ�ʼλ��
				else if ( k == 0 )
					k = i;				//�������ĵڶ����ո�ָ�ʼλ��
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
		//�ֽ��һ���ո�ǰ��ָ��
		memcpy(strCmdCmp, strBuffer, j);
		//�ֽ��һ���ո�� ���ڶ����ո�ǰ������
		memcpy(bufferAddr,&strBuffer[j+1], i-j-1);
		//�ַ�ת��
		StrConvertDec(bufferAddr, nBegAddress);
	}
	else if (k != 0)
	{
		//�ֽ��һ���ո�ǰ��ָ��
		memcpy(strCmdCmp, strBuffer, j);
		//�ֽ��һ���ո�� ���ڶ����ո�ǰ������
		memcpy(bufferAddr,&strBuffer[j+1], k-j-1);		
		StrConvertDec(bufferAddr, nBegAddress);

		//�ֽ�ڶ����ո�������
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
        //���Ӳ���ϵ��int3�ϵ��غ�,ʹint3�ϵ��Ϊ��Ч
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

    //bpl һ��ϵ��б�
	case 9:
    {
        m_MyCmdList.push_back(strCurCmd);
        printf("INT3�ϵ��б�:\r\n");
        printf("-----------------------------------------------------------------\r\n");
        printf("���\t\t  ��ַ\t\t  ״̬\r\n");

        int nBPNumber = 0;
        for (auto n : m_Int3BreakPointList)
        {
            if (n.BPAddress != NULL)
            {
                printf("%2d\t\t  0x%p\t\t  ", nBPNumber + 1, n.BPAddress);
                if (FALSE == n.DelFlag)
                {
                    printf("�ϵ���Ч\r\n");
                }
                    
                else
                {
                    printf("�ϵ����\r\n");
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
        
	//bpc ɾ��һ��ϵ�
	case 10:
    {
        m_MyCmdList.push_back(strCurCmd);
        if (TRUE == DelInt3BreakPoint(nBegAddress))
        {
            printf("\t\tɾ���ɹ�!\r\n");
        }
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
    }
       
           
	//bhe Ӳ��ִ�жϵ�
	case 11:
		m_MyCmdList.push_back(strCurCmd);
        DelInt3BreakPoint(nBegAddress);
        SetHWExecuteBreakPoint(nBegAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }

        return;
    //bhw Ӳ��д��ϵ�
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
        //Ӳ��д��ϵ�
        SetHWWriteBreakPoint(nBegAddress, nLenOrAddr);
        return;
    }
        
    //bha Ӳ�����ʶϵ�
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
        //Ӳ������
        SetHWAccessBreakPoint(nBegAddress, nLenOrAddr);
        return;
    }
        
	//bhl Ӳ���ϵ��б�
	case 14:
    {
        m_MyCmdList.push_back(strCurCmd);
        printf("Ӳ���ϵ��б�:\r\n");
        printf("-----------------------------------------------------------------\r\n");
        printf(" Ӳ���ϵ�Ĵ���\t\t      ��ַ\t\t      ״̬\r\n");
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
        
    //bhc ɾ��Ӳ���ϵ�
	case 15:
        m_MyCmdList.push_back(strCurCmd);
        DelHWBreakPointFlag(nBegAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
        return;
		
	//bmr �ڴ���ϵ�
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
        
    //bmw �ڴ�д��ϵ�
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

	//bml �ڴ�ϵ��б�
	case 18:
        printf("�ڴ�ϵ��б�:\r\n");
        ShowMemBreakPoint();
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
		return;		

	//bmpl��ҳ�ϵ��б�
	case 19:
		
		return;
	//bmcɾ���ڴ�ϵ�
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
		printf("Error��Please Input again!\r\n");
		InputCMD();
		return;
	}

	return;	
}

//��ȡ�߳���Ϣ
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

//��������
void MyDebuger::StepInto()
{
	m_Context.EFlags |= 0x100;
	m_EpnFlag.bSetInFlag = TRUE;
}

void MyDebuger::TraceProc(DWORD dwAddr)
{

}

//��������
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


	// 	REP = (f3 f2) �� CALL = (e8 9a) ָ����� 
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
	//һ��P����
	m_EpnFlag.bSetJumpFlag = FALSE;

	char szCurCode[32] = {0};	
	ReadProcessMemory(m_hProcess, (unsigned char *)m_CurCode.pCurCodeEntryAddress, &szCurCode, sizeof(szCurCode), NULL);
	//���浱ǰ����
	m_CurCode.szCurLineCode[0] = szCurCode[0];
	m_CurCode.szCurLineCode[1] = szCurCode[1];

	ShowRegInfo();
	ShowAsmCode(szCurCode, m_CurCode.pCurCodeEntryAddress);
	if ( FALSE == bSelIptFlag )
	{
		InputCMD();
	}
}

//�������ص�DLL
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
        printf("�������ص�DLL�ļ�:%s\r\n", szDllName);
		m_AnalysePE.GetExportTableInfo(m_hProcess, pLoadDllDi->lpBaseOfDll);
	}
}


