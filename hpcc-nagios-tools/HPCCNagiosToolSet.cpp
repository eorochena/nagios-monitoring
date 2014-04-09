#include <cstdio>
#include "jstream.ipp"
#include "jfile.hpp"
#include "jbuff.hpp"
#include "HPCCNagiosToolSet.hpp"
#include "HPCCNagiosToolSetCommon.hpp"
#include <cstring>

bool CHPCCNagiosToolSet::generateHostGroupFile(const char* pOutputFilePath, const char* pConfigGenPath)
{

}

bool CHPCCNagiosToolSet::generateServiceDefinitionFile(const char* pOutputFilePath, const char* pEnvXML, const char* pConfigGenPath)
{
    if (pOutputFilePath == NULL || *pOutputFilePath == 0 || pConfigGenPath == NULL || *pConfigGenPath == 0 || checkFileExists(pConfigGenPath) == false)
    {
        return false;
    }

    MemoryBuffer memBuff;
    StringBuffer strConfiggenCmdLine(pConfigGenPath);

    strConfiggenCmdLine.append(PCONFIGGEN_PARAM_LIST_ALL).append(PCONFIGGEN_PARAM_ENVIRONMENT).append(pEnvXML);

    FILE *fp = popen(strConfiggenCmdLine.str(), "r");

    if (fp == NULL)
    {
        return false;
    }

    int nCharacter = -1;
    CFileInputStream cfgInputStream(fileno(fp));

    memBuff.clear();

    do
    {
        nCharacter = cfgInputStream.readNext();

        memBuff.append(static_cast<unsigned char>(nCharacter));
    }
    while(nCharacter != -1);

    memBuff.append('\0');

    OwnedIFile outputFile = createIFile(pOutputFilePath);
    OwnedIFileIO io = outputFile->open(IFOcreaterw);

    if (io == NULL)
    {
        return false;
    }

    StringBuffer strOutput(memBuff.toByteArray());
    strOutput.replaceString(",,",",X,"); // sttrok pecularity with adjacent delimiters
    strOutput.replaceString(",\n",",X\n"); // sttrok pecularity with adjacent delimiters
    char *pOutput = strdup(strOutput.str());

    int nCount = 0;
    char pProcess[1024] = "";

    char *pch = NULL;
    pch = strtok(pOutput, ",\n");

    StringBuffer strServiceConfig;

    CHPCCNagiosToolSet::generateNagiosHostConfig(strServiceConfig, pEnvXML);
#ifdef _USE_OTHER_STUFF_
    int i = -1;
    while (pch != NULL)
    {
        if (nCount % 6 ==  0) // Process name
        {
            if (pProcess != NULL && *pProcess != 0 && strcmp(pProcess, pch) != 0)
            {
                strcpy(pProcess, pch);
                i = 0;
            }
            else if (pProcess == NULL || *pProcess == 0 || strcmp(pProcess, pch) != 0)
            {
                strncpy(pProcess, pch, sizeof(pProcess));
                i++;
            }
            else if (strcmp(pProcess,pch) == 0)
            {
                i++;
            }
        }
        else if (nCount % 6 == 2) // IP address
        {
            strServiceConfig.append(P_NAGIOS_HOST_CONFIG_1).append(pProcess).append(i).append(P_NAGIOS_HOST_CONFIG_2).append(pProcess).append(" ")\
                                                                    .append(i).append(P_NAGIOS_HOST_CONFIG_3).append(pch).append(P_NAGIOS_HOST_CONFIG_4);
            strServiceConfig.append("\n");
        }

        pch = strtok(NULL, ",\n");

        nCount++;
    }
#endif // _USE_OTHER_STUFF_
    io->write(0, strServiceConfig.length(), strServiceConfig.str());
    io->close();

    delete pOutput;
}

bool CHPCCNagiosToolSet::generateServiceDefinitionFile(StringBuffer &strOutput, const char* pEnvXML, const char* pConfigGenPath)
{
    return true;
}

bool CHPCCNagiosToolSet::generateHostGroupFile(StringBuffer &strOutput, const char* pConfigGenPath)
{
    return true;
}

void CHPCCNagiosToolSet::createHostGroupString(StringArray &pIP, StringBuffer &strHostGroup)
{

}

bool CHPCCNagiosToolSet::generateNagiosHostConfig(StringBuffer &strHostConfig, const char* pEnvXML, const char* pConfigGenPath)
{
    if (pConfigGenPath == NULL || *pConfigGenPath == 0 || checkFileExists(pConfigGenPath) == false)
    {
        return false;
    }

    MemoryBuffer memBuff;
    StringBuffer strConfiggenCmdLine(pConfigGenPath);

    strConfiggenCmdLine.append(PCONFIGGEN_PARAM_MACHINES).append(PCONFIGGEN_PARAM_ENVIRONMENT).append(pEnvXML);

    FILE *fp = popen(strConfiggenCmdLine.str(), "r");

    if (fp == NULL)
    {
        return false;
    }

    int nCharacter = -1;
    CFileInputStream cfgInputStream(fileno(fp));

    memBuff.clear();

    do
    {
        nCharacter = cfgInputStream.readNext();

        memBuff.append(static_cast<unsigned char>(nCharacter));
    }
    while(nCharacter != -1);

    memBuff.append('\0');

    StringBuffer strOutput(memBuff.toByteArray());
    strOutput.replaceString(",,",",X,"); // sttrok pecularity with adjacent delimiters
    strOutput.replaceString(",\n",",X\n"); // sttrok pecularity with adjacent delimiters
    strOutput.replace('\377',',');

    char *pOutput = strdup(strOutput.str());

    int nCount = 0;
    char pIP[1024] = "";

    char *pch = NULL;
    pch = strtok(pOutput, ",\n");

    int i = 0;
    while (pch != NULL)
    {
        if (nCount % 2 ==  0) // Process name
        {
            char pHostName[1024] = "";
            struct hostent* hp = NULL;

            static bool bDoLookUp = true;

            if (bDoLookUp == true)
            {
                unsigned int addr = inet_addr(pch);
                hp = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
            }

            if (hp == NULL)
            {
                bDoLookUp = false;
                strcpy(pHostName, "Server");
            }
            else
            {
                strcpy(pHostName,hp->h_name);
            }

            strHostConfig.append(P_NAGIOS_HOST_CONFIG_1).append(pHostName).append(i).append(P_NAGIOS_HOST_CONFIG_2).append(pHostName).append(" ")\
                                                                    .append(i).append(P_NAGIOS_HOST_CONFIG_3).append(pch).append(P_NAGIOS_HOST_CONFIG_4);
            strHostConfig.append("\n");

            i++;
        }

        pch = strtok(NULL, ",\n");

        nCount++;
    }

    return true;
}
