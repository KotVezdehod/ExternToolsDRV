﻿#include <chrono>
#include <codecvt>
#include <thread>
#include <string>
#include <locale>
//#include <stdio.h>
//#include <wchar.h>
//#include "pch.h"
#include "AddInNative.h"

#define TIME_LEN 65

#define BASE_ERRNO     7

#ifdef WIN32
#pragma setlocale("ru-RU" )
#endif

using std::this_thread::sleep_for;

static const wchar_t g_kClassNames[] = L"CAddInNative"; //"|OtherClass1|OtherClass2";
static IAddInDefBase *pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);

static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;
static WcharWrapper s_names(g_kClassNames);
//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new CAddInNative;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
    g_capabilities = capabilities;
    return eAppCapabilitiesLast;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
    if(!*pIntf)
        return -1;

    delete *pIntf;
    *pIntf = 0;
    return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    return s_names;
}
//---------------------------------------------------------------------------//
#if !defined( __linux__ ) && !defined(__APPLE__)
VOID CALLBACK MyTimerProc(PVOID lpParam, BOOLEAN TimerOrWaitFired);
#else
static void MyTimerProc(int sig);
#endif //__linux__

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
    m_iMemory = 0;
    m_iConnect = 0;
#if !defined( __linux__ ) && !defined(__APPLE__)
    m_hTimerQueue = 0;
#endif //__linux__
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
#if !defined( __linux__ ) && !defined(__APPLE__)
    if(m_hTimerQueue )
    {
        DeleteTimerQueue(m_hTimerQueue);
        m_hTimerQueue = 0;
    }
#endif //__linux__
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
    const wchar_t* wsExtension = L"ExternToolsDRV";
    uint32_t iActualSize = static_cast<uint32_t>(::wcslen(wsExtension) + 1);

    if (m_iMemory)
    {
        if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
        {
            convToShortWchar(wsExtensionName, wsExtension, iActualSize);
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
    long plPropNum = -1;
    wchar_t* propName = 0;

    ::convFromShortWchar(&propName, wsPropName);
    plPropNum = findName(g_PropNames, propName, ePropLast);

    if (plPropNum == -1)
        plPropNum = findName(g_PropNamesRu, propName, ePropLast);

    delete[] propName;

    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
    if (lPropNum >= ePropLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    size_t iActualSize = 0;

    switch(lPropAlias)
    {
    case 0: // First language
        wsCurrentName = (wchar_t*)g_PropNames[lPropNum];
        break;
    case 1: // Second language
        wsCurrentName = (wchar_t*)g_PropNamesRu[lPropNum];
        break;
    default:
        return 0;
    }
    
    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
    switch(lPropNum)
    {
    /*case ePropIsEnabled:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = m_boolEnabled;
        break;
    case ePropIsTimerPresent:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = true;
        break;*/
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
    switch(lPropNum)
    { 
    /*case ePropIsEnabled:
        if (TV_VT(varPropVal) != VTYPE_BOOL)
            return false;
        m_boolEnabled = TV_BOOL(varPropVal);
        break;
    case ePropIsTimerPresent:*/
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
    switch(lPropNum)
    { 
    /*case ePropIsEnabled:
    case ePropIsTimerPresent:
        return true;*/
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
    switch(lPropNum)
    { 
   /* case ePropIsEnabled:
        return true;
    case ePropIsTimerPresent:
        return false;*/
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
    return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{ 
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    delete[] name;

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 
    if (lMethodNum >= eMethLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    size_t iActualSize = 0;

    switch(lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = (wchar_t*)g_MethodNames[lMethodNum];
        break;
    case 1: // Second language
        wsCurrentName = (wchar_t*)g_MethodNamesRu[lMethodNum];
        break;
    default: 
        return 0;
    }

    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethShowInStatusLine:
        return 1;
    case eMethLoadPicture:
        return 1;
    case eLoopback:
        return 1;
    case eMethDelay:
        return 1;
    case eMethJvdValidate:
        return 2;
    default:
        return 0;
    }
    
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                        tVariant *pvarParamDefValue)
{ 
    TV_VT(pvarParamDefValue)= VTYPE_EMPTY;

    switch(lMethodNum)
    { 
    case eMethEnable:
    case eMethDisable:
    case eMethShowInStatusLine:
    case eMethStartTimer:
    case eMethStopTimer:
    case eMethShowMsgBox:
        // There are no parameter values by default 
        break;
    default:
        return false;
    }

    return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethLoadPicture:
    case eLoopback:
    case eMethJvdValidate:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{ 
    switch(lMethodNum)
    { 
    case eMethEnable:
        m_boolEnabled = true;
        break;
    case eMethDisable:
        m_boolEnabled = false;
        break;
    case eMethShowInStatusLine:
        if (m_iConnect && lSizeArray)
        {
            tVariant *var = paParams;
            m_iConnect->SetStatusLine(var->pwstrVal);
        }
        break;
    case eMethStartTimer:
        pAsyncEvent = m_iConnect;
        /* The task of length of turn of messages
        if (m_iConnect)
            m_iConnect->SetEventBufferDepth(4000);
        */
#if !defined( __linux__ ) && !defined(__APPLE__)
        m_hTimerQueue = CreateTimerQueue();
        CreateTimerQueueTimer( &m_hTimer, m_hTimerQueue, 
            (WAITORTIMERCALLBACK)MyTimerProc, 0, 1000, 1000, 0);
#else
        struct sigaction sa;
        struct itimerval tv;
        memset(&tv, 0, sizeof(tv));

        sa.sa_handler = MyTimerProc;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; 
        sigaction(SIGALRM, &sa, NULL);
        tv.it_interval.tv_sec = 1;
        tv.it_value.tv_sec = 1;
        setitimer(ITIMER_REAL, &tv, NULL);
#endif
        break;
    case eMethStopTimer:
#if !defined( __linux__ ) && !defined(__APPLE__)
        if (m_hTimer != 0)
        {
            DeleteTimerQueue(m_hTimerQueue);
            m_hTimerQueue = 0;
            m_hTimer = 0;
        }
#else
        alarm(0);
#endif
        m_uiTimer = 0;
        pAsyncEvent = NULL;
        break;
    case eMethShowMsgBox:
        {
            if(eAppCapabilities1 <= g_capabilities)
            {
                IAddInDefBaseEx* cnn = (IAddInDefBaseEx*)m_iConnect;
                IMsgBox* imsgbox = (IMsgBox*)cnn->GetInterface(eIMsgBox);
                if (imsgbox)
                {
                    IPlatformInfo* info = (IPlatformInfo*)cnn->GetInterface(eIPlatformInfo);
                    assert(info);
                    const IPlatformInfo::AppInfo* plt = info->GetPlatformInfo();
                    if (!plt)
                        break;
                    tVariant retVal;
                    tVarInit(&retVal);
                    if(imsgbox->Confirm(plt->AppVersion, &retVal))
                    {
                        bool succeed = TV_BOOL(&retVal);
                        WCHAR_T* result = 0;
                        
                        if (succeed)
                            ::convToShortWchar(&result, L"OK");
                        else
                            ::convToShortWchar(&result, L"Cancel");
                        
                        imsgbox->Alert(result);
                        delete[] result;
                        
                    }
                }
            }
        }
        break;

    case eMethDelay:
    {
        if (!lSizeArray || !paParams)
            return false;
        
        sleep_for(std::chrono::milliseconds(paParams->uintVal));

    }
    return true;

    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
    bool ret = false;
    FILE *file = 0;
    char *name = 0;
    size_t size = 0;
    char *mbstr = 0;
    wchar_t* wsTmp = 0;
    char* loc = 0;

    switch(lMethodNum)
    {
        // Method acceps one argument of type BinaryData ant returns its copy
        case eLoopback:
        {
            if (lSizeArray != 1 || !paParams)
                return false;

            if (TV_VT(paParams) != VTYPE_BLOB)
            {
                addError(ADDIN_E_VERY_IMPORTANT, L"AddInNative", L"Parameter type mismatch.", -1);
                return false;
            }

            if (paParams->strLen > 0)
            {
                m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, paParams->strLen);
                memcpy((void*)pvarRetValue->pstrVal, (void*)paParams->pstrVal, paParams->strLen);
            }

            TV_VT(pvarRetValue) = VTYPE_BLOB;
            pvarRetValue->strLen = paParams->strLen;
            return true;
        }
        break;

    case eMethLoadPicture:
        {
            if (!lSizeArray || !paParams)
                return false;
            
            switch(TV_VT(paParams))
            {
            case VTYPE_PSTR:
                name = paParams->pstrVal;
                break;
            case VTYPE_PWSTR:
                loc = setlocale(LC_ALL, "");
                ::convFromShortWchar(&wsTmp, TV_WSTR(paParams));
#pragma warning(push)
#pragma warning(disable: 4996)
                size = wcstombs(0, wsTmp, 0)+1;
#pragma warning(pop)
                assert(size);
                mbstr = new char[size];
                assert(mbstr);
                memset(mbstr, 0, size);
#pragma warning(push)
#pragma warning(disable: 4996)
                size = wcstombs(mbstr, wsTmp, getLenShortWcharStr(TV_WSTR(paParams)));
#pragma warning(pop)
                name = mbstr;
                setlocale(LC_ALL, loc);
                delete[] wsTmp;
                break;
            default:
                return false;
            }
        }
                
#pragma warning(push)
#pragma warning(disable: 4996)
        file = fopen(name, "rb");
#pragma warning(pop)


        if (file == 0)
        {
            wchar_t* wsMsgBuf;
            uint32_t err = errno;
#pragma warning(push)
#pragma warning(disable: 4996)
            name = strerror(err);
            size_t sizeloc = mbstowcs(0, name, 0) + 1;
#pragma warning(pop)
            assert(sizeloc);
            wsMsgBuf = new wchar_t[sizeloc];
            assert(wsMsgBuf);
            memset(wsMsgBuf, 0, sizeloc * sizeof(wchar_t));
#pragma warning(push)
#pragma warning(disable: 4996)
            sizeloc = mbstowcs(wsMsgBuf, name, sizeloc);
#pragma warning(pop)

            addError(ADDIN_E_VERY_IMPORTANT, L"AddInNative", wsMsgBuf, RESULT_FROM_ERRNO(err));
            delete[] wsMsgBuf;
            return false;
        }

        fseek(file, 0, SEEK_END);
        size = ftell(file);
        
        if (size && m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, (unsigned)size))
        {
            fseek(file, 0, SEEK_SET);
            size = fread(pvarRetValue->pstrVal, 1, size, file);
            pvarRetValue->strLen = (unsigned)size;
            TV_VT(pvarRetValue) = VTYPE_BLOB;
            
            ret = true;
        }
        if (file)
            fclose(file);

        if (mbstr && size != -1)
            delete[] mbstr;

        break;

    /*case eMethJvdValidate:
    {
        ValiJson jvd(m_iMemory);
        jvd.validateJsonByScheme(paParams, pvarRetValue);
    }
    return true;*/

    }

    return ret; 


}
//---------------------------------------------------------------------------//
// This code will work only on the client!
#if !defined( __linux__ ) && !defined(__APPLE__)
VOID CALLBACK MyTimerProc(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (!pAsyncEvent)
        return;
    DWORD dwTime = 0;
    
    const wchar_t *who = L"ComponentNative", *what =  L"Timer";

    wchar_t *wstime = new wchar_t[TIME_LEN];
    if (wstime)
    {
        wmemset(wstime, 0, TIME_LEN);
        time_t vtime;
        time(&vtime);
        ::_ui64tow_s(vtime, wstime, TIME_LEN, 10);


        pAsyncEvent->ExternalEvent((wchar_t*)who, (wchar_t*)what, wstime);          //приведение типов мона использовать, т.к. не будет записи в строки.
        delete[] wstime;
    }
}
#else
void MyTimerProc(int sig)
{
    if (!pAsyncEvent)
        return;

    WCHAR_T *who = 0, *what = 0, *wdata = 0;
    wchar_t *data = 0;
    time_t dwTime = time(NULL);

    data = new wchar_t[TIME_LEN];
    
    if (data)
    {
        wmemset(data, 0, TIME_LEN);
        swprintf(data, TIME_LEN, L"%ul", dwTime);
        ::convToShortWchar(&who, L"ComponentNative");
        ::convToShortWchar(&what, L"Timer");
        ::convToShortWchar(&wdata, data);

        pAsyncEvent->ExternalEvent(who, what, wdata);

        delete[] who;
        delete[] what;
        delete[] wdata;
        delete[] data;
    }
}
#endif
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
#if !defined( __linux__ ) && !defined(__APPLE__)
    _wsetlocale(LC_ALL, loc);
#else
    //We convert in char* char_locale
    //also we establish locale
    //setlocale(LC_ALL, char_locale);
#endif
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNative::addError(uint32_t wcode, const wchar_t* source, 
                        const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(const wchar_t* names[], const wchar_t* name, 
                        const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}
//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len)
{
    if (!len)
        len = ::wcslen(Source) + 1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len * sizeof(WCHAR_T));
#ifdef __linux__
    size_t succeed = (size_t)-1;
    size_t f = len * sizeof(wchar_t), t = len * sizeof(WCHAR_T);
    const char* fromCode = sizeof(wchar_t) == 2 ? "UTF-16" : "UTF-32";
    iconv_t cd = iconv_open("UTF-16LE", fromCode);
    if (cd != (iconv_t)-1)
    {
        succeed = iconv(cd, (char**)&tmpWChar, &f, (char**)&tmpShort, &t);
        iconv_close(cd);
        if(succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpShort = (WCHAR_T)*tmpWChar;
    }

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source) + 1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len * sizeof(wchar_t));
#ifdef __linux__
    size_t succeed = (size_t)-1;
    const char* fromCode = sizeof(wchar_t) == 2 ? "UTF-16" : "UTF-32";
    size_t f = len * sizeof(WCHAR_T), t = len * sizeof(wchar_t);
    iconv_t cd = iconv_open("UTF-32LE", fromCode);
    if (cd != (iconv_t)-1)
    {
        succeed = iconv(cd, (char**)&tmpShort, &f, (char**)&tmpWChar, &t);
        iconv_close(cd);
        if(succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpWChar = (wchar_t)*tmpShort;
    }

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//

#ifdef LINUX_OR_MACOS
WcharWrapper::WcharWrapper(const WCHAR_T* str) : m_str_WCHAR(NULL),
                           m_str_wchar(NULL)
{
    if (str)
    {
        int len = getLenShortWcharStr(str);
        m_str_WCHAR = new WCHAR_T[len + 1];
        memset(m_str_WCHAR,   0, sizeof(WCHAR_T) * (len + 1));
        memcpy(m_str_WCHAR, str, sizeof(WCHAR_T) * len);
        ::convFromShortWchar(&m_str_wchar, m_str_WCHAR);
    }
}
#endif
//---------------------------------------------------------------------------//
WcharWrapper::WcharWrapper(const wchar_t* str) :
#ifdef LINUX_OR_MACOS
    m_str_WCHAR(NULL),
#endif 
    m_str_wchar(NULL)
{
    if (str)
    {
        size_t len = wcslen(str);
        m_str_wchar = new wchar_t[len + 1];
        memset(m_str_wchar, 0, sizeof(wchar_t) * (len + 1));
        memcpy(m_str_wchar, str, sizeof(wchar_t) * len);
#ifdef LINUX_OR_MACOS
        ::convToShortWchar(&m_str_WCHAR, m_str_wchar);
#endif
    }

}
//---------------------------------------------------------------------------//
WcharWrapper::~WcharWrapper()
{
#ifdef LINUX_OR_MACOS
    if (m_str_WCHAR)
    {
        delete [] m_str_WCHAR;
        m_str_WCHAR = NULL;
    }
#endif
    if (m_str_wchar)
    {
        delete [] m_str_wchar;
        m_str_wchar = NULL;
    }
}
//---------------------------------------------------------------------------//

