#include <direct.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <locale.h>

#include "serial.h"

#define MAXCOMPORT 128


// listup serial port driver 
// cf. http://www.codeproject.com/system/setupdi.asp?df=100&forumid=4368&exp=0&select=479661
// (2007.8.17 yutaka)
static void ListupSerialPort(LPWORD ComPortTable, int comports, 
							 TCHAR **ComPortDesc, int ComPortMax)
{
	GUID ClassGuid[1];
	DWORD dwRequiredSize;
	BOOL bRet;
	HDEVINFO DeviceInfoSet = NULL;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD dwMemberIndex = 0;

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	// Get ClassGuid from ClassName for PORTS class
	bRet = SetupDiClassGuidsFromName(TEXT("PORTS"), (LPGUID) & ClassGuid, 1,
									 &dwRequiredSize);
	if (!bRet) {
		goto cleanup;
	}

	// Get class devices
	DeviceInfoSet = SetupDiGetClassDevs(&ClassGuid[0], NULL, NULL, 
										DIGCF_PRESENT | DIGCF_PROFILE);

	if (DeviceInfoSet) {
		// Enumerate devices
		dwMemberIndex = 0;
		while (SetupDiEnumDeviceInfo
			   (DeviceInfoSet, dwMemberIndex++, &DeviceInfoData)) {
			TCHAR szFriendlyName[MAX_PATH];
			TCHAR szPortName[MAX_PATH];
			//TCHAR szMessage[MAX_PATH];
			DWORD dwReqSize = 0;
			DWORD dwPropType;
			DWORD dwType = REG_SZ;
			HKEY hKey = NULL;

			// Get friendlyname
			bRet = SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
													&DeviceInfoData,
													SPDRP_FRIENDLYNAME,
													&dwPropType,
													(LPBYTE)
													szFriendlyName,
													sizeof(szFriendlyName),
													&dwReqSize);

			// Open device parameters reg key
			hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
										&DeviceInfoData,
										DICS_FLAG_GLOBAL,
										0, DIREG_DEV, KEY_READ);
			if (hKey) {
				// Qurey for portname
				long lRet;
				dwReqSize = sizeof(szPortName);
				lRet = RegQueryValueEx(hKey,
									   TEXT("PortName"),
									   0,
									   &dwType,
									   (LPBYTE) & szPortName,
									   &dwReqSize);

				(void)lRet;
				// Close reg key
				RegCloseKey(hKey);
			}

#if 0
			_tprintf(TEXT("Name: %s\nPort: %s\n"), szFriendlyName, szPortName);
#endif

			if (_tcsnicmp(szPortName, TEXT("COM"), 3) == 0) {  
				int port = _ttoi(&szPortName[3]);
				int i;

				for (i = 0 ; i < comports ; i++) {
					if (ComPortTable[i] == port) {
						ComPortDesc[i] = _tcsdup(szFriendlyName);
						break;
					}
				}
			}

		}
	}

cleanup:
	// Destroy device info list
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}


int PASCAL DetectComPorts(LPWORD ComPortTable, int ComPortMax, 
						  TCHAR **ComPortDesc)
{
	HMODULE h;
	TCHAR   devicesBuff[65535];
	TCHAR   *p;
	int     comports = 0;
	int     i, j, min;
	WORD    s;

	if (((h = GetModuleHandle(TEXT("kernel32.dll"))) != NULL) &&
		(GetProcAddress(h, "QueryDosDeviceA") != NULL) &&
		(QueryDosDevice(NULL, devicesBuff, 65535) != 0)) {

		p = devicesBuff;
		while (*p != '\0') {
			if (_tcsncmp(p, TEXT("COM"), 3) == 0 && p[3] != '\0') {
				ComPortTable[comports++] = _ttoi(p+3);
				if (comports >= ComPortMax)
					break;
			}
			p += (_tcslen(p)+1);
		}
	
		for (i = 0; i < comports - 1; i++) {
			min = i;
			for (j=i+1; j<comports; j++) {
				if (ComPortTable[min] > ComPortTable[j]) {
					min = j;
				}
			}
			if (min != i) {
				s = ComPortTable[i];
				ComPortTable[i] = ComPortTable[min];
				ComPortTable[min] = s;
			}
		}
	} else {
#if 1
		for (i=1; i<=ComPortMax; i++) {
			FILE *fp;
			char buf[16]; // \\.\COMxxxx + NULL
			_snprintf(buf, sizeof(buf), "\\\\.\\COM%d", i);
			if ((fp = fopen(buf, "r")) != NULL) {
				fclose(fp);
				ComPortTable[comports++] = i;
			}
		}
#else
		comports = -1;
#endif
	}

	ListupSerialPort(ComPortTable, comports, ComPortDesc, ComPortMax);

	return comports;
}

int PASCAL CheckComPort(WORD ComPort)
{
	HMODULE h;
	TCHAR   devicesBuff[65535];
	TCHAR com_str[64];
	BOOL bRet;
	GUID ClassGuid[1];
	DWORD dwRequiredSize;
	HDEVINFO DeviceInfoSet = NULL;
	SP_DEVINFO_DATA DeviceInfoData;
	int found = 0;

	_stprintf_s(com_str, sizeof(com_str), TEXT("COM%d"), ComPort);

	if (((h = GetModuleHandle(TEXT("kernel32.dll"))) == NULL) || 
		(GetProcAddress(h, "QueryDosDeviceA") == NULL) ) {
		/* ERROR */
		return -1;
	}

	if (QueryDosDevice(com_str, devicesBuff, 65535) == 0) {
		DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) {
			/* NOT FOUND */
			return 0;
		}
		/* ERROR */
		return -1;
	}

	bRet = SetupDiClassGuidsFromName(TEXT("PORTS"), (LPGUID) & ClassGuid, 1, 
									 &dwRequiredSize);
	if (bRet == FALSE) {
		return -1;
	}

	DeviceInfoSet = SetupDiGetClassDevs(&ClassGuid[0], NULL, NULL, 
										DIGCF_PRESENT | DIGCF_PROFILE);
	if (DeviceInfoSet == NULL) {
		return -1;
	}

	if (DeviceInfoSet) {
		DWORD dwMemberIndex = 0;
		HKEY hKey = NULL;
		TCHAR szPortName[MAX_PATH];
		DWORD dwReqSize;
		DWORD dwType;

		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		while (SetupDiEnumDeviceInfo(DeviceInfoSet, dwMemberIndex, 
									 &DeviceInfoData)) {
			hKey = SetupDiOpenDevRegKey(DeviceInfoSet, &DeviceInfoData, 
										DICS_FLAG_GLOBAL, 0, DIREG_DEV, 
										KEY_READ);
			if (hKey) {
				long lRet;
				dwReqSize = sizeof(szPortName);
				lRet = RegQueryValueEx(hKey, TEXT("PortName"), 0, &dwType, 
									   (LPBYTE)& szPortName, &dwReqSize);
				(void)lRet;
				RegCloseKey(hKey);
				if (_tcsicmp(szPortName, com_str) == 0) {
					found = TRUE;
					break;
				}
			}
			dwMemberIndex++;
		}
	}

	SetupDiDestroyDeviceInfoList(DeviceInfoSet);

	return found;
}

int serial_port_list(struct port_entry lst[], int max)
{
	WORD ComPortTable[MAXCOMPORT];
	TCHAR * ComPortDesc[MAXCOMPORT];
	DWORD MaxComPort = 256;
	int comports;
	int cnt = 0;

	if ((comports = DetectComPorts(ComPortTable, MaxComPort, 
								   ComPortDesc)) >= 0) {
		int i;

		for (i = 0; i < comports; i++) {

			char * desc;

			if (ComPortTable[i] > MaxComPort) {
				continue;
			}

			desc = lst[cnt].desc;

			WideCharToMultiByte(CP_UTF8, 0, ComPortDesc[i], -1,
								desc, SERIAL_PORT_DESC_MAX, NULL, NULL);
			sprintf(lst[cnt].path, "\\\\.\\COM%d", ComPortTable[i]);

			cnt++;
/*
			if (CheckCOMFlag(ComPortTable[i]) == 1) {
				continue;
			}
			_snprintf_s(EntName, sizeof(EntName), _TRUNCATE, "COM%d", 
						ComPortTable[i]);

			if (ComPortDesc[i] != NULL) {
				strncat_s(EntName, sizeof(EntName), ": ", _TRUNCATE);
				strncat_s(EntName, sizeof(EntName), ComPortDesc[i], _TRUNCATE);
			}
*/
		}
	} else {
	}

	return cnt;
}

