#include "stdafx.h"
#include <iostream>
#include "MyDebuger.h"

int main()
{
    //��ȡ����ĳ�����
    char szName[MAX_PATH] = { 0 };
    printf("Please Input Application File Name:");
    std::cin >> szName;

    getchar();
    
    //�������ԻỰ
    MyDebuger DebugSession(szName);
    BOOL ret = DebugSession.CreateDebugProcess(szName);
    if (!ret)
    {
        printf("���̴���ʧ��!\r\n");
        return 0;
    }
	return 0;
}

