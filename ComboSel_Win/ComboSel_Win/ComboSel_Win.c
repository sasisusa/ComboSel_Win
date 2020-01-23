#if defined(_WIN32) || defined(_WIN64)


#if !(defined(WINVER) && defined(_WIN32_WINNT))
	#include <sdkddkver.h>
	#undef WINVER
	#define WINVER _WIN32_WINNT_VISTA
	#undef _WIN32_WINNT
	#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include "ComboSel_Win.h"


#define CSISWIN_WIDTH 308
#define CSISWIN_HEIGHT 88


typedef struct tagCOMBOSELPARAM{
	const char *sText;
	const char **psItems;
	int iN;
	int iRet;
	int iSel;
}COMBOSELPARAM;

#define INVALID_SEL CB_ERR

#define ID_OK 40000
#define ID_CANCEL 40001
#define ID_CB 40002


static LRESULT CALLBACK ComboSelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:{
		HWND hWndTL, hWndCB, hWndOk, hWndCancel;
		HFONT hFont;
		COMBOSELPARAM *pcsParam;
		int i;

		pcsParam = ((CREATESTRUCT*)lParam)->lpCreateParams;
		if(!pcsParam){
			DestroyWindow(hWnd);
			return -1;
		}
		SetClassLongPtr(hWnd, 0, (LONG_PTR)pcsParam);
		if((COMBOSELPARAM*)GetClassLongPtr(hWnd, 0) != pcsParam){
			DestroyWindow(hWnd);
			return -1;
		}

		hWndTL = CreateWindowEx(0, WC_STATIC, pcsParam->sText, WS_VISIBLE | WS_CHILD,
								4, 4, 300, 50, hWnd, NULL, ((CREATESTRUCT*)lParam)->hInstance, NULL);
		hWndCB = CreateWindowEx(0, WC_COMBOBOX, NULL,
								WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_AUTOHSCROLL,
								8, 26, 292, 180, hWnd, (HMENU)ID_CB, ((CREATESTRUCT*)lParam)->hInstance, NULL);
		hWndOk = CreateWindowEx(0, WC_BUTTON, "OK", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_FLAT,
								126, 60, 86, 22, hWnd, (HMENU)ID_OK, ((CREATESTRUCT*)lParam)->hInstance, NULL);
		hWndCancel = CreateWindowEx(0, WC_BUTTON, "Cancel", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_FLAT | WS_VISIBLE,
									216, 60, 86, 22, hWnd, (HMENU)ID_CANCEL, ((CREATESTRUCT*)lParam)->hInstance, NULL);
		if(!(hWndTL && hWndCB && hWndOk && hWndCancel)){
			DestroyWindow(hWnd);
			return -1;
		}

		hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if(hFont){
			SendMessage(hWndTL, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
			SendMessage(hWndOk, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
			SendMessage(hWndCancel, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
		}
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		if(hFont){
			SendMessage(hWndCB, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
		}

		for(i=0; i<pcsParam->iN; ++i){
			SendMessage(hWndCB, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)pcsParam->psItems[i]);
		}
		SendMessage(hWndCB, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	}return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case ID_OK:{
			COMBOSELPARAM *pcsParam = (COMBOSELPARAM*)GetClassLongPtr(hWnd, 0);
			HWND hWndCB = GetDlgItem(hWnd, ID_CB);
			int iSel = CB_ERR;

			if(hWndCB){
				iSel = SendMessage(hWndCB, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			}

			if(pcsParam){
				if(iSel == CB_ERR){
					pcsParam->iSel = INVALID_SEL;
					pcsParam->iRet = __LINE__;
				}
				else{
					pcsParam->iSel = iSel;
					pcsParam->iRet = 0;
				}

			}
			DestroyWindow(hWnd);
		}break;
		case ID_CANCEL:{
			COMBOSELPARAM *pcsParam = (COMBOSELPARAM*)GetClassLongPtr(hWnd, 0);

			if(pcsParam){
				pcsParam->iSel = INVALID_SEL;
				pcsParam->iRet = COMBOSEL_RET_CANCEL;;
			}
			DestroyWindow(hWnd);
		}break;
		}
		break;
	case WM_CLOSE:{
		COMBOSELPARAM *pcsParam = (COMBOSELPARAM*)GetClassLongPtr(hWnd, 0);

		if(pcsParam){
			pcsParam->iRet = COMBOSEL_RET_CLOSE;;
		}
		}break;
    case WM_DESTROY:
    	PostQuitMessage(0);
    	return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//	ComboSel
//
//  Returns:
//		> 0		on failure
//		0		on success
//		< 0:	COMBOSEL_RET_CANCEL		Cancel-button
//				COMBOSEL_RET_CLOSE		Alt F4, Close Window, Menu->Exit
//
int ComboSel(const char *sTitel, const char *sText, const char **psItem, int iN, int *piSel)
{
	HWND hWndD;
	int iX, iY;
	MSG msg;
	ATOM atomClassID;
	DWORD dwStyle, dwExStyle;
	WNDCLASSEX wcex;
	RECT rect;
	COMBOSELPARAM csParam;

	if(!(sText && psItem && iN > 0 && piSel)){
		return 1;
	}

	InitCommonControls();

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ComboSelProc;
	wcex.cbClsExtra = sizeof(&csParam);
	wcex.hInstance = NULL;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = "ComboSel";

	atomClassID = RegisterClassEx(&wcex);
	if(!atomClassID){
		return __LINE__;
	}

	ZeroMemory(&rect, sizeof(rect));
	ZeroMemory(&csParam, sizeof(csParam));
	dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	csParam.sText = sText;
	csParam.psItems = psItem;
	csParam.iN = iN;
	csParam.iSel = INVALID_SEL;

	SetRect(&rect, 0, 0, CSISWIN_WIDTH, CSISWIN_HEIGHT);
	AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);


	iX = (GetSystemMetrics(SM_CXSCREEN)-(rect.right-rect.left))>>1;
	if(iX < 1){
		iX = CW_USEDEFAULT;
	}
	iY = (GetSystemMetrics(SM_CYSCREEN)-(rect.bottom-rect.top))>>1;
	if(iY < 1){
		iY = CW_USEDEFAULT;
	}
	hWndD = CreateWindowEx( dwExStyle, MAKEINTATOM(atomClassID), sTitel, dwStyle,
							iX, iY, rect.right-rect.left, rect.bottom-rect.top,
							NULL, NULL, wcex.hInstance, &csParam);

	if(!hWndD){
		UnregisterClass(MAKEINTATOM(atomClassID), wcex.hInstance);
		return __LINE__;
	}

	ShowWindow(hWndD, SW_SHOW);
	UpdateWindow(hWndD);
	SetForegroundWindow(hWndD);
	SetFocus(hWndD);

	while(GetMessage(&msg, NULL, 0, 0) > 0){
		if(!IsDialogMessage(hWndD, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if(!UnregisterClass(MAKEINTATOM(atomClassID), wcex.hInstance)){
		csParam.iRet = __LINE__;
	}

	if(!csParam.iRet){
		if(csParam.iSel == INVALID_SEL){
			csParam.iRet = __LINE__;
		}
		else{
			if(!csParam.iRet){
				*piSel = csParam.iSel;
			}
		}
	}

	return csParam.iRet;
}


#endif
