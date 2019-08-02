/*
 * Author：冯泽明
 * Date：2015/12/29
 * Description：常量定义以及函数声明
 */
#pragma once

#include "resource.h"
#include "math.h"

#define PHILOSOPHERS 5
#define ANGLEPERRADIAN 57.295779513082320876798154814105

#define UNUSED  -1
#define THINKING 0
#define WAITING 1
#define EATING  2

#define WM_FORCE_REPAINT WM_APP + 1 

#define DINING_DELAY     g_bFastDining ? (rand() / 25) : ((rand() % 5 + 1) * 1000)

#define DINING_THINKING   (rand() % 6 + 1) * 1000 
#define DINING_EATING    (rand() % 5 + 1) * 1000

void RenderFrame(HDC hdc) ;
void Dining() ;
