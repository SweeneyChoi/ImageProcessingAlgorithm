
// ExperimentImgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "afxdialogex.h"
#include <iostream>
using namespace std;
#define M_PI       3.14159265358979323846
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" void MedianFilter_host(int *pixel, int Width, int Height);
extern "C" void AutoWBalance_host(int *pixel,double rgb, int Width, int Height);
extern "C" void Zoom_host(int *pixel, int *npixel, int Width, int Height, int nWidth, int nHeight, double scaleRate);
extern "C" void Rotate_host(int *pixel, int *npixel, int Width, int Height, int nWidth, int nHeight, double rotate_arc);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CExperimentImgDlg 对话框



CExperimentImgDlg::CExperimentImgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENTIMG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//加载对话框的时候初始化
	m_pImgSrc = NULL;
	//m_pImgCpy = NULL;
	m_nThreadNum = 1;
	m_pThreadParam = new ThreadParam[MAX_THREAD];
	srand(time(0));
}

void CExperimentImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT_INFO, mEditInfo);
	DDX_Control(pDX, IDC_PICTURE, mPictureControl);
	DDX_Control(pDX, IDC_PICTURE2, mPictureControl2);
	DDX_Control(pDX, IDC_CHECK_100, m_CheckCirculation);
	DDX_Control(pDX, IDC_OUTPUT, mOutput);
}

BEGIN_MESSAGE_MAP(CExperimentImgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CExperimentImgDlg::OnBnClickedButtonOpen)
//	ON_EN_CHANGE(IDC_EDIT1, &CExperimentImgDlg::OnEnChangeEdit1)
//	ON_EN_CHANGE(IDC_EDIT_INFO, &CExperimentImgDlg::OnEnChangeEditInfo)
ON_CBN_SELCHANGE(IDC_COMBO_FUNCTION, &CExperimentImgDlg::OnCbnSelchangeComboFunction)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_THREADNUM, &CExperimentImgDlg::OnNMCustomdrawSliderThreadnum)
ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CExperimentImgDlg::OnBnClickedButtonProcess)
ON_MESSAGE(WM_NOISE, &CExperimentImgDlg::OnNoiseThreadMsgReceived)
ON_MESSAGE(WM_MEDIAN_FILTER, &CExperimentImgDlg::OnMedianFilterThreadMsgReceived)
ON_MESSAGE(WM_AUTO_LEVELS,&CExperimentImgDlg::OnAutoLevelsThreadMsgReceived)
ON_MESSAGE(WM_ROTATE, &CExperimentImgDlg::OnRotateThreadMsgReceived)
ON_MESSAGE(WM_ZOOM, &CExperimentImgDlg::OnZoomThreadMsgReceived)
ON_MESSAGE(WM_AUTO_WBALANCE, &CExperimentImgDlg::OnAutoWBalanceThreadMsgReceived)
ON_BN_CLICKED(IDC_BUTTON1, &CExperimentImgDlg::OnBnClickedButton1)
ON_BN_CLICKED(IDC_BUTTON2, &CExperimentImgDlg::OnBnClickedButton2)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, &CExperimentImgDlg::OnNMCustomdrawSlider3)
END_MESSAGE_MAP()


// CExperimentImgDlg 消息处理程序

BOOL CExperimentImgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
//	mEditInfo.SetWindowTextW(CString("File Path"));
	CComboBox * cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	cmb_function->AddString(_T("椒盐噪声"));
	cmb_function->AddString(_T("中值滤波"));
	cmb_function->AddString(_T("自动色阶"));
	cmb_function->AddString(_T("旋转"));
	cmb_function->AddString(_T("缩放"));
	cmb_function->AddString(_T("自动白平衡"));
	cmb_function->AddString(_T("双边滤波"));
	cmb_function->SetCurSel(0);

	CComboBox * cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	cmb_thread->InsertString(0, _T("OpenMP"));
	cmb_thread->InsertString(1, _T("WIN多线程"));
	cmb_thread->InsertString(2, _T("CUDA"));
	//cmb_thread->InsertString(3, _T("Boost"));
	cmb_thread->SetCurSel(0);

	CSliderCtrl * slider = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM));
	slider->SetRange(1, MAX_THREAD, TRUE);
	slider->SetPos(MAX_THREAD);

	CSliderCtrl * slider2 = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER3));
	slider2->SetRange(0, 100, TRUE);
	slider2->SetPos(100);

	AfxBeginThread((AFX_THREADPROC)&CExperimentImgDlg::Update, this);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CExperimentImgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CExperimentImgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		if (m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = m_pImgSrc->GetHeight();
			width = m_pImgSrc->GetWidth();

			mPictureControl.GetClientRect(&rect);
			CDC *pDC1 = mPictureControl.GetDC();
			CDC *pDC2 = mPictureControl2.GetDC();
			SetStretchBltMode(pDC1->m_hDC, STRETCH_HALFTONE);
			SetStretchBltMode(pDC2->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Width())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				m_pImgSrc->StretchBlt(pDC1->m_hDC, rect1, SRCCOPY);
				m_pImgSrc->StretchBlt(pDC2->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
				//将picture control调整到图像缩放后的大小
//				CWnd *pWnd;
//				pWnd = GetDlgItem(IDC_PICTURE);
//				pWnd->MoveWindow(CRect((int)rect.top, (int)rect.left, (int)width*ScaleIndex, (int)height*ScaleIndex));
				m_pImgSrc->StretchBlt(pDC1->m_hDC, rect1, SRCCOPY);
				m_pImgSrc->StretchBlt(pDC2->m_hDC, rect1, SRCCOPY);
			}
			ReleaseDC(pDC1);
			ReleaseDC(pDC2);
		}
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CExperimentImgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CExperimentImgDlg::Update(void* p)
{
	while (1)
	{
		Sleep(200);
		CExperimentImgDlg* dlg = (CExperimentImgDlg*)p;
		if (dlg->m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = dlg->m_pImgSrc->GetHeight();
			width = dlg->m_pImgSrc->GetWidth();

			dlg->mPictureControl.GetClientRect(&rect);
			CDC *pDC = dlg->mPictureControl.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);
			
			
			if (width <= rect.Width() && height <= rect.Width())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
				dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			
			dlg->ReleaseDC(pDC);
		}
	}
	return 0;
}

void CExperimentImgDlg::ThreadDraw(DrawPara *p)
{
	CRect rect;
	GetClientRect(&rect);
	CDC     memDC;             // 用于缓冲绘图的内存画笔  
	CBitmap memBitmap;         // 用于缓冲绘图的内存画布
	memDC.CreateCompatibleDC(p->pDC);  // 创建与原画笔兼容的画笔
	memBitmap.CreateCompatibleBitmap(p->pDC, p->width, p->height);  // 创建与原位图兼容的内存画布
	memDC.SelectObject(&memBitmap);      // 创建画笔与画布的关联
	memDC.FillSolidRect(rect, p->pDC->GetBkColor());
	p->pDC->SetStretchBltMode(HALFTONE);
	// 将pImgSrc的内容缩放画到内存画布中
	p->pImgSrc->StretchBlt(memDC.m_hDC, 0, 0, p->width, p->height);

	// 将已画好的画布复制到真正的缓冲区中
	p->pDC->BitBlt(p->oriX, p->oriY, p->width, p->height, &memDC, 0, 0, SRCCOPY);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
}

void CExperimentImgDlg::ImageCopy(CImage* pImgSrc, CImage* pImgDrt)
{
	int MaxColors = pImgSrc->GetMaxColorTableEntries();
	RGBQUAD* ColorTab;
	ColorTab = new RGBQUAD[MaxColors];

	CDC *pDCsrc, *pDCdrc;
	if (!pImgDrt->IsNull())
	{
		pImgDrt->Destroy();
	}
	pImgDrt->Create(pImgSrc->GetWidth(), pImgSrc->GetHeight(), pImgSrc->GetBPP(), 0);

	if (pImgSrc->IsIndexed())
	{
		pImgSrc->GetColorTable(0, MaxColors, ColorTab);
		pImgDrt->SetColorTable(0, MaxColors, ColorTab);
	}

	pDCsrc = CDC::FromHandle(pImgSrc->GetDC());
	pDCdrc = CDC::FromHandle(pImgDrt->GetDC());
	pDCdrc->BitBlt(0, 0, pImgSrc->GetWidth(), pImgSrc->GetHeight(), pDCsrc, 0, 0, SRCCOPY);
	pImgSrc->ReleaseDC();
	pImgDrt->ReleaseDC();
	delete ColorTab;

}

void CExperimentImgDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files （*.*）|*.*||");
	CString filePath("");
	
	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
//		mEditInfo.SetWindowTextW(strFilePath);	//在文本框中显示图像路径

		if (m_pImgSrc != NULL)
		{
			m_pImgSrc->Destroy();
			delete m_pImgSrc;
		}
		m_pImgSrc = new CImage();
		m_pImgSrc->Load(strFilePath);

		this->Invalidate();

	}
}


void CExperimentImgDlg::OnCbnSelchangeComboFunction()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CExperimentImgDlg::OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM);
	CString text("");
	m_nThreadNum = slider->GetPos();
	text.Format(_T("%d"), m_nThreadNum);
	GetDlgItem(IDC_STATIC_THREADNUM)->SetWindowText(text);
	*pResult = 0;
}


void CExperimentImgDlg::OnBnClickedButtonProcess()
{
	// TODO: 在此添加控件通知处理程序代码
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	int func = cmb_function->GetCurSel();
	switch (func)
	{
	case 0:  //椒盐噪声
		AddNoise();
		break;
	case 1://双边滤波
		BilateralFilter();
		break;
	case 2: //缩放
		ZoomUp();
		break;
	case 3://旋转
		Rotate();
		break;
	case 4://自适应中值滤波
		MedianFilter();
		break;
	case 5://自动白平衡
		AutoWBalance();
	case 6://自动色阶
		AutoLevels();
		break;
	default:
		break;
	}
}

void CExperimentImgDlg::AddNoise()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1:100;
	startTime = CTime::GetTickCount();
	switch (thread)
	{
	case 1://win多线程
	{
		//int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
		AddNoise_WIN();
		//for (int i = 0; i < circulation; i++)
		//{
		//	for (int i = 0; i < m_nThreadNum; ++i)
		//	{
		//		m_pThreadParam[i].startIndex = i * subLength;
		//		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
		//			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		//		m_pThreadParam[i].src = m_pImgSrc;
		//		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
		//	}
		//}
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
		//AfxMessageBox(timeStr);
	}

	break;

	case 0://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

		#pragma omp parallel for num_threads(m_nThreadNum)
			for (int i = 0; i < m_nThreadNum; ++i)
			{
				m_pThreadParam[i].startIndex = i * subLength;
				m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
					(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
				m_pThreadParam[i].src = m_pImgSrc;
				ImageProcess::addNoise(&m_pThreadParam[i]);
			}
	}

	break;

	case 2://cuda
		break;
	}
}

void CExperimentImgDlg::AddNoise_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::MedianFilter()
{
//	AfxMessageBox(_T("中值滤波"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;

	startTime = CTime::GetTickCount();
	m_nThreadNum;
	switch (thread)
	{
	case 1://win多线程
	{
		MedianFilter_WIN();
	}

	break;

	case 0://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = m_pImgSrc;
			ImageProcess::medianFilter(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
	{
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			MedianFilter_host(pixel, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			MedianFilter_host(pixelR, width, height);
			MedianFilter_host(pixelG, width, height);
			MedianFilter_host(pixelB, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}
		//AfxMessageBox(_T("finish!"));
		CTime endTime = CTime::GetTickCount();
		CString timeStr;
		timeStr.Format(_T("耗时:%dms"), endTime - startTime);
		mOutput.SetWindowTextW(timeStr);
	}
		break;
	}
}

void CExperimentImgDlg::MedianFilter_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::medianFilter, &m_pThreadParam[i]);
	}
}

LRESULT CExperimentImgDlg::OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				MedianFilter_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
				//AfxMessageBox(timeStr);
			}
			// 显示消息窗口
//			AfxMessageBox(timeStr);
			mOutput.SetWindowTextW(timeStr);
		}
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
			AddNoise_WIN();
		else
		{
			tempProcessCount = 0;
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
			//AfxMessageBox(timeStr);
			mOutput.SetWindowTextW(timeStr);
		}
	//	AfxMessageBox(timeStr);
	}
	return 0;
}


void CExperimentImgDlg::AutoLevels()
{
	//	AfxMessageBox(_T("自动色阶"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;
	startTime = CTime::GetTickCount();

	long hist[3][256] = { 0 };
	Histogram(m_pImgSrc, hist);

	byte* newmin = new byte[3];
	byte* newmax = new byte[3];
	byte* oldmin = new byte[3];
	byte* oldmax = new byte[3];

	static double lowCat = 0.01;
	static double hightCut = 0.01;
	lowCat += 0.001;
	hightCut += 0.001;
	getBounderValue(hist, newmin, newmax, oldmin, oldmax, lowCat, hightCut, m_pImgSrc->GetWidth()* m_pImgSrc->GetHeight());
	switch (thread)
	{
	case 1://win多线程
	{
		AutoLevels_WIN();
	}

	break;

	case 0://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].newmin = newmin;
			m_pThreadParam[i].newmax = newmax;
			m_pThreadParam[i].oldmin = oldmin;
			m_pThreadParam[i].oldmax = oldmax;
			ImageProcess::autolevels(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
		break;
	}
}

void CExperimentImgDlg::AutoLevels_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	long hist[3][256] = { 0 };
	Histogram(m_pImgSrc, hist);

	byte* newmin = new byte[3];
	byte* newmax = new byte[3];
	byte* oldmin = new byte[3];
	byte* oldmax = new byte[3];

	static double lowCat = 0.01;
	static double hightCut = 0.01;
	lowCat += 0.001;
	hightCut += 0.001;
	getBounderValue(hist, newmin, newmax, oldmin, oldmax, lowCat, hightCut, m_pImgSrc->GetWidth()* m_pImgSrc->GetHeight());
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].newmin = newmin;
		m_pThreadParam[i].newmax = newmax;
		m_pThreadParam[i].oldmin = oldmin;
		m_pThreadParam[i].oldmax = oldmax;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::autolevels, &m_pThreadParam[i]);
	}
}

LRESULT CExperimentImgDlg::OnAutoLevelsThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				AutoLevels_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
				//AfxMessageBox(timeStr);
			}
			// 显示消息窗口
			//			AfxMessageBox(timeStr);
			mOutput.SetWindowTextW(timeStr);
		}
	}
	return 0;
}

void CExperimentImgDlg::Rotate()
{
	//	AfxMessageBox(_T("旋转"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;

	startTime = CTime::GetTickCount();
	m_nThreadNum;
	switch (thread)
	{
	case 1://win多线程
	{
		Rotate_WIN();
	}

	break;

	case 0://openmp
	{
		CImage dst;

		float sinval = sin(rotate_degree * 1.0 * M_PI * (1.0 / 180));
		float cosval = cos(rotate_degree * 1.0 * M_PI * (1.0 / 180));

		int nwidth = int(m_pImgSrc->GetWidth() * 1.0f * cosval + m_pImgSrc->GetHeight() * 1.0f * sinval + 0.5);
		int nheight = int(m_pImgSrc->GetWidth() * 1.0f * sinval + m_pImgSrc->GetHeight() * 1.0f * cosval + 0.5);

		dst.Create(nwidth, nheight, m_pImgSrc->GetBPP());

		int subLength = nwidth*nheight/ m_nThreadNum;

		
#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : dst.GetWidth() * dst.GetHeight() - 1;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].dst = &dst;
			m_pThreadParam[i].u.rotate_arc = rotate_degree * 1.0 * M_PI * (1.0 / 180);
			ImageProcess::rotate(&m_pThreadParam[i]);
		}
		CExperimentImgDlg::ImageCopy(&dst, m_pImgSrc);
	}

	break;

	case 2://cuda
	{
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();

		CImage dst;
		float sinval = sin(rotate_degree * 1.0 * M_PI * (1.0 / 180));
		float cosval = cos(rotate_degree * 1.0 * M_PI * (1.0 / 180));

		int w = int(m_pImgSrc->GetWidth() * 1.0f * cosval + m_pImgSrc->GetHeight() * 1.0f * sinval + 0.5);
		int h = int(m_pImgSrc->GetWidth() * 1.0f * sinval + m_pImgSrc->GetHeight() * 1.0f * cosval + 0.5);

		dst.Create(w, h, m_pImgSrc->GetBPP());
		byte* npRealData = (byte*)dst.GetBits();
		int npit = dst.GetPitch();	//line offset 
		int nbitCount = dst.GetBPP() / 8;
		int nLength = h*w;
		int length = height * width;
		double rotate_arc = rotate_degree * 1.0 * M_PI * (1.0 / 180);
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int *npixel = (int*)malloc(nLength * sizeof(int));
		int *npixelR = (int*)malloc(nLength * sizeof(int));
		int *npixelG = (int*)malloc(nLength * sizeof(int));
		int *npixelB = (int*)malloc(nLength * sizeof(int));
		int *npixelIndex = (int*)malloc(nLength * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			Rotate_host(pixel, npixel, width, height, w, h, rotate_arc);
			index = 0;
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					*(npRealData + npit*y + x*nbitCount) = npixel[index];
					index++;
				}
			}
		}
		else
		{
			Rotate_host(pixelR, npixelR, width, height, w, h, rotate_arc);
			Rotate_host(pixelG, npixelG, width, height, w, h, rotate_arc);
			Rotate_host(pixelB, npixelB, width, height, w, h, rotate_arc);
			index = 0;
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					*(npRealData + npit*y + x*nbitCount + 2) = npixelR[index];
					*(npRealData + npit*y + x*nbitCount + 1) = npixelG[index];
					*(npRealData + npit*y + x*nbitCount) = npixelB[index];
					index++;
				}
			}
		}

		//AfxMessageBox(_T("finish!"));
		CTime endTime = CTime::GetTickCount();
		CString timeStr;
		timeStr.Format(_T("耗时:%dms"), endTime - startTime);
		mOutput.SetWindowTextW(timeStr);
		CExperimentImgDlg::ImageCopy(&dst, m_pImgSrc);
	}
		break;
	}
}

void CExperimentImgDlg::Rotate_WIN()
{
	CImage dst;

	float sinval = sin(rotate_degree * 1.0 * M_PI * (1.0 / 180));
	float cosval = cos(rotate_degree * 1.0 * M_PI * (1.0 / 180));

	int nwidth = int(m_pImgSrc->GetWidth() * 1.0f * cosval + m_pImgSrc->GetHeight() * 1.0f * sinval + 0.5);
	int nheight = int(m_pImgSrc->GetWidth() * 1.0f * sinval + m_pImgSrc->GetHeight() * 1.0f * cosval + 0.5);

	dst.Create(nwidth, nheight, m_pImgSrc->GetBPP());

	int subLength = nwidth*nheight / m_nThreadNum;
	
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?(i + 1) * subLength - 1 : dst.GetWidth() * dst.GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].dst = &dst;
		m_pThreadParam[i].u.rotate_arc = rotate_degree * 1.0 * M_PI * (1.0 / 180);
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::rotate, &m_pThreadParam[i]);
	}
	CExperimentImgDlg::ImageCopy(&dst, m_pImgSrc);
}

LRESULT CExperimentImgDlg::OnRotateThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				Rotate_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
				//AfxMessageBox(timeStr);
			}
			// 显示消息窗口
			//			AfxMessageBox(timeStr);
			mOutput.SetWindowTextW(timeStr);
		}
	}
	return 0;
}

void CExperimentImgDlg::ZoomUp()
{
	//	AfxMessageBox(_T("缩放"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;

	startTime = CTime::GetTickCount();
	m_nThreadNum;
	switch (thread)
	{
	case 1://win多线程
	{
		ZoomUp_WIN();
	}

	break;

	case 0://openmp
	{
		CImage dst;
		int w = m_pImgSrc->GetWidth()*scale_rate;
		int h = m_pImgSrc->GetHeight()*scale_rate;
		int subLength = w*h/ m_nThreadNum;
		dst.Create(m_pImgSrc->GetWidth()*scale_rate, m_pImgSrc->GetHeight()*scale_rate, m_pImgSrc->GetBPP());
#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : w * h - 1;
			m_pThreadParam[i].src = m_pImgSrc;
			m_pThreadParam[i].dst = &dst;
			m_pThreadParam[i].u.scale_rate = 1.0f / scale_rate;
			ImageProcess::zoomUp(&m_pThreadParam[i]);
		}
		//m_pImgSrc = &dst;
		CExperimentImgDlg::ImageCopy(&dst,m_pImgSrc);
	}
	break;

	case 2://cuda
	{
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();

		CImage dst;
		int w = m_pImgSrc->GetWidth()*scale_rate;
		int h = m_pImgSrc->GetHeight()*scale_rate;
		int subLength = w*h / m_nThreadNum;
		dst.Create(m_pImgSrc->GetWidth()*scale_rate, m_pImgSrc->GetHeight()*scale_rate, m_pImgSrc->GetBPP());
		byte* npRealData = (byte*)dst.GetBits();
		int npit = dst.GetPitch();	//line offset 
		int nbitCount = dst.GetBPP() / 8;
		int nLength = h*w;
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int *npixel = (int*)malloc(nLength * sizeof(int));
		int *npixelR = (int*)malloc(nLength * sizeof(int));
		int *npixelG = (int*)malloc(nLength * sizeof(int));
		int *npixelB = (int*)malloc(nLength * sizeof(int));
		int *npixelIndex = (int*)malloc(nLength * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			Zoom_host(pixel,npixel, width, height,w,h,scale_rate);
			index = 0;
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					*(npRealData + npit*y + x*nbitCount) = npixel[index];
					index++;
				}
			}
		}
		else
		{
			Zoom_host(pixelR,npixelR, width, height,w,h,scale_rate);
			Zoom_host(pixelG,npixelG, width, height,w,h,scale_rate);
			Zoom_host(pixelB,npixelB, width, height,w,h,scale_rate);
			index = 0;
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					*(npRealData + npit*y + x*nbitCount + 2) = npixelR[index];
					*(npRealData + npit*y + x*nbitCount + 1) = npixelG[index];
					*(npRealData + npit*y + x*nbitCount) = npixelB[index];
					index++;
				}
			}
		}
		
		//AfxMessageBox(_T("finish!"));
		CTime endTime = CTime::GetTickCount();
		CString timeStr;
		timeStr.Format(_T("耗时:%dms"), endTime - startTime);
		mOutput.SetWindowTextW(timeStr);
		CExperimentImgDlg::ImageCopy(&dst, m_pImgSrc);
	}
		break;
	}
}

void CExperimentImgDlg::ZoomUp_WIN()
{
	CImage dst;
		int w = m_pImgSrc->GetWidth()*scale_rate;
		int h = m_pImgSrc->GetHeight()*scale_rate;
		int subLength = w*h/ m_nThreadNum;
		dst.Create(m_pImgSrc->GetWidth()*scale_rate, m_pImgSrc->GetHeight()*scale_rate, m_pImgSrc->GetBPP());
	
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : w * h - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		m_pThreadParam[i].dst = &dst;
		m_pThreadParam[i].u.scale_rate = 1.0f / scale_rate;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::zoomUp, &m_pThreadParam[i]);
	}
	CExperimentImgDlg::ImageCopy(&dst, m_pImgSrc);
	
}

LRESULT CExperimentImgDlg::OnZoomThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				ZoomUp_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
				//AfxMessageBox(timeStr);
			}
			// 显示消息窗口
			//			AfxMessageBox(timeStr);
			mOutput.SetWindowTextW(timeStr);
		}
	}
	return 0;
}

void CExperimentImgDlg::AutoWBalance()
{
	//	AfxMessageBox(_T("自动白平衡"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;

	startTime = CTime::GetTickCount();
	int maxWidth = m_pImgSrc->GetWidth(),
		maxHeight = m_pImgSrc->GetHeight(),

		length = m_pImgSrc->GetWidth() *  m_pImgSrc->GetHeight(),
		subLength = length / m_nThreadNum;

	byte* pRealData = (byte*)m_pImgSrc->GetBits();

	int bitCount = m_pImgSrc->GetBPP() / 8,
		pit = m_pImgSrc->GetPitch();

	startTime = CTime::GetTickCount();
	if (bitCount == 1) {
		for (int i = 0; i < length; i++) {
			int x = i % maxWidth,
				y = i / maxWidth;
			byte *cor = pRealData + y * pit + x * bitCount;
			avg[0] += *cor;
		}
	}
	else {
		for (int i = 0; i < length; i++) {
			int x = i % maxWidth,
				y = i / maxWidth;
			byte *cor = pRealData + y * pit + x * bitCount;
			for (int j = 0; j < 3; j++) {
				avg[j] += (double)*(cor + j);
			}
		}
	}

	for (int i = 0; i < 3; i++) {
		avg[i] /= length;
	}
	
	for (int i = 0; i < 3; ++i)
	{
		sum += avg[i];
	}
	double K = sum / 3;
	for (int i = 0; i < 3; i++)
	{
		rgb[i] = K / avg[i];
	}
	switch (thread)
	{
	case 1://win多线程
	{
		AutoWBalance_WIN();
		break;
	}


	case 0://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].rgb = rgb;
			m_pThreadParam[i].src = m_pImgSrc;
			ImageProcess::autoWBalance(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
	{
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			AutoWBalance_host(pixel,rgb[0],width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			AutoWBalance_host(pixelR,rgb[2],width, height);
			AutoWBalance_host(pixelG,rgb[1],width, height);
			AutoWBalance_host(pixelB,rgb[0],width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}
		//AfxMessageBox(_T("finish!"));
		CTime endTime = CTime::GetTickCount();
		CString timeStr;
		timeStr.Format(_T("耗时:%dms"), endTime - startTime);
		mOutput.SetWindowTextW(timeStr);
	}
		break;
	}
}

void CExperimentImgDlg::AutoWBalance_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].rgb = rgb;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::autoWBalance, &m_pThreadParam[i]);
	}
}

LRESULT CExperimentImgDlg::OnAutoWBalanceThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				AutoWBalance_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
				//AfxMessageBox(timeStr);
			}
			// 显示消息窗口
			//			AfxMessageBox(timeStr);
			mOutput.SetWindowTextW(timeStr);
		}
	}
	return 0;
}




void CExperimentImgDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files （*.*）|*.*||");
	CString filePath("");

	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
		//		mEditInfo.SetWindowTextW(strFilePath);	//在文本框中显示图像路径

		if (m_pImgSrc2 != NULL)
		{
			m_pImgSrc2->Destroy();
			delete m_pImgSrc;
		}
		m_pImgSrc2 = new CImage();
		m_pImgSrc2->Load(strFilePath);

		this->Invalidate();
	}
}




void CExperimentImgDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	startTime = CTime::GetTickCount();
	ImageProcess::mix(m_pImgSrc, m_pImgSrc2, alpha);
	CTime endTime = CTime::GetTickCount();
	CString timeStr;
	timeStr.Format(_T("耗时:%dms"), endTime - startTime);
	mOutput.SetWindowTextW(timeStr);

}

void CExperimentImgDlg::BilateralFilter() {
	startTime = CTime::GetTickCount();
	ImageProcess::bilateralFilter(m_pImgSrc);
	CTime endTime = CTime::GetTickCount();
	CString timeStr;
	timeStr.Format(_T("耗时:%dms"), endTime - startTime);
	mOutput.SetWindowTextW(timeStr);

}


void CExperimentImgDlg::OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CSliderCtrl *slider2 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
	CString text("");
	alpha = slider2->GetPos()/100.0;
	text.Format(_T("%f"), alpha);
	std::cout << alpha << endl;
	GetDlgItem(IDC_STATIC_ALPHA)->SetWindowText(text);
	*pResult = 0;
}
