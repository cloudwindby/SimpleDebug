#include "stdafx.h"
#include "AnalysePE.h"

AnalysePE::AnalysePE()
{
    m_hFile = NULL;
    m_hFileMapHandle = NULL;

    m_pFileDosHeader = NULL;
    m_pFileNtHeader = NULL;
    m_pFileSectionHeader = NULL;

}

AnalysePE::~AnalysePE()
{

}

BOOL AnalysePE::AnalysePeFile(LPVOID lpBaseAddress)
{
    m_pFileDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
    if (m_pFileDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return FALSE;
    }

    m_pFileNtHeader = (IMAGE_NT_HEADERS *)((int)lpBaseAddress + m_pFileDosHeader->e_lfanew);

    if (m_pFileNtHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        return FALSE;
    }

    m_pFileHeader = &m_pFileNtHeader->FileHeader;

    m_pFileSectionHeader = (IMAGE_SECTION_HEADER *)((int)lpBaseAddress + m_pFileDosHeader->e_lfanew
        + sizeof(DWORD)
        + m_pFileHeader->SizeOfOptionalHeader
        + sizeof(IMAGE_FILE_HEADER));



    //可选头
    m_pOptionalHeader = &m_pFileNtHeader->OptionalHeader;

    //数据目录
    m_pDataDirectory = m_pOptionalHeader->DataDirectory;

    return TRUE;
}

void AnalysePE::GetDllName(HANDLE hProcess, LPVOID lpBaseAddress, char * szDllName)
{
    DWORD ImportAddress = m_pDataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    //判断是否存在导出表
    if (ImportAddress != 0)
    {
        //属性
        char szDllNameTmp[MAX_PATH] = { 0 };
        int k = 0;

        //直接取导出表的结构
        int nExportAddr = m_pDataDirectory->VirtualAddress + (DWORD)lpBaseAddress;
        ReadProcessMemory(hProcess, (LPVOID)nExportAddr,
            &m_DllExportTable, sizeof(IMAGE_EXPORT_DIRECTORY), &m_dwOldProtected);

        //导出函数地址RVA
        m_dwStartFunAddr = (int)(m_DllExportTable.AddressOfFunctions + (DWORD)lpBaseAddress);
        //函数名序号表RVA
        m_dwStartFunNameOrd = (DWORD)(m_DllExportTable.AddressOfNameOrdinals + (DWORD)lpBaseAddress);
        //函数名地址RVA
        m_dwStartFunName = (DWORD)(m_DllExportTable.AddressOfNames + (DWORD)lpBaseAddress);

        DWORD dwFunNameAddr = (DWORD)(m_DllExportTable.Name + (DWORD)lpBaseAddress);
        ReadProcessMemory(hProcess, (LPVOID)dwFunNameAddr,
            (LPVOID)szDllName, 24, &m_dwOldProtected);
        while (*(szDllName + k) != '\0')
        {
            k++;
        }
        strncpy(szDllNameTmp, szDllName, k - 4);
        memcpy(m_ExportTable.szDllName, szDllNameTmp, k);
    }
}

//导出表
void AnalysePE::GetExportTableInfo(HANDLE hProcess, LPVOID lpBaseAddress)
{
    int i = 0;
    int j = 0;
    int nAllFunOffset = 0;
    WORD nFunNameAddr = 0;
    DWORD nCurAddress = 0;
    char szFunName[MAX_PATH] = { 0 };

    ExportFunc pEptFunInfor;
    ZeroMemory(&pEptFunInfor, sizeof(ExportFunc));

    for (i = 0; i <= (int)m_DllExportTable.NumberOfFunctions; i++)
    {
        ReadProcessMemory(hProcess, (LPCVOID)m_dwStartFunAddr,
            &nAllFunOffset, sizeof(int), &m_dwOldProtected);
        if (0 == nAllFunOffset)
        {
            //循环读取链表
            m_dwStartFunAddr += sizeof(DWORD);
            continue;
        }

        for (j = 0; j <= (int)m_DllExportTable.NumberOfNames; j++)
        {

            ReadProcessMemory(hProcess, (LPVOID)(m_dwStartFunNameOrd + j * 2),
                &nFunNameAddr, sizeof(WORD), &m_dwOldProtected);

            if (i == (int)nFunNameAddr)
            {
                memset(szFunName, 0, MAX_PATH - 1);

                DWORD nAddress = j * sizeof(DWORD) + m_dwStartFunName;

                ReadProcessMemory(hProcess, (LPVOID)(nAddress),
                    &nCurAddress, sizeof(DWORD), &m_dwOldProtected);

                nCurAddress += (DWORD)lpBaseAddress;
                ReadProcessMemory(hProcess, (LPVOID)(nCurAddress),
                    szFunName, MAX_PATH, &m_dwOldProtected);
                break;
            }
        }

        if (szFunName != NULL)
        {
            strcpy(pEptFunInfor.szFuncName, szFunName);
        }
        pEptFunInfor.nExportOrdinal = i + m_DllExportTable.Base;
        pEptFunInfor.dwFuncAddress = nAllFunOffset + (int)lpBaseAddress;

        m_ExportTable.ExportFuncList.push_back(pEptFunInfor);
        //循环读取链表
        m_dwStartFunAddr += sizeof(DWORD);
    }
    m_ExportTableList.push_back(m_ExportTable);
}

void AnalysePE::ShowDllName()
{
    for (auto n : m_ExportTableList)
    {
       printf("模块:%s\r\n",n.szDllName);
    }
}
