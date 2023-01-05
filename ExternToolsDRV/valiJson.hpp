#pragma once

#include <set>
//#include <fstream>

#include "../valijson/adapters/jsoncpp_adapter.hpp"
#include "../valijson/schema_parser.hpp"
#include "../valijson/validator.hpp"
#include "../valijson/validation_results.hpp"
#include "Json.h"
#include "StringConverters.h"

//#include "AddInNative.h"

class ValiJson
{

public:

	void validateJsonByScheme(tVariant* paParams, tVariant* pvarRetValue)
	{

		if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
		{
			sconv.DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр - схема (строка)");
			return;
		}

		if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
		{
			sconv.DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - сам json (строка)");
			return;
		}

		wchar_t* wchChema = nullptr;
		sconv.convFromShortWchar(&wchChema, (&paParams[0])->pwstrVal);

		if (wchChema)
		{
			wchar_t* wchData = nullptr;
			sconv.convFromShortWchar(&wchData, (&paParams[1])->pwstrVal);

			if (wchData)
			{
				size_t chemaSz = (wcslen(wchChema) + 1) * sizeof(wchar_t);
				char* chChema = nullptr;
				chChema = new char[chemaSz];

				if (chChema)
				{
					memset(chChema, 0, chemaSz);
					wcstombs(chChema, wchChema, chemaSz);

					size_t dataSz = (wcslen(wchData) + 1) * sizeof(wchar_t);

					char* chData = nullptr;
					chData = new char[dataSz];

					if (chData)
					{
						memset(chData, 0, dataSz);
						wcstombs(chData, wchData, dataSz);

						Json::Value root;

						Json::Value details = Json::arrayValue;
						details.clear();

						std::string sChema = sconv.utf8_encode(std::wstring(reinterpret_cast<wchar_t*>(chChema)));		//из 1с приезжает wchar_t, а либы работают с multibyte
						std::string sData = sconv.utf8_encode(std::wstring(reinterpret_cast<wchar_t*>(chData)));

						root["Status"] = validate(sChema.c_str(), strlen(sChema.c_str()), sData.c_str(), strlen(sData.c_str()), &details);
						root["Description"] = details;
						root["Data"] = "";

						Json::FastWriter fw;
						std::string s_res = fw.write(root);

						sconv.ToV8StringFromChar(s_res.c_str(), pvarRetValue, iMemoryManager);

						delete[] chData;
					}


					delete[] chChema;
				}
				else
				{
					sconv.DiagToV8String(pvarRetValue, iMemoryManager, false, L"не удалось выделить память под строку схемы.");
				}

				delete[] wchData;
			}
			else
			{
				sconv.DiagToV8String(pvarRetValue, iMemoryManager, false, L"не удалось выделить память под строку json.");
			}
			delete[] wchChema;
		}
		else
		{
			sconv.DiagToV8String(pvarRetValue, iMemoryManager, false, L"не удалось выделить память под строку схемы.");
		}

		return;
	};

	ValiJson(IMemoryManager* iMemoryManagerIn, IAddInDefBase* m_iConnectIn) :
		iMemoryManager(iMemoryManagerIn), m_iConnect(m_iConnectIn) {};

	~ValiJson() {};


private:

	IMemoryManager* iMemoryManager = nullptr;
	IAddInDefBase* m_iConnect;
	StringConverters sconv;

	//bool validate(char* chemaIn, size_t chemaSz, char* dataIn, size_t dataSz, Json::Value* outDiag)
	bool validate(const char* chemaIn, size_t chemaSz, const char* dataIn, size_t dataSz, Json::Value* outDiag)
	{
		Json::Value rootSchema;
		Json::CharReaderBuilder builder;
		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

		std::string err;

		try
		{
			if (reader->parse(chemaIn, chemaIn + chemaSz, &rootSchema, &err))
			{

				valijson::Schema schema;
				valijson::SchemaParser parser;
				valijson::adapters::JsonCppAdapter mySchemaAdapter(rootSchema);

				parser.populateSchema(mySchemaAdapter, schema);
				
				Json::Value rootData;

				try
				{
					if (reader->parse(dataIn, dataIn + dataSz, &rootData, &err))
					{

						valijson::Validator validator;
						valijson::adapters::JsonCppAdapter dataAdapter(rootData);
						valijson::ValidationResults res;

						if (!validator.validate(schema, dataAdapter, &res))
						{
							buildFullErrorMessage(&res, outDiag);
							return false;
						}

						outDiag->append(std::string("OK"));
						return true;
					}
					else
					{
						outDiag->append(std::string("Ошибка при разборе JSON: "));
						outDiag->append(err);
						return false;
					}
				}
				catch (const std::exception& ex)
				{
					outDiag->append(std::string("Исключение при разборе схемы: ") + ex.what());
					outDiag->append(err);
					return false;
				}


			}
			else
			{
				outDiag->append(std::string("Ошибка при разборе схемы: "));
				outDiag->append(err);
				return false;
			}
		}
		catch (const std::exception& ex)
		{
			outDiag->append(std::string("Исключение при разборе схемы: ") + ex.what());
			outDiag->append(err);
			return false;
		}

		return false;
	};

	void buildFullErrorMessage(valijson::ValidationResults* diagObject, Json::Value* outDiag)
	{
		std::set<std::string> setDiag;

		size_t lastLen = 0;
		for (const valijson::ValidationResults::Error& it : *diagObject)
		{
			if (it.context.size() >= lastLen)
			{
				std::string currDiag = it.description + std::string(" Поле: ");

				bool firstLoc = true;
				for (const std::string& strErr : it.context)
				{

					if (!firstLoc)
						currDiag += " --> ";

					currDiag += strErr;

					firstLoc = false;
				};


				setDiag.insert(currDiag);
			};
			lastLen = it.context.size();
		}


		for (const std::string& strLoc : setDiag)
		{

			outDiag->append(strLoc.c_str());

		}
		return;
	};


	/*void Alert(const wchar_t* msgIn)
	{
		if (m_iConnect)
		{
			WCHAR_T* msg = 0;
			sconv.convToShortWchar(&msg, msgIn);
			IAddInDefBaseEx* cnn = (IAddInDefBaseEx*)m_iConnect;
			IMsgBox* imsgbox = (IMsgBox*)cnn->GetInterface(eIMsgBox);
			imsgbox->Alert(msg);
			delete[] msg;
		}
	}*/

	//void flushToFile(std::string sIn)
	//{
	//	std::ofstream ofs("C:\\C++\\JsonValidator\\JsonValidator\\1.txt", std::ios_base::binary);
	//	ofs.write(sIn.c_str(), sIn.length());
	//	ofs.flush();
	//	ofs.close();

	//	return;
	//}

};

