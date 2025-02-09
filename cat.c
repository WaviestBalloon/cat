#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "thecat.h"

#include <windows.h>
#include <shlobj.h>
#include <tlhelp32.h>

bool file_exists(const char *filename) {
	FILE *fp = fopen(filename, "r");
	bool is_exist = false;
	if (fp != NULL) {
		is_exist = true;
		fclose(fp);
	}

	return is_exist;
}

bool set_bg(wchar_t *bg_path) {
	HKEY hKey;
	long reg_error = RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_ALL_ACCESS, &hKey);

	if (reg_error == ERROR_FILE_NOT_FOUND) {
		printf("regkey missing!\n");
	} else {
		printf("regkey open state = %ld\n", reg_error);
	}

	const char* sz_wallpaperstyle = "2";
	long long_result = RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE*)sz_wallpaperstyle, strlen(sz_wallpaperstyle) + 1);

	if (long_result != ERROR_SUCCESS) {
		printf("failure during regkey value set = %ld\n", long_result);
	}

	RegCloseKey(hKey);

	int status;
	status = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, bg_path, SPIF_UPDATEINIFILE);
	
	printf("spi state = %d\n", status);

	return true;
}

int kill_process_by_name(const char *target_process_name) {
	HANDLE handle_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (handle_snapshot == INVALID_HANDLE_VALUE) {
		printf("CreateToolhelp32Snapshot failure = %d\n", GetLastError());
		return 1;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(handle_snapshot, &pe32)) {
		printf("Process32First failure = %d\n", GetLastError());
		CloseHandle(handle_snapshot);
		return 1;
	}

	do {
		char* process_name = pe32.szExeFile;
		//printf("process found = %s (%s)\n", process_name, target_process_name);

		if (strcmp(process_name, target_process_name) == 0) {
			DWORD process_id = pe32.th32ProcessID;
			printf("terminating process = %s [%d]\n", process_name, process_id);
			
			HANDLE handle_process = OpenProcess(PROCESS_TERMINATE, FALSE, process_id);
			TerminateProcess(handle_process, 1);
			break;
		}
	} while (Process32Next(handle_snapshot, &pe32));

	CloseHandle(handle_snapshot);
}

int main(int argc, char *argv[]) {
	wchar_t appdata_local_path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata_local_path))) {
		wcscat(appdata_local_path, L"\\catto.png");
	} else {
		printf("unable to get appdata path :c\n");
		return 1;
	}

	char custom_path[MAX_PATH];
	for (int i = 0; i < argc; i++) {
		printf("arg = %s\n", argv[i]);
		if (i == 1) {
			strcpy(custom_path, argv[i]);
		}
	}

	char appdata_local_path_str[MAX_PATH];
	wcstombs(appdata_local_path_str, appdata_local_path, MAX_PATH);

	FILE *fp = fopen(appdata_local_path_str, "wb");
	if (fp == NULL) {
		printf("unable to drop catto :c\n");
		return 1;
	}

	printf("catto dropping into %s!!\n", appdata_local_path_str);
	size_t bytes_written = fwrite(__cat_png, sizeof(unsigned char), __cat_png_len, fp);
	printf("written %zu bytes\n", bytes_written);

	fclose(fp);
	
	if (!file_exists(appdata_local_path_str)) {
		printf("unable to find catto :c\n");
		return 1;
	}

	if (strlen(custom_path) != 1) {
		printf("using custom!!\n");
		wchar_t custom_path_wchar[MAX_PATH];
		mbstowcs(custom_path_wchar, custom_path, MAX_PATH);
		set_bg(custom_path_wchar);
	} else {
		set_bg(appdata_local_path);
	}

	//MessageBoxA(0, "Soggy cat has been deployed!", "cat.exe", 0);

	kill_process_by_name("wallpaper32.exe");
	kill_process_by_name("wallpaper64.exe");
	kill_process_by_name("ui32.exe");

	return 0;
}
