#include "stdafx.h"
#include "MyDebuger.h"

//访问异常处理
void MyDebuger::AccessExceptionProc(EXCEPTION_RECORD RecordTmp)
{
    BOOL bRWMemFlag;
    if (RecordTmp.ExceptionInformation[0] == NULL)
    {
        m_RWMemFlag = READ;
        bRWMemFlag = READ;
        //bDelRWMemFlag = READ;
    }
    else
    {
        m_RWMemFlag = WRITE;
        bRWMemFlag = WRITE;
        //bDelRWMemFlag = WRITE;
    }

    POSITION Pos = m_MemBPList.GetHeadPosition();

    unsigned int EpnPageAddr = RecordTmp.ExceptionInformation[1] / 0x1000 * 0x1000;
    MemPageBreakPoint mBPListNode;
    DWORD dwProtect = 0;
    char szEpnCode[32] = { 0 };

    //分页
    for (int i = 0; i < m_MemBPList.GetCount(); i++)
    {
        mBPListNode = m_MemBPList.GetNext(Pos);

        if (mBPListNode.BeginPageAddress == EpnPageAddr)
        {
            MemBreakPointInfo BPStartAddr;
            POSITION PosPage = NULL;
            CList<MemBreakPointInfo, MemBreakPointInfo> *RWListData = NULL;


            PosPage = mBPListNode.RWMemBPList->GetHeadPosition();
            RWListData = mBPListNode.RWMemBPList;

            for (int j = 0; j < RWListData->GetCount(); j++)
            {
                BPStartAddr = RWListData->GetNext(PosPage);

                //判断命中断点地址
                if ((BPStartAddr.dwBeginAddr <= (RecordTmp.ExceptionInformation[1])) &&
                    ((RecordTmp.ExceptionInformation[1]) <= (BPStartAddr.dwEndAddr)) &&
                    (BPStartAddr.bDelFlag == TRUE) && 
                    (!BPStartAddr.bRWFlag || (bRWMemFlag == WRITE)))
                {
                    m_RWMemFlag = BPStartAddr.bRWFlag;
                    m_StartMemAddr = BPStartAddr.dwBeginAddr;
                    VirtualProtectEx(m_hProcess, (void *)BPStartAddr.dwBeginAddr, 1, mBPListNode.OldPageProtect, &dwProtect);
                    m_CurCode.pCurCodeEntryAddress = (DWORD)RecordTmp.ExceptionAddress;
                    ReadProcessMemory(m_hProcess, RecordTmp.ExceptionAddress, szEpnCode, sizeof(szEpnCode), NULL);
                    m_CurCode.szCurLineCode[0] = szEpnCode[0];
                    m_CurCode.szCurLineCode[1] = szEpnCode[1];

                    m_Context.EFlags |= 0x100;
                    m_MemStepFlag = TRUE;

                    if (READ == bRWMemFlag)
                    {
                        printf("当前EIP: 0x%08X, 触发内存断点地址: 0x%08X, 类型:读取内存\r\n", m_Context.Eip, RecordTmp.ExceptionInformation[1]);
                    }
                    else
                    {
                        printf("当前EIP: 0x%08X, 触发内存断点地址: 0x%08X, 类型:写入内存\r\n", m_Context.Eip, RecordTmp.ExceptionInformation[1]);
                    }

                    ShowRegInfo();
                    ShowAsmCode(szEpnCode, m_CurCode.pCurCodeEntryAddress);
                    if (FALSE == bSelIptFlag)
                    {
                        InputCMD();
                    }
                    return;
                }
                else
                {
                    m_RWMemFlag = BPStartAddr.bRWFlag;
                    m_StartMemAddr = BPStartAddr.dwBeginAddr;
                    VirtualProtectEx(m_hProcess, (void *)BPStartAddr.dwBeginAddr, 1, mBPListNode.OldPageProtect, &dwProtect);
                    m_CurCode.pCurCodeEntryAddress = (DWORD)RecordTmp.ExceptionAddress;
                    ReadProcessMemory(m_hProcess, RecordTmp.ExceptionAddress, szEpnCode, sizeof(szEpnCode), NULL);

                    m_Context.EFlags |= 0x100;
                    m_MemStepFlag = TRUE;
                }
            }
        }
    }

    if (FALSE == m_MemStepFlag)
    {
        m_bExNOProc = TRUE;
    }
}

//断点访问异常
void MyDebuger::BreakPointExceptionProc(EXCEPTION_RECORD RecordTmp)
{
    char szCurcode[32] = { 0 };

    //入口点INT3断点
    if (RecordTmp.ExceptionAddress == m_ProcessInfo.lpStartAddress)
    {
        SetInt3BreakPoint(m_ProcessInfo.hProcess, m_ProcessInfo.lpStartAddress, m_oepCode);
        m_Context.Eip--;
        ReadProcessMemory(m_ProcessInfo.hProcess, m_ProcessInfo.lpStartAddress, &szCurcode, sizeof(szCurcode), NULL);

        ShowRegInfo();
        ShowAsmCode(szCurcode, (DWORD)m_ProcessInfo.lpStartAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
    }
    else if (m_EpnFlag.bGRunFlag)
    {
        m_EpnFlag.bGRunFlag = FALSE;
        WriteProcessMemory(m_hProcess, (LPVOID)m_CurCode.pCurCodeEntryAddress, &m_GRunCode, sizeof(char), NULL);
        m_Context.Eip--;
        ReadProcessMemory(m_hProcess, (LPVOID)m_CurCode.pCurCodeEntryAddress, szCurcode, sizeof(szCurcode), NULL);
        //1.分析CALL或JMP特殊指令 2.EIP特殊处理
        m_CurCode.szCurLineCode[0] = szCurcode[0];
        m_CurCode.szCurLineCode[1] = szCurcode[1];

        ShowRegInfo();
        ShowAsmCode(szCurcode, m_CurCode.pCurCodeEntryAddress);
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
    }
    else       //默认bp命令(int 3)处理
    {
        for (auto n : m_Int3BreakPointList)
        {
        
            if ((RecordTmp.ExceptionAddress == (void *)n.BPAddress) &&
                (TRUE == n.DelFlag))
            {
                SetInt3BreakPoint(m_hProcess, (LPVOID)n.BPAddress, n.b_code);
                m_Context.Eip--;
                ReadProcessMemory(m_hProcess, (LPVOID)n.BPAddress, szCurcode, sizeof(szCurcode), NULL);
                m_CurCode.pCurCodeEntryAddress = m_Context.Eip;
                //1.分析CALL或JMP特殊指令 2.EIP特殊处理
                m_CurCode.szCurLineCode[0] = szCurcode[0];
                m_CurCode.szCurLineCode[1] = szCurcode[1];

                printf("触发INT3断点: 0x%08X\r\n", m_Context.Eip);
                ShowRegInfo();
                ShowAsmCode(szCurcode, n.BPAddress);

                //置单步
                m_Context.EFlags |= 0x100;
                m_EpnFlag.bSetInt3Flag = TRUE;
                if (FALSE == bSelIptFlag)
                {
                    InputCMD();
                }
                break;
            }
        }
    }

}

//单步异常
void MyDebuger::StepExceptionProc(EXCEPTION_RECORD RecordTmp)
{
    //重置INT3断点
    if (TRUE == m_EpnFlag.bSetInt3Flag)
    {
        SetInt3BreakPoint(m_ProcessInfo.hProcess, (LPVOID)m_BpNode.BPAddress, code);
        m_EpnFlag.bSetInt3Flag = FALSE;
    }

    //硬件断点重设
    if (m_EpnFlag.bHardWareSetinFlag == TRUE)
    {
        m_Context.Dr7 = m_HWBPointValue;
        m_HWBPointValue = 0;
        m_EpnFlag.bHardWareSetinFlag = FALSE;
        return;
    }

    //单步步入
    if (m_EpnFlag.bSetInFlag == TRUE)
    {
        StepIntoProc();
        return;
    }

    //单步步过
    if (m_EpnFlag.bSetJumpFlag == TRUE)
    {
        StepOverProc();
        return;
    }

    if (m_MemStepFlag == TRUE)
    {
        DWORD dwProtect = 0;
        if (m_RWMemFlag == READ)
            VirtualProtectEx(m_hProcess, (void *)m_StartMemAddr, 1, PAGE_NOACCESS, &dwProtect);

        if (m_RWMemFlag == WRITE)
            VirtualProtectEx(m_hProcess, (void *)m_StartMemAddr, 1, PAGE_EXECUTE_READ, &dwProtect);

        //代码跟踪，保存当前程序运行时的EIP值
        m_CurCode.pCurCodeEntryAddress = m_Context.Eip;
        m_MemStepFlag = FALSE;
       
    }

    //硬件断点处理
    if (m_EpnFlag.bHardWareFlag == TRUE)
    {
        HWBreakPointProc();
    }
}
