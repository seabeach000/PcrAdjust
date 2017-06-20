// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "PcrAdjustFilter.h"
#include <InitGuid.h>
// Using this pointer in constructor
#pragma warning(disable:4355 4127)

// {06402A72-0DA7-4258-9DAC-497F505F3D87}
DEFINE_GUID(CLSID_ZQVIDEOPcrAdjust,
	0x6402a72, 0xda7, 0x4258, 0x9d, 0xac, 0x49, 0x7f, 0x50, 0x5f, 0x3d, 0x87);

// --- COM factory table and registration code --------------

const AMOVIESETUP_MEDIATYPE
sudMediaTypes[] =
{
	{&MEDIATYPE_Stream,&MEDIASUBTYPE_NULL},
};
const AMOVIESETUP_PIN sudOutputPins[] =
{
	{
		L"Output",            // pin name
		FALSE,              // is rendered?    
		TRUE,               // is output?
		FALSE,              // zero instances allowed?
		TRUE,               // many instances allowed?
		&CLSID_NULL,        // connects to filter (for bridge pins)
		nullptr,            // connects to pin (for bridge pins)
		0,                  // count of registered media types
		nullptr             // list of registered media types
	},
	{
		L"Input",             // pin name
		FALSE,              // is rendered?    
		FALSE,              // is output?
		FALSE,              // zero instances allowed?
		FALSE,              // many instances allowed?
		&CLSID_NULL,        // connects to filter (for bridge pins)
		nullptr,            // connects to pin (for bridge pins)
		1,                  // count of registered media types
		&sudMediaTypes[0]   // list of registered media types
	}
};

const AMOVIESETUP_FILTER sudFilterReg =
{
	&CLSID_ZQVIDEOPcrAdjust,//&__uuidof(CPcrAdjustFilter),        // filter clsid
	L"Zqvideo Pcr filter",                   // filter name
	MERIT_PREFERRED + 4,            // merit
	2,                              // count of registered pins
	sudOutputPins,                  // list of pins to register
	CLSID_LegacyAmFilterCategory
};

// --- COM factory table and registration code --------------

// DirectShow base class COM factory requires this table, 
// declaring all the COM objects in this DLL
CFactoryTemplate g_Templates[] = {
	// one entry for each CoCreate-able object
	{
		sudFilterReg.strName,
		sudFilterReg.clsID,
		CPcrAdjustFilter::CreateInstance,
		nullptr,
		&sudFilterReg
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


// self-registration entrypoint
STDAPI DllRegisterServer()
{
	// base classes will handle registration using the factory template table
	return AMovieDllRegisterServer2(true);
}
STDAPI DllUnregisterServer()
{
	// base classes will handle de-registration using the factory template table
	return AMovieDllRegisterServer2(false);
}

// if we declare the correct C runtime entrypoint and then forward it to the DShow base
// classes we will be sure that both the C/C++ runtimes and the base classes are initialized
// correctly
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint(reinterpret_cast<HINSTANCE>(hDllHandle), dwReason, lpReserved);
}



//BOOL APIENTRY DllMain( HMODULE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//					 )
//{
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:
//	case DLL_PROCESS_DETACH:
//		break;
//	}
//	return TRUE;
//}

