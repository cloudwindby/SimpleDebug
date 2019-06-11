#pragma once
#include "StdAfx.h"
#include <list>
#include <vector>
using namespace std;

// 导出函数
struct ExportFunc                        
{
	char  szFuncName[MAX_PATH] = { 0 };//函数名
    int   nExportOrdinal;               //序号导出
    DWORD dwFuncAddress;                //函数地址
};

//导出表
struct ExportTable
{
    char szDllName[MAX_PATH] = { 0 };//DLL名
    list<ExportFunc> ExportFuncList;//导出函数链表
};

class AnalysePE  
{
public:
	AnalysePE();
	~AnalysePE();

public:
	//解析DLL
	BOOL AnalysePeFile(LPVOID lpBaseAddress);
    //获取DLL名(通过加载DLL事件无法获取到NTDLL的名字.故额外处理DLL名)
    void GetDllName(HANDLE hProcess, LPVOID lpBaseAddress, char* szDllName);
    //获取导出表信息
	void GetExportTableInfo(HANDLE hProcess,LPVOID lpBaseAddress);
    void ShowDllName();

public:	
	//打开文件的句柄
    HANDLE m_hFile;
	//文件映射句柄
    HANDLE m_hFileMapHandle;
	//文件映射
    LPVOID m_hFileMapImageBase;

	//Dos头
	PIMAGE_DOS_HEADER m_pFileDosHeader;
	//NT头
	PIMAGE_NT_HEADERS m_pFileNtHeader;

	//File头
	PIMAGE_FILE_HEADER m_pFileHeader;
	//可选头
	PIMAGE_OPTIONAL_HEADER32 m_pOptionalHeader;

	//节表位置
	PIMAGE_SECTION_HEADER m_pFileSectionHeader;
	//IMAGE_DATA_DIRECTORY位置
	PIMAGE_DATA_DIRECTORY m_pDataDirectory;

	//导出表
	IMAGE_EXPORT_DIRECTORY m_DllExportTable;

public:	
    //导出表
	ExportTable m_ExportTable;
    //导出表链表
	list<ExportTable> m_ExportTableList;
    //导出函数地址RVA
    DWORD m_dwStartFunAddr;
   //函数名序号表RVA
    DWORD m_dwStartFunNameOrd;
    //函数名地址RVA
    DWORD m_dwStartFunName;
    //旧的内存属性
    DWORD m_dwOldProtected;
};
