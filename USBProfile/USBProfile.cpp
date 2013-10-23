/* 
	USBProfile
	© ultrashot 2013
*/
#include "stdafx.h"
#include "regext.h"
#include "xnaframeworkcore.h"

typedef struct
{
	wchar_t *name;
	wchar_t item[500];
}LANGUAGE_ITEM;

#define LNG_ITEM_EXPAND(a) L##a, L##a

LANGUAGE_ITEM items[] = {
	{LNG_ITEM_EXPAND("Title")},
	{LNG_ITEM_EXPAND("CurrentProfileDefault")},
	{LNG_ITEM_EXPAND("CurrentProfileVideoOut")},
	{LNG_ITEM_EXPAND("DoYouWantToSwitchToDefault")}, 
	{LNG_ITEM_EXPAND("DoYouWantToSwitchToVideoOut")},
	{LNG_ITEM_EXPAND("SlowdownWarning")},
	{LNG_ITEM_EXPAND("AfterRebootWarning")},
	{LNG_ITEM_EXPAND("RebootWarning")},
	{LNG_ITEM_EXPAND("Yes")},
	{LNG_ITEM_EXPAND("No")},
	{LNG_ITEM_EXPAND("Ok")},
	{LNG_ITEM_EXPAND("Cancel")},
	{LNG_ITEM_EXPAND("Retry")},
	{NULL, L" "}
};

typedef enum
{
	Title = 0,
	CurrentProfileIsDefault,
	CurrentProfileIsVideoOut,
	DoYouWantToSwitchToDefault,
	DoYouWantToSwitchToVideoOut,
	SlowdownWarning,
	AfterRebootWarning,
	RebootWarning,
	Yes,
	No,
	Ok,
	Cancel,
	Retry
}Language;


wchar_t *GetTranslation(Language lng)
{
	return items[lng].item;
}


void InitializeLanguages()
{
	DWORD dwValue = 0;
	HKEY hKey = NULL;
	wchar_t keyName[500];
	swprintf(keyName, L"Software\\OEM\\UsbProfile\\Localization");
	RegCreateKeyExW(HKEY_LOCAL_MACHINE, keyName, 0, NULL, 0, 0, NULL, &hKey, &dwValue);

	if (hKey)
	{
		int x = 0;
		while (true)
		{
			if (items[x].name == NULL)
				break;
			DWORD size = 500;
			DWORD type = REG_SZ;
			RegQueryValueEx(hKey, items[x].name, 0, &type, (LPBYTE)items[x].item, &size);
			x++;
		}
		RegCloseKey(hKey);
	}
}




#define XNA_RESULT_NONE		0
#define XNA_RESULT_LEFT		1
#define XNA_RESULT_RIGHT	2

int XnaMessageBox(HWND hWnd, wchar_t *text, wchar_t *caption, int flags)
{
	int icon = None;
	wchar_t *btn1 = GetTranslation(Ok);
	wchar_t *btn2 = NULL;
	if (flags & MB_OKCANCEL)
	{
		btn2 = GetTranslation(Cancel);
	}
	else if (flags & MB_YESNO)
	{
		btn1 = GetTranslation(Yes);
		btn2 = GetTranslation(No);
	}
	else if (flags & MB_RETRYCANCEL)
	{
		btn1 = GetTranslation(Retry);
		btn2 = GetTranslation(Cancel);
	}
	if (flags & MB_ICONQUESTION)
		icon = Alert;
	else if (flags & MB_ICONEXCLAMATION)
		icon = Warning;

	GS_ActionDialogResult result;
	int funcResult = XNA_RESULT_NONE;
	HANDLE mb = CreateMessageBoxDialog(caption, text, btn1, btn2, Alert);
	
	GetDialogResult(mb, &result);
	if (result.disReason == ButtonPressed)
	{
		if (result.iButtonPressed == 0)
		{
			funcResult = XNA_RESULT_LEFT;
		}
		else if (result.iButtonPressed == 1)
		{
			funcResult = XNA_RESULT_RIGHT;
		}
	}
	ReleaseDialog(mb);
	return funcResult;
}

int _tmain(int argc, _TCHAR* argv[])
{
	InitializeLanguages();
	bool fromStart = false;
	if (argc == 2 && wcsicmp(argv[1], L"-fromstart") == 0)
		fromStart = true;

	if (fromStart == true)
	{
		wchar_t defClientDriver[500];
		RegistryGetString(HKEY_LOCAL_MACHINE, L"Drivers\\USB\\FunctionDrivers", L"DefaultClientDriver", defClientDriver, sizeof(defClientDriver));
		bool isComposite = false;
		if (wcsicmp(defClientDriver, L"CompositeFN") == 0)
		{
			isComposite = true;
		}
		wchar_t msg[1000];
		swprintf(msg, L"%ls\r\n%ls\r\n\r\n%ls", 
			isComposite ? GetTranslation(CurrentProfileIsDefault) : GetTranslation(CurrentProfileIsVideoOut),
			isComposite ? GetTranslation(DoYouWantToSwitchToVideoOut) : GetTranslation(DoYouWantToSwitchToDefault), 
			isComposite ? GetTranslation(SlowdownWarning) : L"");

		if (XnaMessageBox(NULL, msg, GetTranslation(Title), MB_YESNO) == XNA_RESULT_LEFT)
		{
			RegistrySetString(HKEY_LOCAL_MACHINE, L"Drivers\\USB\\FunctionDrivers", L"DefaultClientDriver", isComposite ? L"UFN_VIDSTREAM_CLASS" : L"CompositeFN");
			if (XnaMessageBox(NULL, GetTranslation(RebootWarning), GetTranslation(Title), MB_YESNO) == XNA_RESULT_LEFT)
			{
				ExitWindowsEx(EWX_REBOOT, 0);
			}
		}
	}
	else
	{
		wchar_t defClientDriver[500];
		RegistryGetString(HKEY_LOCAL_MACHINE, L"Drivers\\USB\\FunctionDrivers", L"DefaultClientDriver", defClientDriver, sizeof(defClientDriver));
		bool isComposite = false;
		if (wcsicmp(defClientDriver, L"UFN_VIDSTREAM_CLASS") == 0)
		{
			XnaMessageBox(NULL, GetTranslation(AfterRebootWarning), GetTranslation(Title), MB_OK);
		}
	}
	return 0;
}

