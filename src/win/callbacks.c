
#include "callbacks.h"
#include "resource.h"
#include <stdio.h>

#define IDC_MAIN_EDIT	101

struct terminal {
  HWND hWnd;
};


struct terminal * WinTerminalInit(HWND hWnd)
{
	static struct terminal term;

	term.hWnd = GetDlgItem(hWnd, IDC_MAIN_EDIT);

	return &term;
}

void term_puts(struct terminal * term, const char * s)
{
	HWND hwnd = term->hWnd;
	// get the current selection
	DWORD StartPos, EndPos;
	int outLength;
	LPWSTR  lpwcs;
	int n;

	n = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)s, -1, NULL, 0);
	lpwcs = (LPWSTR)LocalAlloc(0, n * sizeof(TCHAR));
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)s, -1, lpwcs, n);

	SendMessage(hwnd, EM_GETSEL, (WPARAM)&StartPos, (WPARAM)&EndPos);

	// move the caret to the end of the text
	outLength = GetWindowTextLength(hwnd);
	SendMessage(hwnd, EM_SETSEL, outLength, outLength);

	// insert the text at the new caret position
	SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)lpwcs);

	// restore the previous selection
	SendMessage(hwnd, EM_SETSEL, StartPos, EndPos);

	LocalFree(lpwcs);
}

int term_vprintf(struct terminal * term, const char * fmt, va_list ap)
{
	char s[1024];
	int cnt;

	cnt = vsnprintf(s, 1023, fmt, ap);
	term_puts(term, s);

	return cnt;
}

int term_printf(struct terminal * term, const char * fmt, ...)
{
	va_list ap;
	int cnt;

	va_start(ap, fmt);
	cnt = term_vprintf(term, fmt, ap);
	va_end(ap);

	return cnt;
}

static void SetEditCtrlFont(HWND hwnd)
{
	long lfHeight;
	HWND hEdit;
	HDC hdc;
	HFONT hf;

	hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);

	hdc = GetDC(NULL);
	lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);

	hf = CreateFont(lfHeight, 0, 0, 0, 0, FALSE, 
					0, 0, 0, 0, 0, 0, 0, TEXT("Lucida Console"));

	if(hf) {
//		DeleteObject(g_hfFont);
//		g_hfFont = hf;
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hf, MAKELPARAM(FALSE, 0));
	} else {
		MessageBox(hwnd, TEXT("Font creation failed!"), 
				   TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
	}     

}

static HWND CreateEditCtrl(HWND hwnd)
{
	HWND hEdit;

	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), 
						   WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
						   WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL |
						   ES_AUTOHSCROLL, 
						   0, 0, 100, 100, 
						   hwnd, (HMENU)IDC_MAIN_EDIT, 
						   GetModuleHandle(NULL), NULL);
	if(hEdit == NULL)
		MessageBox(NULL, TEXT("Could not create edit box."), 
				   TEXT("Error"), MB_ICONERROR | MB_OK);

	SetEditCtrlFont(hwnd);

	return hEdit;
}

static void ResizeEditCtrl(HWND hwnd)
{
	HWND hEdit;
	RECT rcClient;

	GetClientRect(hwnd, &rcClient);

	hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
	SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, 
				 rcClient.bottom, SWP_NOZORDER);
}

// Window procedure for our main window.
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInstance;

	switch (msg) {  
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_HELP_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), 
					  hWnd, &AboutDialogProc);
			return 0;

		case ID_FILE_EXIT:
			DestroyWindow(hWnd);
			return 0;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *minMax = (MINMAXINFO*) lParam;
			minMax->ptMinTrackSize.x = 220;
			minMax->ptMinTrackSize.y = 110;
		}
		return 0;

	case WM_SYSCOMMAND:
		switch (LOWORD(wParam)) {
		case ID_HELP_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), 
					  hWnd, &AboutDialogProc);
			return 0;
		}
		break;

	case WM_CREATE:
		hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
		CreateEditCtrl(hWnd);
		return 0;

	case WM_SIZE:
		ResizeEditCtrl(hWnd);
		break;


	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


// Dialog procedure for our "about" dialog.
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, 
								 LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, (INT_PTR) LOWORD(wParam));
			return (INT_PTR) TRUE;
		}
		break;

	case WM_INITDIALOG:
		return (INT_PTR) TRUE;
	}

	return (INT_PTR) FALSE;
}
