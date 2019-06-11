#include "stdafx.h"
#include "MyDebuger.h"

//Ӳ���ϵ㴦���¼�
void MyDebuger::HWBreakPointProc()
{
    int nDRIndex;

    //������õ�Ӳ����־
    if ((m_Context.Dr6 & 1) == 1)
    {
        m_HWBPointValue = m_Context.Dr7;

        m_Context.Dr7 &= 0xfffffffe;
        m_HWHMFlag = TRUE;
        nDRIndex = 1;

    }
    else if ((m_Context.Dr6 & 2) == 2)
    {
        m_HWBPointValue = m_Context.Dr7;
        m_Context.Dr7 &= 0xfffffffb;
        m_HWHMFlag = TRUE;
        nDRIndex = 2;

    }
    else if ((m_Context.Dr6 & 4) == 4)
    {
        m_HWBPointValue = m_Context.Dr7;
        m_Context.Dr7 &= 0xffffffef;
        m_HWHMFlag = TRUE;
        nDRIndex = 3;

    }
    else if ((m_Context.Dr6 & 8) == 8)
    {
        m_HWBPointValue = m_Context.Dr7;
        m_Context.Dr7 &= 0xffffffbf;
        m_HWHMFlag = TRUE;
        nDRIndex = 4;
    }

    if (m_HWHMFlag == TRUE)
    {
        char szCurCode[32] = { 0 };
        ReadProcessMemory(m_hProcess, (LPVOID)m_Context.Eip, szCurCode, sizeof(szCurCode), NULL);
        m_CurCode.pCurCodeEntryAddress = m_Context.Eip;
        m_CurCode.szCurLineCode[0] = szCurCode[0];
        m_CurCode.szCurLineCode[1] = szCurCode[1];

        switch (nDRIndex)
        {
        case 1:
            if (((m_Context.Dr7 >> 16) & 3) == 0)
            {
                printf("��ǰEIP��0x%p ����Ӳ��ִ�жϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 16) & 3) == 1)
            {
                printf("��ǰEIP��0x%p ����Ӳ��д��ϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 16) & 3) == 3)
            {
                printf("��ǰEIP��0x%p ����Ӳ�����ʶϵ�\r\n", m_Context.Eip);
            }

            break;
        case 2:
            if (((m_Context.Dr7 >> 20) & 3) == 0)
            {
                printf("��ǰEIP��0x%p ����Ӳ��ִ�жϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 20) & 3) == 1)
            {
                printf("��ǰEIP��0x%p ����Ӳ��д��ϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 20) & 3) == 3)
            {
                printf("��ǰEIP��0x%p ����Ӳ�����ʶϵ�\r\n", m_Context.Eip);
            }
            break;
        case 3:
            if (((m_Context.Dr7 >> 24) & 3) == 0)
            {
                printf("��ǰEIP��0x%p ����Ӳ��ִ�жϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 24) & 3) == 1)
            {
                printf("��ǰEIP��0x%p ����Ӳ��д��ϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 24) & 3) == 3)
            {
                printf("��ǰEIP��0x%p ����Ӳ�����ʶϵ�\r\n", m_Context.Eip);
            }
            break;
        case 4:
            if (((m_Context.Dr7 >> 28) & 3) == 0)
            {
                printf("��ǰEIP��0x%p ����Ӳ��ִ�жϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 28) & 3) == 1)
            {
                printf("��ǰEIP��0x%p ����Ӳ��д��ϵ�\r\n", m_Context.Eip);
            }
            else if (((m_Context.Dr7 >> 28) & 3) == 3)
            {
                printf("��ǰEIP��0x%p ����Ӳ�����ʶϵ�\r\n", m_Context.Eip);
            }
            break;
        default:
            break;
        }


        ShowRegInfo();
        ShowAsmCode(szCurCode, m_Context.Eip);

        m_Context.EFlags |= 0x100;
        m_EpnFlag.bHardWareSetinFlag = TRUE;
        m_HWHMFlag = FALSE;
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
    }
}

void MyDebuger::SetDR0HWBreakPoint(int PointType, int nLen)
{
    //���� 1�ֽ�<00> 2�ֽ�<01> 4�ֽ�<11> 
    switch (nLen)
    {
        //1�ֽ�
    case 1:
    {
        //����  ����<11>=3  д��<01>=1  ִ��<00>=0
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR0EVALUE;
            break;
            //д
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR0WVALUE;
            break;
            //����д
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR0RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //2�ֽ�
    case 2:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | TWODR0EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | TWODR0WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | TWODR0RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //4�ֽ�	
    case 4:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR0EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR0WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR0RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}
void MyDebuger::SetDR1HWBreakPoint(int PointType, int nLen)
{
    switch (nLen)
    {
        //1�ֽ�
    case 1:
    {
        //����  ����<11>=3  д��<01>=1  ִ��<00>=0
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR1EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR1WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR1RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //2�ֽ�
    case 2:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | TWODR1EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | TWODR1WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | TWODR1RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //4�ֽ�	
    case 4:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR1EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR1WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR1RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}
void MyDebuger::SetDR2HWBreakPoint(int PointType, int nLen)
{
    switch (nLen)
    {
        //1�ֽ�
    case 1:
    {
        //����  ����<11>=3  д��<01>=1  ִ��<00>=0
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR2EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR2WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR2RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //2�ֽ�
    case 2:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | TWODR2EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | TWODR2WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | TWODR2RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //4�ֽ�	
    case 4:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR2EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR2WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR2RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}
void MyDebuger::SetDR3HWBreakPoint(int PointType, int nLen)
{
    switch (nLen)
    {
        //1�ֽ�
    case 1:
    {
        //����  ����<11>=3  д��<01>=1  ִ��<00>=0
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR3EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR3WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | ONEDR3RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //2�ֽ�
    case 2:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | TWODR3EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | TWODR3WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | TWODR3RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    //4�ֽ�	
    case 4:
    {
        switch (PointType)
        {
            //ִ��
        case 0:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR3EVALUE;
            break;
            //д��
        case 1:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR3WVALUE;
            break;
            //����
        case 3:
            m_Context.Dr7 = m_Context.Dr7 | FOURDR3RWVALUE;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

}


void MyDebuger::SetHWBreakPoint(DWORD SetAddress, int DR, int PointType, int nLen)
{

    switch (DR)
    {
        //�Ĵ���DR0
    case 1:
    {
        m_Context.Dr0 = SetAddress;
        m_HWflag.DR0 = TRUE;
        SetDR0HWBreakPoint(PointType, nLen);
    }
    break;
    //�Ĵ���DR1
    case 2:
    {
        m_Context.Dr1 = SetAddress;
        m_HWflag.DR1 = TRUE;
        SetDR1HWBreakPoint(PointType, nLen);
    }
    break;
    //�Ĵ���DR2
    case 3:
    {
        m_Context.Dr2 = SetAddress;
        m_HWflag.DR2 = TRUE;
        SetDR2HWBreakPoint(PointType, nLen);
    }

    break;
    //�Ĵ���DR3
    case 4:
    {
        m_Context.Dr3 = SetAddress;
        m_HWflag.DR3 = TRUE;
        SetDR3HWBreakPoint(PointType, nLen);
    }
    break;
    }
    m_EpnFlag.bHardWareFlag = TRUE;
    if (FALSE == bSelIptFlag)
    {
        InputCMD();
    }
}

void MyDebuger::SetHWExecuteBreakPoint(DWORD ExecuteAddress)
{
    int nDRIndex;

    if (m_HWflag.DR0 == FALSE)
    {
        nDRIndex = 1;
        SetHWBreakPoint(ExecuteAddress, nDRIndex, 0);
        return;
    }
    else if (m_HWflag.DR1 == FALSE)
    {
        nDRIndex = 2;
        SetHWBreakPoint(ExecuteAddress, nDRIndex, 0);
        return;
    }
    else if (m_HWflag.DR2 == FALSE)
    {
        nDRIndex = 3;
        SetHWBreakPoint(ExecuteAddress, nDRIndex, 0);
        return;
    }
    else if (m_HWflag.DR3 == FALSE)
    {
        nDRIndex = 4;
        SetHWBreakPoint(ExecuteAddress, nDRIndex, 0);
        return;
    }
    else
    {
        printf("Ӳ���ϵ㱥�ͣ������!\r\n");
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
    }

}

//д��
void MyDebuger::SetHWWriteBreakPoint(DWORD WriteAddress, int nLen)
{
    int nDRIndex;

    if (m_HWflag.DR0 == FALSE)
    {
        nDRIndex = 1;
        SetHWBreakPoint(WriteAddress, nDRIndex, 1, nLen);
        return;
    }
    else if (m_HWflag.DR1 == FALSE)
    {
        nDRIndex = 2;
        SetHWBreakPoint(WriteAddress, nDRIndex, 1, nLen);
        return;
    }
    else if (m_HWflag.DR2 == FALSE)
    {
        nDRIndex = 3;
        SetHWBreakPoint(WriteAddress, nDRIndex, 1, nLen);
        return;
    }
    else if (m_HWflag.DR3 == FALSE)
    {
        nDRIndex = 4;
        SetHWBreakPoint(WriteAddress, nDRIndex, 1, nLen);
        return;
    }
    else
    {
        printf("Ӳ���ϵ㱥�ͣ������!\r\n");
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
    }

}

//����
void MyDebuger::SetHWAccessBreakPoint(DWORD RWriteAddress, int nLen)
{
    int nDRIndex;

    if (m_HWflag.DR0 == FALSE)
    {
        nDRIndex = 1;
        SetHWBreakPoint(RWriteAddress, nDRIndex, 3, nLen);
        return;
    }
    else if (m_HWflag.DR1 == FALSE)
    {
        nDRIndex = 2;
        SetHWBreakPoint(RWriteAddress, nDRIndex, 3, nLen);
        return;
    }
    else if (m_HWflag.DR2 == FALSE)
    {
        nDRIndex = 3;
        SetHWBreakPoint(RWriteAddress, nDRIndex, 3, nLen);
        return;
    }
    else if (m_HWflag.DR3 == FALSE)
    {
        nDRIndex = 4;
        SetHWBreakPoint(RWriteAddress, nDRIndex, 3, nLen);
        return;
    }
    else
    {
        printf("Ӳ���ϵ㱥�ͣ������!\r\n");
        if (FALSE == bSelIptFlag)
        {
            InputCMD();
        }
    }
}

//ɾ�����õ�Ӳ����־
void MyDebuger::DelHWBreakPointFlag(int DRindex)
{
    switch (DRindex)
    {
        //DR0
    case 1:
        //���
        m_Context.Dr7 &= 0xfffffffe;
        SetInt3BPointAgain(m_Context.Dr0);
        m_Context.Dr0 = 0;
        m_HWflag.DR0 = FALSE;
        break;
        //DR1
    case 2:
        m_Context.Dr7 &= 0xfffffffb;
        SetInt3BPointAgain(m_Context.Dr1);
        m_Context.Dr1 = 0;
        m_HWflag.DR1 = FALSE;
        break;
        //DR2
    case 3:
        m_Context.Dr7 &= 0xffffffef;
        SetInt3BPointAgain(m_Context.Dr2);
        m_Context.Dr2 = 0;
        m_HWflag.DR2 = FALSE;
        break;
        //DR3
    case 4:
        m_Context.Dr7 &= 0xffffffbf;
        SetInt3BPointAgain(m_Context.Dr3);
        m_Context.Dr3 = 0;
        m_HWflag.DR3 = FALSE;
        break;
    }
}