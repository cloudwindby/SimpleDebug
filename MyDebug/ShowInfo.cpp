#include "stdafx.h"
#include "MyDebuger.h"

//��ʾ��Ϣ��صĺ���.(��ʾ�ڴ���Ϣ,������,�Ĵ���,������Ϣ,������,�����)

//��ʾ���õĺ���
void MyDebuger::ShowFuncName(char *szBuf)
{
    int i = 0;
    int n = strlen(szBuf);
    char szBufTmp[32] = { 0 };
    char szAddr[32] = { 0 };
    memcpy(szBufTmp, szBuf, 4);
    //����3��ָ�� ת����Ӧ����
    while (strcmp(szBufTmp, "call") == 0 ||
        strcmp(szBufTmp, "jmp ") == 0 ||
        strcmp(szBufTmp, "mov ") == 0)
    {
        if (*szBuf == NULL)
        {
            return;
        }
        for (i = 3; i < n; i++)
        {
            if ((szBuf[i] == '[') && (szBuf[i + 9] == ']'))
            {
                char szBuffer[9] = { 0 };
                char szTmp[8] = { 0 };
                DWORD dwOldProto = 0;
                DWORD nAddr = 0;
                unsigned int nFunAddrOffset = 0;

                char szJumpCode[256] = { 0 };
                UINT nCodeSize = 0;
                char szBuffer2[64] = { 0 };

                strncpy(szBuffer, (const char *)&szBuf[i + 1], 8);
                StrConvertDec(szBuffer, (unsigned int &)nAddr);

                if (0 == nAddr)
                    return;

                ReadProcessMemory(m_hProcess, (LPVOID)nAddr, &szBuffer2, sizeof(szBuffer2), &dwOldProto);
                Decode2Asm((PBYTE)szBuffer2, szJumpCode, &nCodeSize, m_CurCode.pCurCodeEntryAddress);
                memcpy(szTmp, szJumpCode, 4);
                if ((strcmp(szTmp, "call") == 0) ||
                    (strcmp(szTmp, "jmp ") == 0) ||
                    (strcmp(szBufTmp, "mov ") == 0))
                {
                    int j = strlen(szJumpCode);
                    for (i = 3; i < j; i++)
                    {
                        if ((szJumpCode[i] == '[') && (szJumpCode[i + 9] == ']') ||
                            (szJumpCode[i] == ' ') && (szJumpCode[i + 9] == NULL))
                        {
                            ShowFuncName(szJumpCode);
                        }
                    }
                    ReadProcessMemory(m_hProcess, (LPVOID)nAddr, &nFunAddrOffset, 4, &dwOldProto);
                    FindFuncNameByAddr(nFunAddrOffset);
                    return;
                }
                else
                {
                    ReadProcessMemory(m_hProcess, (LPVOID)nAddr, &nFunAddrOffset, 4, &dwOldProto);
                    FindFuncNameByAddr(nFunAddrOffset);
                    return;
                }
                return;
            }

            if ((szBuf[i] == ' ') && (szBuf[i + 9] == NULL))
            {
                char szBuffer[9] = { 0 };
                char szBuffer2[256] = { 0 };
                char szJumpCode[64] = { 0 };
                char szTmp[8] = { 0 };
                DWORD dwOldProto = 0;
                unsigned int nAddr = 0;
                UINT nCodeSize = 0;
                unsigned int nFunAddrOffset = 0;
                strncpy(szBuffer, (const char *)&szBuf[i + 1], 8);
                //�����Ĵ����ĵ���
                if (strcmp(szBuffer, "eax") == 0)
                {
                    FindFuncNameByAddr(m_Context.Eax);
                    return;
                }
                if (strcmp(szBuffer, "ebx") == 0)
                {
                    FindFuncNameByAddr(m_Context.Ebx);
                    return;
                }
                if (strcmp(szBuffer, "ecx") == 0)
                {
                    FindFuncNameByAddr(m_Context.Ecx);
                    return;
                }
                if (strcmp(szBuffer, "edx") == 0)
                {
                    FindFuncNameByAddr(m_Context.Edx);
                    return;
                }
                if (strcmp(szBuffer, "esi") == 0)
                {
                    FindFuncNameByAddr(m_Context.Esi);
                    return;
                }
                if (strcmp(szBuffer, "edi") == 0)
                {
                    FindFuncNameByAddr(m_Context.Edi);
                    return;
                }
                StrConvertDec(szBuffer, nAddr);

                if (0 == nAddr)
                    return;

                ReadProcessMemory(m_hProcess, (LPVOID)nAddr, &szBuffer2, sizeof(szBuffer2), &dwOldProto);
                Decode2Asm((PBYTE)szBuffer2, szJumpCode, &nCodeSize, m_CurCode.pCurCodeEntryAddress);
                memcpy(szTmp, szJumpCode, 4);
                if ((strcmp(szTmp, "call") == 0) ||
                    (strcmp(szTmp, "jmp ") == 0) ||
                    (strcmp(szBufTmp, "mov ") == 0))
                {
                    int j = strlen(szJumpCode);
                    for (i = 3; i < j; i++)
                    {
                        if ((szJumpCode[i] == '[') && (szJumpCode[i + 9] == ']') ||
                            (szJumpCode[i] == ' ') && (szJumpCode[i + 9] == NULL))
                        {
                            ShowFuncName(szJumpCode);
                        }
                    }
                    ReadProcessMemory(m_hProcess, (LPVOID)nAddr, &nFunAddrOffset, 4, &dwOldProto);
                    FindFuncNameByAddr(nFunAddrOffset);
                    return;
                }
                else
                {
                    ReadProcessMemory(m_hProcess, (LPVOID)nAddr, &nFunAddrOffset, 4, &dwOldProto);
                    FindFuncNameByAddr(nFunAddrOffset);
                    return;
                }
                return;
            }
        }
        if (i == n)
            break;
    }
}

//U �����
void MyDebuger::ShowCurCode(DWORD CurCodeAddr)
{
    int i, j, k;
    char szBuf[128] = { 0 };
    UINT nCodeSize;
    char szCude[128] = { 0 };
    char szInt3Code[8] = { 0 };
    char szFunName[128] = { 0 };
    static unsigned int sOldEip = 0;
    //��¼��ǰ�ĵ�ַ
    if (CurCodeAddr == 0)
    {
        if (m_CurCodeAddress == 0)
        {
            sOldEip = m_CurCodeAddress = m_CurCode.pStartAddress;
        }
        if (sOldEip != m_Context.Eip)
        {
            m_CurCodeAddress = m_CurCode.pCurCodeEntryAddress;
            sOldEip = m_Context.Eip;
        }

    }
    else
        m_CurCodeAddress = CurCodeAddr;

    for (i = 0; i < 10; i++)
    {
        memset(szCude, 0, sizeof(szCude));
        memset(szBuf, 0, sizeof(szBuf));
        memset(szInt3Code, 0, sizeof(szInt3Code));
        memset(szFunName, 0, sizeof(szFunName));


        ReadProcessMemory(m_hProcess, (PVOID)m_CurCodeAddress, szCude, sizeof(szCude), NULL);
        //�����ǰ������ϵ� ��ԭ��ԭ���Ĵ������
        if (TRUE == FindInt3BreakPoint(m_CurCodeAddress, szInt3Code))
        {
            szCude[0] = szInt3Code[0];
        }
        Decode2Asm((PBYTE)szCude, szBuf, &nCodeSize, m_CurCodeAddress);

        printf("%08X  ", m_CurCodeAddress);
        for (j = 0; j < (int)nCodeSize; j++)
        {
            printf("%02X ", (unsigned char)(*(szCude + j)));
        }
        j = 20 - (int)nCodeSize - j * 2;
        for (k = 0; k <= j; k++)
        {
            printf(" ");
        }
        printf("%-30s \r\n", szBuf);

        ShowFuncName(szBuf);

        m_CurCodeAddress += nCodeSize;
    }
    if (FALSE == bSelIptFlag)
    {
        InputCMD();
    }
}


void MyDebuger::ShowAsmCode(char* szPcode, DWORD CurAddress)
{
    char szBuf[128] = { 0 };
    char szCodeTmp[32] = { 0 };
    char szInt3Code[1] = { 0 };
    UINT nCodeSize;
    strcpy(szCodeTmp, szPcode);

    Decode2Asm((PBYTE)szPcode, szBuf, &nCodeSize, CurAddress);
    printf("%08X   ", CurAddress);
    for (int i = 0; i < (int)nCodeSize; i++)
    {
        if (0xCC == (unsigned char)(*(szPcode + i)))
        {
            //�����ǰ������ϵ� ��ԭ��ԭ���Ĵ������
            if ((TRUE == FindInt3BreakPoint(CurAddress, szInt3Code)) || m_GRunCode)
            {
                szCodeTmp[0] = (unsigned char)(*(szInt3Code + i));
                Decode2Asm((PBYTE)szCodeTmp, szBuf, &nCodeSize, CurAddress);
                printf("%02X ", (unsigned char)(*(szInt3Code + i)));
            }
        }
        else
            printf("%02X ", (unsigned char)(*(szPcode + i)));
    }
    printf("\t%s\r\n", szBuf);
    //��SaveCurCode��ǰ�ṹ�帳ֵ
    strcpy(m_CurCode.szCurAsmBuf, szBuf);
    m_CurCode.nCodeSize = nCodeSize;
    m_CurCode.pCurCodeEntryAddress += nCodeSize;

    ShowFuncName(szBuf);
}

void MyDebuger::ShowRegInfo()
{
    printf("EAX = %08X ", m_Context.Eax);
    printf("ECX = %08X ", m_Context.Ecx);
    printf("EDX = %08X ", m_Context.Edx);
    printf("EBX = %08X ", m_Context.Ebx);
    printf("ESP = %08X \r\n", m_Context.Esp);
    printf("EBP = %08X ", m_Context.Ebp);
    printf("ESI = %08X ", m_Context.Esi);
    printf("EDI = %08X ", m_Context.Edi);
    printf("EIP = %08X \r\n", m_Context.Eip);
    printf("CS=%04X  DS=%04X  ES=%04X  SS=%04X  FS=%04X\t",
        m_Context.SegCs, m_Context.SegDs,
        m_Context.SegEs, m_Context.SegSs,
        m_Context.SegFs);
    printf("OF IF TF SF ZF AF PF CF\r\n");
    printf("\t\t\t\t\t\t%02d ", (m_Context.EFlags & EFLAG_OF) ? 1 : 0);
    printf("%02d ", (m_Context.EFlags & EFLAG_IF) ? 1 : 0);
    printf("%02d ", (m_Context.EFlags & EFLAG_TF) ? 1 : 0);
    printf("%02d ", (m_Context.EFlags & EFLAG_SF) ? 1 : 0);
    printf("%02d ", (m_Context.EFlags & EFLAG_ZF) ? 1 : 0);
    printf("%02d ", (m_Context.EFlags & EFLAG_AF) ? 1 : 0);
    printf("%02d ", (m_Context.EFlags & EFLAG_PF) ? 1 : 0);
    printf("%02d \r\n", (m_Context.EFlags & EFLAG_CF) ? 1 : 0);

}


void MyDebuger::ShowMemData(DWORD ShowAddressData)
{
    char szHexData[256] = { 0 };
    static unsigned int sOldEip = 0;
    //��¼��ǰ�ĵ�ַ ��ǰEIP�Ͳ鿴���ڴ��ַͬ��
    if (ShowAddressData == 0)
    {
        if (m_CurCodeAddress == 0)
        {
            sOldEip = m_CurCodeMemAddress = m_CurCode.pStartAddress;
        }
        if (sOldEip != m_Context.Eip)
        {
            m_CurCodeMemAddress = m_CurCode.pCurCodeEntryAddress;
            sOldEip = m_Context.Eip;
        }

    }
    else
        m_CurCodeMemAddress = ShowAddressData;


    ReadProcessMemory(m_hProcess, (LPVOID)m_CurCodeMemAddress, szHexData, sizeof(szHexData), NULL);

    for (int i = 0; i < 8; i++)
    {
        printf("%p  ", m_CurCodeMemAddress);
        for (int j = 0; j < 16; j++)
        {
            if (j == 7)
            {
                printf("%02X", (unsigned char)szHexData[i * 16 + j]);
                printf("-");
            }
            else
                printf("%02X ", (unsigned char)szHexData[i * 16 + j]);
        }
        for (int k = 0; k < 16; k++)
        {
            //����3�������ַ�(0A 0D 09)��Ӱ�������ʾ���Ч��
            if ((unsigned char)szHexData[i * 16 + k] == NULL ||
                (unsigned char)szHexData[i * 16 + k] == 0xFF ||
                (unsigned char)szHexData[i * 16 + k] == 0x0A ||  //0A ASC��Ϊ����
                (unsigned char)szHexData[i * 16 + k] == 0x0D ||  //0D ASC��Ϊ�س�
                (unsigned char)szHexData[i * 16 + k] == 0x09 ||  //09 ASC��ΪTAB��
                (unsigned char)szHexData[i * 16 + k] == 0x07)    //07 ASC������
            {
                printf(".");
            }
            else
                printf("%c", (unsigned char)szHexData[i * 16 + k]);
        }
        printf("\r\n");
        m_CurCodeMemAddress += 16;
    }
    if (FALSE == bSelIptFlag)
    {
        InputCMD();
    }
}

//�� �� ����HexToAsc()
//������������16����ת��ΪASCII
unsigned char MyDebuger::HexToAsc(unsigned char aHex)
{

    if ((aHex >= 0) && (aHex <= 9))

        aHex += 0x30;

    else if ((aHex >= 10) && (aHex <= 15))//A-F

        aHex += 0x37;

    else return 0;

    return aHex;
}

//�� �� ����AscToHex()
//������������ASCIIת��Ϊ16����
unsigned char MyDebuger::AscToHex(unsigned char aChar)
{

    if ((aChar >= 0x30) && (aChar <= 0x39))

        aChar -= 0x30;

    else if ((aChar >= 'A') && (aChar <= 'F'))//��д��ĸ

        aChar -= 0x37;

    else if ((aChar >= 'a') && (aChar <= 'f'))//Сд��ĸ

        aChar -= 0x57;

    else return 0;

    return aChar;
}

char MyDebuger::ConvertCode(char& code1, char code2)
{
    tagChar ch;
    //char ch = 0;

    ch.m_b = AscToHex(code1);
    ch.m_a = AscToHex(code2);

    return *(char *)&ch;
}

void MyDebuger::EditMem(DWORD dwAddr)
{
    UINT nCurrent = 0;

    // �ӱ����Խ��̻�ȡ������
    DWORD dwBytesRead = 0;
    tagChar chCodeBuff = { 0 };
    ReadProcessMemory(m_hProcess, (LPVOID)dwAddr, (LPVOID)&chCodeBuff, 1, &dwBytesRead);
    if (dwBytesRead < 1)
    {
        return;
    }
    // ����������ʾ����Ļ��
    printf("%c%c.", HexToAsc(chCodeBuff.m_b), HexToAsc(chCodeBuff.m_a));


    // ���û���ȡ�µ�ָ��
    char chInputCode = 0;
    char szInputBuff[4] = { 0 };
    int nCount = 0;
    while (chInputCode != '\n' && chInputCode != ' ' && nCount != 2)   /*ÿ��getchar()���ζ���һ���ַ�*/
    {
        chInputCode = _getche();       /*�ȴ�����һ��*/
        szInputBuff[nCount] = chInputCode;
        nCount++;
    }

    // convert input buff to binary
    char chConvrtedCode = 0;
    chConvrtedCode = ConvertCode(szInputBuff[0], szInputBuff[1]);

    // write memory
    DWORD dwWritten = 0;
    WriteProcessMemory(m_hProcess, (LPVOID)dwAddr, &chConvrtedCode, 1, &dwWritten);
    printf("\r\n");

}


void MyDebuger::ShowHelp()
{
    printf("--------------------------------------------------------------------------\r\n");
    printf("1:��������  t  2:��������  p  3:����  g  4:�����  u  5:��ʾ�ڴ�����  d      \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("6:�Ĵ���  r  7:�޸��ڴ�����  e  8:һ��ϵ�  bp  9:һ��ϵ��б�  bpl          \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("10:ɾ��һ��ϵ�  bpc  11:Ӳ��ִ�жϵ�	 bhe  12:Ӳ��д��ϵ�	  bhw             \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("13:Ӳ�����ʶϵ�  bha  14:Ӳ���ϵ��б�  bhl 15:ɾ��Ӳ���ϵ�	bhc               \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("16:�ڴ���ϵ�  bmr  17:�ڴ�д�ϵ�  bmw  18:�ڴ�ϵ��б�	 bml              \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("19:��ҳ�ϵ��б�  bmpl  20:ɾ���ڴ�ϵ�  bmc  21:����ű�  ls                \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("22:�����ű�  es  23:�˳�����  q  24:�鿴ģ��  ml  25:����  cls              \r\n");
    printf("--------------------------------------------------------------------------\r\n");
    printf("26:�Զ�����  trace  27 : ����  help                                        \r\n");
    printf("--------------------------------------------------------------------------\r\n");
}