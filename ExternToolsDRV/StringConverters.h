#pragma once
#include "..\include\AddInDefBase.h"
#include "..\include\IMemoryManager.h"

class StringConverters
{

public:
	StringConverters() {};
	~StringConverters() {};
		
	std::string utf8_encode(const std::wstring& wstr);
	std::wstring utf8_decode(const std::string& str);

	void DiagToV8String(tVariant* pvarRetValue, IMemoryManager* m_iMemory, bool status, const wchar_t* wch_description);
	bool DiagStructure(bool status, const wchar_t* wch_description, const wchar_t* wch_data, wchar_t** out_str);
	void ToV8String(const wchar_t* wstr, tVariant* par, IMemoryManager* m_iMemory);
	void ToV8StringFromChar(const char* str, tVariant* par, IMemoryManager* m_iMemory);

	uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len = 0);
	uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
	uint32_t getLenShortWcharStr(const WCHAR_T* Source);
};

