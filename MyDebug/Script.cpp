#include "stdafx.h"
#include "MyDebuger.h"

//导出脚本
void MyDebuger::ExportScript()
{
    int nCount = 0;
    int j = 0;
    DWORD dwOut;

    char szCmdTmp[MAX_PATH] = { 0 };
    char szFileName[MAX_PATH] = { 0 };

    strncpy(szFileName, m_pFileName, strlen(m_pFileName)-4);
    strcat(szFileName, ".scp");

    HANDLE hfile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    memset(szCmdTmp, 0, sizeof(szCmdTmp));

        for (auto n : m_MyCmdList)
        {
            strcpy(szCmdTmp,n);
            strcat(szCmdTmp, "\r\n");
            WriteFile(hfile, szCmdTmp, strlen(szCmdTmp), &dwOut, NULL);
        }

    bSelectEptFlag = TRUE;
    CloseHandle(hfile);
    printf("脚本保存成功!\r\n");

}

//导入脚本文件
void MyDebuger::ImportScript()
{
    char szFileName[MAX_PATH] = { 0 };
    OPENFILENAME ofn;
    HANDLE hFile;
    HANDLE hFileMap;
    LPVOID hFileMapView;

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.nFilterIndex = 1;
    if (GetOpenFileNameA(&ofn))
    {
        hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            printf("打开文件失败!\r\n");
            return;
        }

        hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, NULL, NULL, NULL);
        if (hFileMap == NULL)
        {
            printf("打开文件失败!\r\n");
            CloseHandle(hFile);
            return;
        }

        hFileMapView = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (hFileMapView == NULL)
        {
            printf("打开文件失败!\r\n");
            CloseHandle(hFileMap);
            CloseHandle(hFile);
            return;
        }

        //开始读取指令
        int i = 0, j = 0;
        char strCmdTmp[256] = { 0 };
        char strCmd[256] = { 0 };
        char *pStr = (char*)hFileMapView;

        while (*(pStr + i) != '\0')
        {
            if (*(pStr + i) == '\r' && *(pStr + i + 1) == '\n')
            {
                strcpy(strCmd, strCmdTmp);
                bSelIptFlag = TRUE;
                InputCMD(strCmd);
                bSelIptFlag = FALSE;
                j = 0;
                i += 2;
            }
            strCmdTmp[j] = *(pStr + i);
            i++;
            j++;
        }

        m_MyCmdList.clear();

        if (hFileMapView)
        {
            UnmapViewOfFile(hFileMapView);
        }
        if (hFile)
        {
            CloseHandle(hFile);
        }
    }
}