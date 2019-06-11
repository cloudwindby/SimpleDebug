#pragma once
#include "StdAfx.h"
#include <list>
#include <vector>
using namespace std;

// ��������
struct ExportFunc                        
{
	char  szFuncName[MAX_PATH] = { 0 };//������
    int   nExportOrdinal;               //��ŵ���
    DWORD dwFuncAddress;                //������ַ
};

//������
struct ExportTable
{
    char szDllName[MAX_PATH] = { 0 };//DLL��
    list<ExportFunc> ExportFuncList;//������������
};

class AnalysePE  
{
public:
	AnalysePE();
	~AnalysePE();

public:
	//����DLL
	BOOL AnalysePeFile(LPVOID lpBaseAddress);
    //��ȡDLL��(ͨ������DLL�¼��޷���ȡ��NTDLL������.�ʶ��⴦��DLL��)
    void GetDllName(HANDLE hProcess, LPVOID lpBaseAddress, char* szDllName);
    //��ȡ��������Ϣ
	void GetExportTableInfo(HANDLE hProcess,LPVOID lpBaseAddress);
    void ShowDllName();

public:	
	//���ļ��ľ��
    HANDLE m_hFile;
	//�ļ�ӳ����
    HANDLE m_hFileMapHandle;
	//�ļ�ӳ��
    LPVOID m_hFileMapImageBase;

	//Dosͷ
	PIMAGE_DOS_HEADER m_pFileDosHeader;
	//NTͷ
	PIMAGE_NT_HEADERS m_pFileNtHeader;

	//Fileͷ
	PIMAGE_FILE_HEADER m_pFileHeader;
	//��ѡͷ
	PIMAGE_OPTIONAL_HEADER32 m_pOptionalHeader;

	//�ڱ�λ��
	PIMAGE_SECTION_HEADER m_pFileSectionHeader;
	//IMAGE_DATA_DIRECTORYλ��
	PIMAGE_DATA_DIRECTORY m_pDataDirectory;

	//������
	IMAGE_EXPORT_DIRECTORY m_DllExportTable;

public:	
    //������
	ExportTable m_ExportTable;
    //����������
	list<ExportTable> m_ExportTableList;
    //����������ַRVA
    DWORD m_dwStartFunAddr;
   //��������ű�RVA
    DWORD m_dwStartFunNameOrd;
    //��������ַRVA
    DWORD m_dwStartFunName;
    //�ɵ��ڴ�����
    DWORD m_dwOldProtected;
};
