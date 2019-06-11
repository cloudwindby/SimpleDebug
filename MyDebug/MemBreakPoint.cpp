#include "stdafx.h"
#include "MyDebuger.h"

//删除指定内存断点数据
void MyDebuger::DelMemBreakPoint(DWORD StartAddress, DWORD EndAddress, int MemFlag)
{
    MemBreakPointInfo *mBPList = NULL;
    POSITION Pos = m_ShowMemList.GetHeadPosition();

    for (int i = 0; i < m_ShowMemList.GetCount(); i++)
    {
        mBPList = &m_ShowMemList.GetNext(Pos);

        if (StartAddress == mBPList->dwBeginAddr &&
            EndAddress == mBPList->dwEndAddr &&
            TRUE == mBPList->bDelFlag)
        {
            mBPList->bDelFlag = FALSE;
        }
    }
}

//显示所有下内存断点数据
void MyDebuger::ShowMemBreakPoint()
{

    printf("   序  号\t\t         地  址\t\t           状   态\r\n");
    printf("-----------------------------------------------------------------\r\n");
    MemBreakPointInfo mBPList;
    POSITION Pos = m_ShowMemList.GetHeadPosition();

    for (int i = 0; i < m_ShowMemList.GetCount(); i++)
    {
        mBPList = m_ShowMemList.GetNext(Pos);

        if (TRUE == mBPList.bDelFlag)
        {
            printf("    %2d\t\t\t  0x%08X-0x%08X\t\t   ", i + 1, mBPList.dwBeginAddr,
                mBPList.dwEndAddr);
            if (FALSE == mBPList.bRWFlag)
                printf(" READ\r\n");
            else
                printf(" WRITE\r\n");
        }
    }
    printf("-----------------------------------------------------------------\r\n");

}


//删除已下的内存断点
BOOL MyDebuger::DelMemBPoint(DWORD StartAddress, DWORD EndAddress, int MemFlag)
{
    MemPageBreakPoint *mBPListNode = NULL;
    DWORD EpnPageAddr = StartAddress / 0x1000 * 0x1000;

    POSITION Pos = m_MemBPList.GetHeadPosition();

    //在已有断点分页信息的情况下，先遍历>
    for (int i = 0; i < m_MemBPList.GetCount(); i++)
    {
        mBPListNode = &m_MemBPList.GetNext(Pos);

        if (mBPListNode->BeginPageAddress == EpnPageAddr)
        {
            DWORD dwProtect = 0;
            MemBreakPointInfo *BPStartAddr;
            POSITION PosPage = mBPListNode->RWMemBPList->GetHeadPosition();
            CList<MemBreakPointInfo, MemBreakPointInfo> *RWListData = mBPListNode->RWMemBPList;
            int k = RWListData->GetCount();

            for (int j = 0; j < k; j++)
            {
                BPStartAddr = &RWListData->GetNext(PosPage);

                if (mBPListNode->BeginPageAddress == BPStartAddr->dwBeginAddr)
                {
                    StartAddress = mBPListNode->BeginPageAddress;
                }

                if (BPStartAddr->dwBeginAddr == StartAddress &&
                    BPStartAddr->dwEndAddr == EndAddress &&
                    BPStartAddr->bDelFlag == TRUE)
                {
                    if (1 == k)
                    {
                        VirtualProtectEx(m_hProcess, (void *)StartAddress, 1, mBPListNode->OldPageProtect, &dwProtect);
                        BPStartAddr->bDelFlag = FALSE;
                    }
                    else
                    {
                        BPStartAddr->bDelFlag = FALSE;
                    }
                }

            }

        }
        EpnPageAddr += 0x1000;
    }
    return FALSE;
}

//查找相同的内存分页
BOOL MyDebuger::FindMemPage(DWORD PageAddress, DWORD EndAddress)
{
    POSITION Pos = m_MemBPList.GetHeadPosition();
    //在已有断点分页信息的情况下，先遍历>
    for (int i = 0; i < m_MemBPList.GetCount(); i++)
    {
        MemPageBreakPoint *PageListTmp = &m_MemBPList.GetNext(Pos);

        //找到相同的分页  返回
        if (PageListTmp->BeginPageAddress == PageAddress)
        {
            //更改分页地址的最后地址
            if (PageListTmp->EndPageAddress < EndAddress)
                PageListTmp->EndPageAddress = EndAddress;

            return TRUE;
        }
    }
    return FALSE;
}

void MyDebuger::SetMemReadWriteBPoint(DWORD StartAddress, DWORD EndAddress)
{
    //内存断点
    DWORD		  StartPage;
    DWORD		  EndPage = 0;
    //MemPageBreakPoint BPMemData;
    MemBreakPointInfo BPointInfor;
    ZeroMemory(&m_BPMemData, sizeof(m_BPMemData));
    ZeroMemory(&BPointInfor, sizeof(BPointInfor));
    m_BPMemData.RWMemBPList = new CList<MemBreakPointInfo, MemBreakPointInfo>;


    //得到当前下断地址的分页块
    StartPage = (StartAddress / 0x1000) * 0x1000;
    if (EndAddress != 0)
    {
        EndPage = (EndAddress / 0x1000) * 0x1000;
    }

    //当前断点信息
    BPointInfor.dwBeginAddr = StartAddress;
    BPointInfor.bDelFlag = TRUE;
    if (0 == EndAddress)
        BPointInfor.dwEndAddr = StartAddress;
    else
        BPointInfor.dwEndAddr = EndAddress;

    //保存一个链表，用于查询
    if (TRUE == m_bRWBPointFlag.bMemRead)
    {
        BPointInfor.bRWFlag = READ;
    }
    if (TRUE == m_bRWBPointFlag.bMemWrite)
    {
        BPointInfor.bRWFlag = WRITE;
    }
    m_ShowMemList.AddTail(BPointInfor);

    POSITION Pos = m_MemBPList.GetHeadPosition();
    DWORD StartPageTmp = StartPage;
    //若找到首页地址存在就直接添加地址<在已有断点分页信息的情况下，先遍历>
    for (int i = 0; i < m_MemBPList.GetCount(); i++)
    {
        MemPageBreakPoint *BPMemListData = &m_MemBPList.GetNext(Pos);

        //下断区域块在同一分页处理  只需增加下断地址
        if (BPMemListData->BeginPageAddress == StartPageTmp)
        {
            MemBreakPointInfo *pMemInfor = NULL;
            BOOL bSelWrite = FALSE;     //当前仅当该页无任何断点的情况下 才设置
            POSITION PosTmp = BPMemListData->RWMemBPList->GetHeadPosition();

            for (int j = 0; j < BPMemListData->RWMemBPList->GetCount(); j++)
            {
                pMemInfor = &BPMemListData->RWMemBPList->GetNext(PosTmp);
                //判断该页是否存在内存断点， 包括已经删除的内存断点全部扫描
                if (TRUE == pMemInfor->bDelFlag)
                {
                    bSelWrite = TRUE;
                }
            }

            DWORD dwProtect;
            if (TRUE == m_bRWBPointFlag.bMemRead)
            {
                BPointInfor.bRWFlag = READ;
                //m_bRWBPointFlag.bMemRead = FALSE;
                VirtualProtectEx(m_hProcess, (void *)StartPage, 1, PAGE_NOACCESS, &dwProtect);
            }


            if (TRUE == m_bRWBPointFlag.bMemWrite)
            {
                BPointInfor.bRWFlag = WRITE;
                if (!bSelWrite)
                {
                    VirtualProtectEx(m_hProcess, (void *)StartPage, 1, PAGE_EXECUTE_READ, &dwProtect);
                }
            }

            if (StartPage == StartPageTmp)
                BPMemListData->RWMemBPList->AddTail(BPointInfor);

            if (BPMemListData->EndPageAddress < EndAddress)
                BPMemListData->EndPageAddress = EndAddress;

            //找到已存在的页面，保存下断点首地址 根据条件直接可退出
            //StartPage == EndPage  同一页面 相当于  bpmr 地址-地址
            if (0 == EndPage || StartPage == EndPage || StartPageTmp >= EndPage)
            {
                m_bRWBPointFlag.bMemRead = FALSE;
                m_bRWBPointFlag.bMemWrite = FALSE;
                return;
            }

            StartPageTmp += 0x1000;
        }
    }

    //首地址和结束地址重合的情况处理
    if ((StartPage == EndPage) || (0 == EndPage))
    {
        m_BPMemData.BeginPageAddress = StartPage;

        if (0 == EndAddress)
            m_BPMemData.EndPageAddress = StartAddress;
        else
            m_BPMemData.EndPageAddress = EndAddress;

        if (TRUE == m_bRWBPointFlag.bMemRead)
        {
            BPointInfor.bRWFlag = READ;
            VirtualProtectEx(m_hProcess, (void *)StartAddress, 1, PAGE_NOACCESS, &m_BPMemData.OldPageProtect);
            m_bRWBPointFlag.bMemRead = FALSE;
        }
        if (TRUE == m_bRWBPointFlag.bMemWrite)
        {
            BPointInfor.bRWFlag = WRITE;
            VirtualProtectEx(m_hProcess, (void *)StartAddress, 1, PAGE_EXECUTE_READ, &m_BPMemData.OldPageProtect);
            m_bRWBPointFlag.bMemWrite = FALSE;
        }
        m_BPMemData.RWMemBPList->AddTail(BPointInfor);
        m_MemBPList.AddTail(m_BPMemData);
        return;
    }
    else
    {
        //内存跨分页的处理情况
        DWORD StartPageAddTmp = StartPage;
        if (StartPageTmp != StartPage)
        {
            //断点起始位置分页已存在且加入，所以从已循环到分页的下个分页开始设置
            StartPage = StartPageTmp;
        }
        while (StartPage <= EndPage)
        {
            //MemPageBreakPoint m_BPMemDataTmp;
            ZeroMemory(&m_BPMemDataTmp, sizeof(m_BPMemDataTmp));
            m_BPMemDataTmp.RWMemBPList = new CList<MemBreakPointInfo, MemBreakPointInfo>;
            m_BPMemDataTmp.BeginPageAddress = StartPage;
            m_BPMemDataTmp.EndPageAddress = EndPage;

            MemBreakPointInfo BPointInforTmp;
            ZeroMemory(&BPointInforTmp, sizeof(BPointInforTmp));
            if (StartPageAddTmp == StartPage)
                BPointInforTmp.dwBeginAddr = StartAddress;
            else
                BPointInforTmp.dwBeginAddr = StartPage;
            BPointInforTmp.dwEndAddr = EndAddress;
            BPointInforTmp.bDelFlag = TRUE;

            if (TRUE == m_bRWBPointFlag.bMemRead)
            {
                BPointInfor.bRWFlag = READ;
                BPointInforTmp.bRWFlag = READ;
                VirtualProtectEx(m_hProcess, (void *)StartPage, 1, PAGE_NOACCESS, &m_BPMemDataTmp.OldPageProtect);

            }
            if (TRUE == m_bRWBPointFlag.bMemWrite)
            {
                BPointInfor.bRWFlag = WRITE;
                BPointInforTmp.bRWFlag = WRITE;
                VirtualProtectEx(m_hProcess, (void *)StartPage, 1, PAGE_EXECUTE_READ, &m_BPMemDataTmp.OldPageProtect);

            }
            if (StartPageAddTmp == StartPage)
                m_BPMemDataTmp.RWMemBPList->AddTail(BPointInfor);
            else
            {
                BPointInfor.dwBeginAddr = StartPage;
                m_BPMemDataTmp.RWMemBPList->AddTail(BPointInforTmp);
            }

            if (FindMemPage(StartPage, EndAddress) == FALSE)
            {
                m_MemBPList.AddTail(m_BPMemDataTmp);
            }

            StartPage += 0x1000;
        }
        m_bRWBPointFlag.bMemRead = FALSE;
        m_bRWBPointFlag.bMemWrite = FALSE;
        return;
    }
}