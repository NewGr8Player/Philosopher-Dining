/*
 * Author：冯泽明
 * Date：2015/12/29
 * Description：互斥
 * Function List：
 *		DWORD WINAPI DiningThread(LPVOID pVoid)； 用餐线程：	使用WaitForMultipleObjects()函数实现预先分配 
 *															使用WaitForSingleObject()函数实现死锁演示
 *		void Dining()；初始化 [PHILOSOPHERS] 个线程
 */

#include "stdafx.h"

extern HWND hWndMain ;

extern int g_nChopstickState[] ;
extern int g_nDinerState[] ;
extern HANDLE g_hChopstickMutex[] ;
extern HANDLE g_hThrd[] ;

extern BOOL g_bWaitMultiple ;
extern BOOL g_bFastDining ;

DWORD WINAPI DiningThread(LPVOID pVoid)
{
	int nMilSec ;

	int nPhilosopher = (int) pVoid;
	int nLeftChopstick = nPhilosopher ;
	int nRightChopstick = nLeftChopstick + 1 ;
	HANDLE hChopstickMutex[2] ;

	if (nRightChopstick > PHILOSOPHERS - 1) {
		nRightChopstick = 0 ;
	}

	hChopstickMutex[0] = g_hChopstickMutex[nLeftChopstick] ;
	hChopstickMutex[1] = g_hChopstickMutex[nRightChopstick] ;

	srand((unsigned) time(NULL) + (nPhilosopher + 1)) ;

	Sleep(DINING_DELAY) ;
 
	for (; ;)
	{
		if (g_bWaitMultiple)
		{
			g_nDinerState[nPhilosopher] = WAITING ;
			SendMessage(hWndMain, WM_FORCE_REPAINT, NULL, NULL) ;
			WaitForMultipleObjects(2, hChopstickMutex, TRUE, INFINITE) ;
			g_nChopstickState[nLeftChopstick] = nPhilosopher ;
			g_nChopstickState[nRightChopstick] = nPhilosopher ;
		}
		else 
		{
			g_nDinerState[nPhilosopher] = WAITING ;
			SendMessage(hWndMain, WM_FORCE_REPAINT, NULL, NULL) ;
			WaitForSingleObject(hChopstickMutex[0], INFINITE) ;
			g_nChopstickState[nLeftChopstick] = nPhilosopher ;

			Sleep(DINING_DELAY) ;
			g_nDinerState[nPhilosopher] = WAITING ;
			SendMessage(hWndMain, WM_FORCE_REPAINT, NULL, NULL) ;
			WaitForSingleObject(hChopstickMutex[1], INFINITE) ;
			g_nChopstickState[nRightChopstick] = nPhilosopher ;
		}

		g_nDinerState[nPhilosopher] = EATING ;
		SendMessage(hWndMain, WM_FORCE_REPAINT, NULL, NULL) ;

		Sleep(DINING_DELAY) ;
		g_nDinerState[nPhilosopher] = THINKING;
		g_nChopstickState[nLeftChopstick] = UNUSED ;
		g_nChopstickState[nRightChopstick] = UNUSED ;
		SendMessage(hWndMain, WM_FORCE_REPAINT, NULL, NULL) ;
		ReleaseMutex(hChopstickMutex[0]) ;
		ReleaseMutex(hChopstickMutex[1]) ;

		Sleep(DINING_DELAY) ;
	}

	return 0 ;
}

void Dining()
{
	for (int i = 0 ; i < PHILOSOPHERS ; i++)
	{
		g_hThrd[i] = CreateThread(NULL, 0, DiningThread, (LPVOID) i, NULL, NULL) ;
	}
}
