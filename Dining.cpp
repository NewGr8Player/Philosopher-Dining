/*
 * Author：冯泽明
 * Date：2015/12/29
 * Description：MFC窗体 ： 激活不同菜单项目对程序运行的影响
 */

#include "stdafx.h"
#include "mtverify.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;								
TCHAR szTitle[MAX_LOADSTRING];					
TCHAR szWindowClass[MAX_LOADSTRING];			

HWND hWndMain ;

RECT rcWinExt = { 0, 0, 200, 200 } ;

int g_nXExt = 200, g_nYExt = 200 ;
int g_nRadiusTable = g_nXExt / 4 ;
POINT g_ptWinOrg = { g_nXExt / 2, g_nYExt / 2 } ;

int g_nChopstickState[PHILOSOPHERS] ;
int g_nDinerState[PHILOSOPHERS] ;
HANDLE g_hChopstickMutex[PHILOSOPHERS] ;
HANDLE g_hThrd[PHILOSOPHERS] ;

BOOL g_bWaitMultiple = TRUE ;
BOOL g_bFastDining = FALSE ;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MYDINING, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYDINING));

// 	CREATE_DEBUG_CONSOLE ;

// 	_tprintf(_T("Time: %d\n"), time(NULL)) ;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

// 	RELEASE_DEBUG_CONSOLE ;

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYDINING));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1) ;
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MYDINING);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      50, 5, 800, 600, NULL, NULL, hInstance, NULL);

   hWndMain = hWnd ;

   if (!hWnd)
   {
      return FALSE;
   }

   for (int i = 0 ; i < PHILOSOPHERS ; i++)
   {
	   g_nDinerState[i] = THINKING ; 
	   g_nChopstickState[i] = UNUSED ;
	   g_hChopstickMutex[i] = CreateMutex(NULL, FALSE, NULL) ;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   Dining() ;

   return TRUE;
}

void RenderFrame(HDC hdc)
{
	RECT rcClient, rc ;
	HPEN hPenOld ;
	HFONT hFont, hFontOld ;
	int  nRadius ;
	int nPos, nPosNext, x, y, nXPhilosopher, nYPhilosopher ;
	double dAngle, dRadian ;
	TCHAR  szName[16] ;

	GetClientRect(hWndMain, &rcClient) ;

	SelectObject(hdc, GetStockObject(NULL_BRUSH)) ;

	SetMapMode(hdc, MM_ANISOTROPIC) ;

	SetWindowExtEx(hdc, g_nXExt, g_nYExt, NULL) ;
 	SetWindowOrgEx(hdc, g_ptWinOrg.x, g_ptWinOrg.y, NULL) ;
	SetViewportExtEx(hdc, rcClient.right, rcClient.bottom, NULL) ;
	SetViewportOrgEx(hdc, rcClient.right / 2, rcClient.bottom / 2, NULL) ;

	hFont = CreateFont(5, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_SWISS, TEXT("楷体")) ;
	hFontOld = (HFONT) SelectObject(hdc, hFont) ;

	rc.left  = g_ptWinOrg.x - 10 ; rc.top    = g_ptWinOrg.y - 10 ;
	rc.right = g_ptWinOrg.x + 10 ; rc.bottom = g_ptWinOrg.y + 10 ;
	Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom) ;
	Ellipse(hdc, rc.left - 40, rc.top - 40, rc.right + 40, rc.bottom + 40) ;
	StringCchPrintf(szName, 16, TEXT("桌子")) ;
	DrawText(hdc, szName, -1, &rc,  DT_CENTER | DT_SINGLELINE | DT_VCENTER) ;

	dAngle = 360.0 / PHILOSOPHERS ;
	for (nPos = 0 ; nPos < PHILOSOPHERS ; nPos++)
	{
		dRadian = (nPos * dAngle) / ANGLEPERRADIAN ;
		x = g_ptWinOrg.x + (int)(sin(dRadian) * g_nRadiusTable * 0.5) ;
		y = g_ptWinOrg.y - (int)(cos(dRadian) * g_nRadiusTable * 0.5) ;
		MoveToEx(hdc, x, y, NULL) ;

		x = g_ptWinOrg.x + (int)(sin(dRadian) * g_nRadiusTable * 0.9) ;
		y = g_ptWinOrg.y - (int)(cos(dRadian) * g_nRadiusTable * 0.9) ;
		LineTo(hdc, x, y) ;
	}

	for (nPos = 0 ; nPos < PHILOSOPHERS ; nPos++)
	{
		nRadius = (int) (g_nRadiusTable * 0.2) ;
		dRadian = (nPos * dAngle + dAngle / 2) / ANGLEPERRADIAN ;
		x = g_ptWinOrg.x + (int)(sin(dRadian) * g_nRadiusTable * 0.6) ;
		y = g_ptWinOrg.y - (int)(cos(dRadian) * g_nRadiusTable * 0.6) ;
		rc.left  = x - nRadius ; rc.top    = y - nRadius ;
		rc.right = x + nRadius ; rc.bottom = y + nRadius ;

		switch (g_nDinerState[nPos])
		{
		case THINKING:
			hPenOld = (HPEN) SelectObject(hdc, CreatePen(PS_SOLID, 1, 0x00FF00)) ;
			SetTextColor(hdc, 0x00FF00) ;
			StringCchPrintf(szName, 16, TEXT("思考ing")) ;
			break ;

		case WAITING:
			hPenOld = (HPEN) SelectObject(hdc, CreatePen(PS_SOLID, 1, 0x0000FF)) ;
			SetTextColor(hdc, 0x0000FF) ;
			StringCchPrintf(szName, 16, TEXT("想吃ing!")) ;
			break ;

		case EATING:
			hPenOld = (HPEN) SelectObject(hdc, CreatePen(PS_SOLID, 1, 0xE28C12)) ;
			SetTextColor(hdc, 0xE28C12) ;
			StringCchPrintf(szName, 16, TEXT("吃饭ing!")) ;
			break ;
		}

		Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom) ; 
		DrawTextEx(hdc, szName, -1, &rc,  DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL) ;

		nRadius = (int)(g_nRadiusTable * 0.3) ;
		dRadian = (nPos * dAngle + dAngle / 2) / ANGLEPERRADIAN ;
		nXPhilosopher = g_ptWinOrg.x + (int)(sin(dRadian) * g_nRadiusTable * 1.4) ;
		nYPhilosopher = g_ptWinOrg.y - (int)(cos(dRadian) * g_nRadiusTable * 1.4) ;

		rc.left  = nXPhilosopher - nRadius ; rc.top    = nYPhilosopher - nRadius ;
		rc.right = nXPhilosopher + nRadius ; rc.bottom = nYPhilosopher + nRadius ;
		Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom) ;

		SetTextColor(hdc, 0x0) ;
		StringCchPrintf(szName, 16, TEXT("哲学家 %d"), nPos + 1) ;
		DrawText(hdc, szName, -1, &rc,  DT_CENTER | DT_SINGLELINE | DT_VCENTER) ;

		if (g_nChopstickState[nPos] == nPos)
		{
			dRadian = (nPos * dAngle) / ANGLEPERRADIAN ;
			x = g_ptWinOrg.x + (int)(sin(dRadian) * g_nRadiusTable * 0.9) ;
			y = g_ptWinOrg.y - (int)(cos(dRadian) * g_nRadiusTable * 0.9) ;
			MoveToEx(hdc, x, y, NULL) ;
			LineTo(hdc, nXPhilosopher, nYPhilosopher) ;
		}
		
		nPosNext = nPos + 1 ;
		if (nPosNext >= PHILOSOPHERS)
			nPosNext = 0 ;

		if (g_nChopstickState[nPosNext] == nPos)
		{
			dRadian = (nPosNext * dAngle) / ANGLEPERRADIAN ;
			x = g_ptWinOrg.x + (int)(sin(dRadian) * g_nRadiusTable * 0.9) ;
			y = g_ptWinOrg.y - (int)(cos(dRadian) * g_nRadiusTable * 0.9) ;
			MoveToEx(hdc, x, y, NULL) ;
			LineTo(hdc, nXPhilosopher, nYPhilosopher) ;
		}

		DeleteObject(SelectObject(hdc, hPenOld)) ;
	}

	DeleteObject(SelectObject(hdc, hFontOld)) ;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		EnableMenuItem(GetMenu(hWnd), IDM_WAITFORMULTIPLE, MF_GRAYED | MF_BYCOMMAND) ;
		EnableMenuItem(GetMenu(hWnd), IDM_CREATETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
		EnableMenuItem(GetMenu(hWnd), IDM_RESUMETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
		break ;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDM_WAITFORMULTIPLE:
			EnableMenuItem(GetMenu(hWnd), IDM_WAITFORMULTIPLE, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_FASTDEADLOCK, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_SLOWDEADLOCK, MF_ENABLED | MF_BYCOMMAND) ;
			g_bWaitMultiple = TRUE ;
			g_bFastDining = FALSE ;
			break ;

		case IDM_FASTDEADLOCK:
			EnableMenuItem(GetMenu(hWnd), IDM_FASTDEADLOCK, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_WAITFORMULTIPLE, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_SLOWDEADLOCK, MF_ENABLED | MF_BYCOMMAND) ;
			g_bWaitMultiple = FALSE ;
			g_bFastDining = TRUE ;
			break ;

		case IDM_SLOWDEADLOCK:
			EnableMenuItem(GetMenu(hWnd), IDM_SLOWDEADLOCK, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_WAITFORMULTIPLE, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_FASTDEADLOCK, MF_ENABLED | MF_BYCOMMAND) ;
			g_bWaitMultiple = FALSE ;
			g_bFastDining = FALSE ;
			break ;

		case IDM_CREATETHREAD:
			EnableMenuItem(GetMenu(hWnd), IDM_CREATETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_TERMINATETHREAD, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_SUSPENDTHREAD, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_RESUMETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			Dining() ;
			break ;

		case IDM_SUSPENDTHREAD:
			EnableMenuItem(GetMenu(hWnd), IDM_SUSPENDTHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_RESUMETHREAD, MF_ENABLED | MF_BYCOMMAND) ;
			for (int i = 0 ; i < PHILOSOPHERS ; i++)
			{
				SuspendThread(g_hThrd[i]) ;
			}
			break ;

		case IDM_RESUMETHREAD:
			EnableMenuItem(GetMenu(hWnd), IDM_SUSPENDTHREAD, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_RESUMETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			for (int i = 0 ; i < PHILOSOPHERS ; i++)
			{
				ResumeThread(g_hThrd[i]) ;
			}
			break ;

		case IDM_TERMINATETHREAD:
			EnableMenuItem(GetMenu(hWnd), IDM_TERMINATETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_CREATETHREAD, MF_ENABLED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_RESUMETHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			EnableMenuItem(GetMenu(hWnd), IDM_SUSPENDTHREAD, MF_GRAYED | MF_BYCOMMAND) ;
			for (int i = 0 ; i < PHILOSOPHERS ; i++) 
			{
				TerminateThread(g_hThrd[i], i) ;
				g_nDinerState[i] = THINKING ; 
				g_nChopstickState[i] = UNUSED ;
			}
			InvalidateRect(hWnd, NULL, TRUE) ;
			break ;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		DrawMenuBar(hWnd) ;
		break;

	case WM_FORCE_REPAINT:
		{
			MSG msg ;
			
			InvalidateRect(hWnd, NULL, TRUE) ;
			while (PeekMessage(&msg, hWnd, WM_FORCE_REPAINT, WM_FORCE_REPAINT, PM_REMOVE))
				;
		}
		break ;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		RenderFrame(hdc) ;
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		for (int i = 0 ; i < PHILOSOPHERS ; i++)
		{
			CloseHandle(g_hChopstickMutex[i]) ;
			TerminateThread(g_hThrd[i], i) ;
		}
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
