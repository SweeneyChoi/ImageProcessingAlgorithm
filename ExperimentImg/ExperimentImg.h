
// ExperimentImg.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CExperimentImgApp: 
// �йش����ʵ�֣������ ExperimentImg.cpp
//

class CExperimentImgApp : public CWinApp
{
public:
	CExperimentImgApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CExperimentImgApp theApp;