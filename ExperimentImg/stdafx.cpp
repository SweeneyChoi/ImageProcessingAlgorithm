
// stdafx.cpp : 只包括标准包含文件的源文件
// ExperimentImg.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"


// 自定义的线程间通信消息
#ifndef WM_MEDIAN_FILTER
#define WM_MEDIAN_FILTER WM_USER+1
#endif

#ifndef WM_NOISE
#define WM_NOISE WM_USER+2
#endif