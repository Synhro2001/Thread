#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "resource.h"
#include <iostream>
#include <commctrl.h>

bool Terminate = false;
HWND hMainWnd = 0;
HANDLE hClock = 0;
bool ClockPaused = false;
HANDLE handle;

BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL,	DLGPROC(MainWndProc));
	return 0;

}


DWORD WINAPI ClockThread(LPVOID lpParameter) {
	while (!Terminate) {
		char timestr[9];
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf(timestr, "%.2d:%.2d:%.2d",
			time.wHour, time.wMinute, time.wSecond);
		SetDlgItemText(hMainWnd, IDC_CLOCK, timestr);
		Sleep(250);
	}
	return 0;
}


BOOL RunNotepad(HWND hWnd) { // ������ �������
	char processName[50]; // ��� ������ "Notepad.exe"
	int processNameLength = sizeof(processName);
	if (!GetDlgItemText(hWnd, IDC_COMMANDLINE, processName, processNameLength)) // ������ ��� �� �������� � ������ � ��������� �����
		return 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si)); // �������� ����� ������������� ��������
	si.cb = sizeof(si);

	
		if (!CreateProcess(
			NULL,
			processName, // ��������� ������
			NULL,
			NULL,
			FALSE,
			0,   // ������ ��������
			NULL,
			NULL,
			&si, // ���������� �������������
			&pi) // ���������� ��������
			)return 0;

	if (handle != 0) CloseHandle(handle);
	CloseHandle(pi.hThread);
	handle = pi.hProcess;
	return 1; // return true



}

bool BrowseFileName(HWND Wnd, char* FileName) {
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn); // ���������� ������ � ������ !!! 
	ofn.hwndOwner = Wnd;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0" // �������������� ������, ������� ��������� ������
		"All Files(*.*)\0 * .*\0"; //  ���������� ������ �������
	ofn.lpstrFile = FileName; // ��������� �� �����, ������� �������� ��� �����
	ofn.nMaxFile = MAX_PATH; // ������ ������
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "exe";

	return GetOpenFileName(&ofn); // ��������� ���������� ���� � ������� ��������� ����������� ����, ��� �����...
}

unsigned __int64 SystemTimeToInt(SYSTEMTIME Time) {
	FILETIME ft;
	SystemTimeToFileTime(&Time, &ft);
	ULARGE_INTEGER fti;
	fti.HighPart = ft.dwHighDateTime;
	fti.LowPart = ft.dwLowDateTime;
	return fti.QuadPart;
}

DWORD WINAPI ScheduleThread(LPVOID lpParameter) {
	SYSTEMTIME time;
	SYSTEMTIME localtime;
	int pos_time;
	char time_str[9];
	SendDlgItemMessage(hMainWnd, IDC_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&time);
	while (!Terminate) {
		GetLocalTime(&localtime);
		if (SystemTimeToInt(localtime) > SystemTimeToInt(time)) break;
		Sleep(10);
	}
	sprintf(time_str, "%.2d:%.2d:%.2d", time.wHour, time.wMinute, time.wSecond);
	pos_time = SendDlgItemMessage(hMainWnd, IDC_TIMELIST, LB_FINDSTRINGEXACT, -1, (LPARAM)time_str);
	if (!Terminate) {
		RunNotepad(hMainWnd);
	}
	SendDlgItemMessage(hMainWnd, IDC_TIMELIST, LB_DELETESTRING, -1, pos_time);
}



BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_INITDIALOG:
		hMainWnd = hWnd;
		hClock = CreateThread(NULL, 0, ClockThread, NULL, 0, NULL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_PAUSE: {
			if (ClockPaused == false) {
				SuspendThread(hClock); // ���������������� ����� ��������� ������
				ClockPaused = true;
				return TRUE;
			}
			if (ClockPaused == true) {
				ResumeThread(hClock); // ��������� ���� ������� ������������ ������ ������
				ClockPaused = false;
				return TRUE;
			}
		}
		case IDOK:
			DestroyWindow(hWnd);
			return TRUE;

		case IDC_BROWSE: {
			char filename[MAX_PATH] = "notepad.exe";
			if (BrowseFileName(hWnd, filename)) SetDlgItemText(hWnd, IDC_COMMANDLINE, filename);
			return TRUE;
		}
	
		return FALSE;
	case WM_DESTROY:
		Terminate = true;
		Sleep(500);
		CloseHandle(hClock);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}