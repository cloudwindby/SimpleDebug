#include "stdafx.h"
#include <iostream>
#include "MyDebuger.h"

int main()
{
    //获取输入的程序名
    char szName[MAX_PATH] = { 0 };
    printf("Please Input Application File Name:");
    std::cin >> szName;

    getchar();
    
    //创建调试会话
    MyDebuger DebugSession(szName);
    BOOL ret = DebugSession.CreateDebugProcess(szName);
    if (!ret)
    {
        printf("进程创建失败!\r\n");
        return 0;
    }
	return 0;
}

