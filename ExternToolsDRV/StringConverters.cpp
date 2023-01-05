#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <string>
#include "StringConverters.h"
#include "Json.h"

// Convert a wide Unicode string to an UTF8 string
std::string StringConverters::utf8_encode(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring StringConverters::utf8_decode(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void StringConverters::DiagToV8String(tVariant* pvarRetValue, IMemoryManager* m_iMemory, bool status, const wchar_t* wch_description)
{
    wchar_t* wch_err = nullptr;
    if (DiagStructure(status, wch_description, L"", &wch_err))
    {
        ToV8String(wch_err, pvarRetValue, m_iMemory);
        delete[] wch_err;
    }

}

bool StringConverters::DiagStructure(bool status, const wchar_t* wch_description, const wchar_t* wch_data, wchar_t** out_str)
{
    std::wstring ws_dwscription = std::wstring(wch_description);
    std::wstring ws_data = std::wstring(wch_data);

    //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    //std::string s_description = converter.to_bytes(ws_dwscription);
    std::string s_description = utf8_encode(ws_dwscription);
    //std::string s_data = converter.to_bytes(ws_data);
    std::string s_data = utf8_encode(ws_data);

    Json::Value root;
    root["Status"] = status;
    root["Description"] = s_description.c_str();
    root["Data"] = s_data.c_str();

    Json::StreamWriterBuilder builder;
    std::string s_res = Json::writeString(builder, root);
    //std::wstring wstr = converter.from_bytes(s_res);
    std::wstring wstr = utf8_decode(s_res);

    if (!*out_str)
    {
        *out_str = new wchar_t[(wstr.length() + 1) * sizeof(wchar_t)];
    }
    else
    {
        return false;
    }

    wcscpy(*out_str, wstr.c_str());
    return true;
}

void StringConverters::ToV8String(const wchar_t* wstr, tVariant* par, IMemoryManager* m_iMemory)
{
    if (wstr)
    {
        ULONG len = wcslen(wstr);
        m_iMemory->AllocMemory((void**)&par->pwstrVal, (len + 1) * sizeof(WCHAR_T));
        convToShortWchar(&par->pwstrVal, wstr);
        par->vt = VTYPE_PWSTR;
        par->wstrLen = len;
    }
    else
        par->vt = VTYPE_EMPTY;
}

void StringConverters::ToV8StringFromChar(const char* str, tVariant* par, IMemoryManager* m_iMemory)
{
    if (str)
    {
        
        std::wstring wstr = utf8_decode(std::string(str));

        ULONG len = wcslen(wstr.c_str());
        m_iMemory->AllocMemory((void**)&par->pwstrVal, (len + 1) * sizeof(WCHAR_T));
        convToShortWchar(&par->pwstrVal, wstr.c_str());
        par->vt = VTYPE_PWSTR;
        par->wstrLen = len;
    }
    else
        par->vt = VTYPE_EMPTY;
}

uint32_t StringConverters::convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len)
{
    if (!len)
        len = ::wcslen(Source) + 1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*)Source;
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
        if (succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpShort = (WCHAR_T)*tmpWChar;
    }

    return res;
}

uint32_t StringConverters::convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
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
        if (succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpWChar = (wchar_t)*tmpShort;
    }

    return res;
}

uint32_t StringConverters::getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
