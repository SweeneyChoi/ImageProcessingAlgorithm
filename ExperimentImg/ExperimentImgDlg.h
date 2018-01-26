
// ExperimentImgDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ImageProcess.h"


#define MAX_THREAD 8
#define MAX_SPAN 15
struct DrawPara
{
	CImage* pImgSrc;
	CDC* pDC;
	int oriX;
	int oriY;
	int width;
	int height;
};

// CExperimentImgDlg 对话框
class CExperimentImgDlg : public CDialogEx
{
// 构造
public:
	CExperimentImgDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENTIMG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CImage* getImage() { return m_pImgSrc; }
	void MedianFilter();
	void AddNoise();
	void AddNoise_WIN();
	void AutoLevels();
	void AutoLevels_WIN();
	void Rotate();
	void Rotate_WIN();
	void ZoomUp();
	void ZoomUp_WIN();
	void AutoWBalance();
	void AutoWBalance_WIN();
	void BilateralFilter();
	void ThreadDraw(DrawPara *p);
	static UINT Update(void* p);
	void ImageCopy(CImage* pImgSrc, CImage* pImgDrt);
	void MedianFilter_WIN();
	afx_msg LRESULT OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnAutoLevelsThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRotateThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnZoomThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoWBalanceThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	

// 实现
protected:
	HICON m_hIcon;
	CImage * m_pImgSrc;
	CImage * m_pImgSrc2;
	CImage * m_pImgCpy;
	//CImage *dst;
	double scale_rate = 2;
	int rotate_degree = 45;
	int m_nThreadNum;
	ThreadParam* m_pThreadParam;
	CTime startTime;
	double avg[3] = { 0.0,0.0,0.0 };
	double sum = 0, rgb[3] = { 0.0,0.0,0.0 };
	double alpha = 0;
//	ThreadParam * m_pThreadParam;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	CEdit mEditInfo;
	CStatic mPictureControl;
	afx_msg void OnCbnSelchangeComboFunction();
	afx_msg void OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonProcess();
	CButton m_CheckCirculation;
	CStatic mPictureControl2;
	CStatic mOutput;
	
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult);
};
