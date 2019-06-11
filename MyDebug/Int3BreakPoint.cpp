#include "stdafx.h"
#include "MyDebuger.h"
BOOL MyDebuger::FindInt3BreakPoint(DWORD BreakAddress, char *Code)
{
    for (auto n: m_Int3BreakPointList)
    {
        if (n.BPAddress == BreakAddress)
        {
            strcpy(Code,(const char *)n.b_code);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL MyDebuger::DelInt3BreakPoint(DWORD BreakAddress)
{
    for (list<Int3BreakPoint>::iterator it = m_Int3BreakPointList.begin();it != m_Int3BreakPointList.end();it++)
    {
        if (it->BPAddress == BreakAddress)
        {
            SetInt3BreakPoint(m_hProcess, (LPVOID)it->BPAddress, it->b_code);
           // n.DelFlag = FALSE;
            m_EpnFlag.bSetInt3Flag = FALSE;
            it = m_Int3BreakPointList.erase(it);
            return TRUE;
        }
    }

    return FALSE;
}

void MyDebuger::SetOneTimeBreakPoint(DWORD BreakAddress)
{
    BYTE szBuf;
    Int3BreakPoint pInt3Point;
    memset(&pInt3Point, 0, sizeof(pInt3Point));
    char CodeTmp[8] = { 0 };


    ReadProcessMemory(m_ProcessInfo.hProcess, (unsigned char *)BreakAddress, &szBuf, sizeof(char), NULL);
    SetInt3BreakPoint(m_ProcessInfo.hProcess, (LPVOID)BreakAddress, code);

    pInt3Point.b_code = szBuf;
    pInt3Point.BPAddress = (DWORD)BreakAddress;
    pInt3Point.DelFlag = TRUE;



    if (FALSE == FindInt3BreakPoint(BreakAddress, CodeTmp))
    {
        m_Int3BreakPointList.push_back(pInt3Point);
        printf("设置一个INT3断点,地址:0x%p\r\n", BreakAddress);
    }
    else
    {
        for (auto n: m_Int3BreakPointList)
        {
            if (n.BPAddress == BreakAddress && n.DelFlag == FALSE)
            {
                SetInt3BreakPoint(m_ProcessInfo.hProcess, (LPVOID)BreakAddress, code);
                n.DelFlag = TRUE;
                return;
            }
        }
        printf("\t当前地址已经存在int3断点:0x%p\r\n", BreakAddress);
    }
}
void MyDebuger::RunInt3BreakPoint(DWORD RunAddressData)
{
    m_EpnFlag.bGRunFlag = TRUE;

    m_CurCode.pCurCodeEntryAddress = RunAddressData;
    m_GRunCode = SetInt3BreakPoint(m_hProcess, (LPVOID)RunAddressData, code);
}

BOOL MyDebuger::SetInt3BPointAgain(DWORD BreakAddress)
{
    for (auto n : m_Int3BreakPointList)
    {
        if (n.BPAddress == BreakAddress && n.DelFlag == FALSE)
        {
            n.DelFlag = TRUE;
            SetInt3BreakPoint(m_hProcess, (LPVOID)n.BPAddress, code);
            return TRUE;
        }
    }

    return FALSE;
}