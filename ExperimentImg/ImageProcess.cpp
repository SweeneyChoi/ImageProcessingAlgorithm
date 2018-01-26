#include "stdafx.h"
#include "ImageProcess.h"
#include <vector>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include <math.h>
#include <string.h>
#include <opencv/cv.h>
#include <stdio.h>
using namespace cv;

static bool GetValue(int p[], int size, int &value)
{
	//数组中间的值
	int zxy = p[(size - 1) / 2];
	//用于记录原数组的下标
	int *a = new int[size];
	int index = 0;
	for (int i = 0; i<size; ++i)
		a[index++] = i;

	for (int i = 0; i<size - 1; i++)
		for (int j = i + 1; j<size; j++)
			if (p[i]>p[j]) {
				int tempA = a[i];
				a[i] = a[j];
				a[j] = tempA;
				int temp = p[i];
				p[i] = p[j];
				p[j] = temp;

			}
	int zmax = p[size - 1];
	int zmin = p[0];
	int zmed = p[(size - 1) / 2];

	if (zmax>zmed&&zmin<zmed) {
		if (zxy>zmin&&zxy<zmax)
			value = (size - 1) / 2;
		else
			value = a[(size - 1) / 2];
		delete[]a;
		return true;
	}
	else {
		delete[]a;
		return false;
	}

}

void MatToCImage(cv::Mat& mat, CImage* cimage)
{
	if (0 == mat.total())
	{
		return;
	}


	int nChannels = mat.channels();
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = mat.cols;
	int nHeight = mat.rows;


	//重建cimage  
	cimage->Destroy();
	cimage->Create(nWidth, nHeight, 8 * nChannels);


	//拷贝数据  


	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage->GetBits();     //指向数据区的指针  
	int nStep = cimage->GetPitch();                  //每行的字节数,注意这个返回值有正有负  


	if (1 == nChannels)                             //对于单通道的图像需要初始化调色板  
	{
		RGBQUAD* rgbquadColorTable;
		int nMaxColors = 256;
		rgbquadColorTable = new RGBQUAD[nMaxColors];
		cimage->GetColorTable(0, nMaxColors, rgbquadColorTable);
		for (int nColor = 0; nColor < nMaxColors; nColor++)
		{
			rgbquadColorTable[nColor].rgbBlue = (uchar)nColor;
			rgbquadColorTable[nColor].rgbGreen = (uchar)nColor;
			rgbquadColorTable[nColor].rgbRed = (uchar)nColor;
		}
		cimage->SetColorTable(0, nMaxColors, rgbquadColorTable);
		delete[]rgbquadColorTable;
	}


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				*(pucImage + nRow * nStep + nCol) = pucRow[nCol];
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					*(pucImage + nRow * nStep + nCol * 3 + nCha) = pucRow[nCol * 3 + nCha];
				}
			}
		}
	}
}
void CImageToMat(cv::Mat& mat, CImage* cimage)
{
	if (true == cimage->IsNull())
	{
		return;
	}


	int nChannels = cimage->GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = cimage->GetWidth();
	int nHeight = cimage->GetHeight();


	//重建mat  
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}


	//拷贝数据  


	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage->GetBits();     //指向数据区的指针  
	int nStep = cimage->GetPitch();                  //每行的字节数,注意这个返回值有正有负  


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					pucRow[nCol * 3 + nCha] = *(pucImage + nRow * nStep + nCol * 3 + nCha);
				}
			}
		}
	}
}

float BOUND(float val, float min, float max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

float Sinxx(float value) {
	if (value < 0) value = -value;

	if (value < 1.0) {
		float temp = value * value;
		return 0.5 * temp * value - temp + 2.0 / 3.0;
	}
	else if (value < 2.0) {
		value = 2.0 - value;
		value *= value * value;
		return value / 6.0;
	}
	else {
		return 0.0;
	}
}

inline void CubicInterpolation(CImage* sourceImage, float dblXX, float dblYY, int bitCount, byte rgb[3])
{
	byte* srcData = (byte*)sourceImage->GetBits();

	int dwWidthBytes = sourceImage->GetPitch();

	int X, Y;
	X = floor(dblXX);
	Y = floor(dblYY);

	int xx[4], yy[4]; //邻域坐标单位距离
	xx[0] = -1;  xx[1] = 0; xx[2] = 1; xx[3] = 2;
	yy[0] = -1;  yy[1] = 0; yy[2] = 1; yy[3] = 2;
	//保证合法
	if ((X - 1) < 0) xx[0] = 0;
	if ((X + 1) > (sourceImage->GetWidth() - 1)) xx[2] = 0;
	if ((X + 2) > (sourceImage->GetWidth() - 1)) xx[3] = ((xx[2] == 0) ? 0 : 1);

	if ((Y - 1) < 0) yy[0] = 0;
	if ((Y + 1) > (sourceImage->GetHeight() - 1)) yy[2] = 0;
	if ((Y + 2) > (sourceImage->GetHeight() - 1)) yy[3] = ((yy[2] == 0) ? 0 : 1);

	//相邻像素的像素值
	BYTE abyRed[4][4], abyGreen[4][4], abyBlue[4][4];
	for (int i = 0; i < 4; i++)
	{
		BYTE* pbySrcBase = srcData + (Y + yy[i]) * dwWidthBytes;
		for (int j = 0; j < 4; j++)
		{
			BYTE* pbySrc = pbySrcBase + (X + xx[j])*bitCount;
			abyBlue[i][j] = *pbySrc++;
			abyGreen[i][j] = *pbySrc++;
			abyRed[i][j] = *pbySrc;
		}
	}

	float u, v;
	u = dblXX - X;
	v = dblYY - Y;
	//领域值的权重
	float afu[4];
	float afv[4];
	afu[0] = Sinxx(1.0f + u);
	afu[1] = Sinxx(u);
	afu[2] = Sinxx(1.0f - u);
	afu[3] = Sinxx(2.0f - u);

	afv[0] = Sinxx(1.0f + v);
	afv[1] = Sinxx(v);
	afv[2] = Sinxx(1.0f - v);
	afv[3] = Sinxx(2.0f - v);

	//矩阵乘向量的中间值
	float afRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float afGreen[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float afBlue[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			afRed[i] += afv[j] * abyRed[j][i];
			afGreen[i] += afv[j] * abyGreen[j][i];
			afBlue[i] += afv[j] * abyBlue[j][i];
		}
	}
	rgb[0] = (BYTE)(BOUND((afu[0] * afRed[0] + afu[1] * afRed[1] + afu[2] * afRed[2] +
		afu[3] * afRed[3]), 0, 255));
	rgb[1] = (BYTE)(BOUND((afu[0] * afGreen[0] + afu[1] * afGreen[1] + afu[2] * afGreen[2] +
		afu[3] * afGreen[3]), 0, 255));
	rgb[2] = (BYTE)(BOUND((afu[0] * afBlue[0] + afu[1] * afBlue[1] + afu[2] * afBlue[2] +
		afu[3] * afBlue[3]), 0, 255));
}

void mulTemplate(CImage* sourceImage, CImage* dstImage, int X, int Y, int bitCount, double matrix[3][3])
{
	int dwWidthBytes = sourceImage->GetPitch();

	byte* srcData = (byte*)sourceImage->GetBits();
	byte* dstData = (byte*)dstImage->GetBits();
	int xx[3], yy[3]; //邻域坐标单位距离
	xx[0] = -1;  xx[1] = 0; xx[2] = 1;
	yy[0] = -1;  yy[1] = 0; yy[2] = 1;
	//保证合法
	if ((X - 1) < 0) xx[0] = 0;
	if ((X + 1) > (sourceImage->GetWidth() - 1)) xx[2] = 0;

	if ((Y - 1) < 0) yy[0] = 0;
	if ((Y + 1) > (sourceImage->GetHeight() - 1)) yy[2] = 0;

	//像素值
	BYTE   pixels[3][3];
	for (int i = 0; i < 3; i++)
	{
		BYTE* pbySrcBase = srcData + (Y + yy[i]) * dwWidthBytes;
		for (int j = 0; j < 3; j++)
		{
			BYTE* pbySrc = pbySrcBase + (X + xx[j])*bitCount;
			pixels[i][j] = *pbySrc;
		}
	}

	BYTE   pixel = 0;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			pixel += (pixels[i][j] * matrix[i][j]);
		}
	}
	if (pixel > 255)
	{
		pixel = 255;
	}
	else if (pixel < 0)
	{
		pixel = 0;
	}
	*(dstData + Y * dwWidthBytes + X * bitCount) =
		*(dstData + Y * dwWidthBytes + X * bitCount + 1) =
		*(dstData + Y * dwWidthBytes + X * bitCount + 2) = pixel;
}

void Histogram(CImage *img, long hist[3][256])
{
	long size = img->GetHeight()* img->GetWidth();
	int maxWidth = img->GetWidth();
	int pit = img->GetPitch();
	int bitCount = img->GetBPP() / 8;
	byte* data = (byte*)img->GetBits();
	for (int i = 0; i < size; ++i)
	{
		int X = i%maxWidth;
		int Y = i / maxWidth;
		hist[0][data[Y * pit + X * bitCount]]++;
		hist[1][data[Y * pit + X * bitCount + 1]]++;
		hist[2][data[Y * pit + X * bitCount + 2]]++;
	}
}

void getBounderValue(long hist[3][256], byte *newmin, byte *newmax, byte *oldmin, byte *oldmax, double lowCut, double hightCut, long PixelAmount)
{
	long lowAmount = PixelAmount * lowCut;
	long hightAmount = PixelAmount * hightCut;
	long sum = 0;
	for (size_t i = 0; i < 3; ++i)
	{
		sum = 0;
		for (size_t j = 0; j < 256; ++j)
		{
			sum += hist[i][j];
			if (sum > lowAmount)
			{
				newmin[i] = j;
				break;
			}
		}
		sum = 0;
		for (size_t j = 0; j < 256; ++j)
		{
			sum += hist[i][j];
			if (sum != 0)
			{
				oldmin[i] = j;
				break;
			}
		}
		sum = 0;
		for (size_t j = 255; j >= 0; --j)
		{
			sum += hist[i][j];
			if (sum > hightAmount)
			{
				newmax[i] = j;
				break;
			}
		}
		sum = 0;
		for (size_t j = 255; j >= 0; --j)
		{
			sum += hist[i][j];
			if (sum != 0)
			{
				oldmax[i] = j;
				break;
			}
		}
	}
}

UINT ImageProcess::medianFilter(LPVOID  p)
{
	
	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int maxSpan = param->maxSpan;
	int maxLength = (maxSpan * 2 + 1) * (maxSpan * 2 + 1);

	byte* pRealData = (byte*)param->src->GetBits();
	int pit = param->src->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	int *pixel = new int[maxLength];//存储每个像素点的灰度
	int *pixelR = new int[maxLength];
	int *pixelB = new int[maxLength];
	int *pixelG = new int[maxLength];
	int index = 0;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int Sxy = 1;
		int med = 0;
		int state = 0;
		int x = i % maxWidth;
		int y = i / maxWidth;
		while (Sxy <= maxSpan)
		{
			index = 0;
			for (int tmpY = y - Sxy; tmpY <= y + Sxy && tmpY <maxHeight; tmpY++)
			{
				if (tmpY < 0) continue;
				for (int tmpX = x - Sxy; tmpX <= x + Sxy && tmpX<maxWidth; tmpX++)
				{
					if (tmpX < 0) continue;
					if (bitCount == 1)
					{
						pixel[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixelR[index++] = pixel[index];

					}
					else
					{
						pixelR[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 2);
						pixelG[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 1);
						pixelB[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);

					}
				}

			}
			if (index <= 0)
				break;
			if ((state = GetValue(pixel, index, med)) == 1)
				break;

			Sxy++;
		};

		if (state)
		{
			if (bitCount == 1)
			{
				*(pRealData + pit*y + x*bitCount) = pixelR[med];

			}
			else
			{
				*(pRealData + pit*y + x*bitCount + 2) = pixelR[med];
				*(pRealData + pit*y + x*bitCount + 1) = pixelG[med];
				*(pRealData + pit*y + x*bitCount) = pixelB[med];

			}
		}

	}



	delete[]pixel;
	delete[]pixelR;
	delete[]pixelG;
	delete[]pixelB;

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_MEDIAN_FILTER, 1, NULL);
	return 0;
}


UINT ImageProcess::addNoise(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		if ((rand() % 1000) * 0.001 < NOISE)
		{
			int value = 0;
			if (rand() % 1000 < 500)
			{
				value = 0;
			}
			else
			{
				value = 255;
			}
			if (bitCount == 1)
			{
				*(pRealData + pit * y + x * bitCount) = value;
			}
			else
			{
				*(pRealData + pit * y + x * bitCount) = value;
				*(pRealData + pit * y + x * bitCount + 1) = value;
				*(pRealData + pit * y + x * bitCount + 2) = value;
			}
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}


UINT ImageProcess::zoomUp(LPVOID  p) {

	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->dst->GetWidth();
	int maxHeight = param->dst->GetHeight();
	double scaleRate = param->u.scale_rate;   //实际缩放率的倒数   dst img -> src img
	int dst_startIndex = param->startIndex;     //dst img对应的位置
	int dst_endIndex = param->endIndex;

	byte* srcData = (byte*)param->src->GetBits();
	byte* dstData = (byte*)param->dst->GetBits();
	int src_pit = param->src->GetPitch();
	int dst_pit = param->dst->GetPitch();
	int bitCount = param->src->GetBPP() / 8;


	for (int i = dst_startIndex; i <= dst_endIndex; ++i)
	{
		int X = i % maxWidth;  //dst img对应像素点坐标
		int Y = i / maxWidth;
		float fx = X * scaleRate; //src img对应像素点坐标
		float fy = Y * scaleRate;
		byte rgb[3];
		if (int(fx) == fx && int(fy) == fy) //可在src img找到对应像素点
		{
			rgb[0] = *(srcData + src_pit*(int(fy)) + (int(fx)) *bitCount + 2);
			rgb[1] = *(srcData + src_pit*(int(fy)) + (int(fx)) *bitCount + 1);
			rgb[2] = *(srcData + src_pit*(int(fy)) + (int(fx)) *bitCount);
		}
		else
		{
			CubicInterpolation(param->src, fx, fy, bitCount, rgb);
		}
		*(dstData + dst_pit*Y + X*bitCount + 2) = rgb[0];
		*(dstData + dst_pit*Y + X*bitCount + 1) = rgb[1];
		*(dstData + dst_pit*Y + X*bitCount) = rgb[2];
	}



		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_ZOOM, 1, NULL);
		return 0;
	
	}


UINT ImageProcess::rotate(LPVOID  p) {

	ThreadParam* param = (ThreadParam*)p;

	int src_width = param->src->GetWidth();
	int src_height = param->src->GetHeight();
	int dst_width = param->dst->GetWidth();
	int dst_height = param->dst->GetHeight();
	double rotate_arc = param->u.rotate_arc;
	double sinval = sin(rotate_arc);
	double cosval = cos(rotate_arc);

	int dst_startIndex = param->startIndex;     //dst img对应的位置
	int dst_endIndex = param->endIndex;

	byte* srcData = (byte*)param->src->GetBits();
	byte* dstData = (byte*)param->dst->GetBits();
	int src_pit = param->src->GetPitch();
	int dst_pit = param->dst->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	double num1 = -0.5* dst_width *cosval - 0.5* dst_height *sinval + 0.5* src_width;
	double num2 = 0.5*dst_width* sinval - 0.5 * dst_height *cosval + 0.5 * src_height;

	for (int i = dst_startIndex; i <= dst_endIndex; ++i)
	{
		int X = i % dst_width;
		int Y = i / dst_width;
		float fx = X * cosval + Y * sinval + num1;
		float fy = -X * sinval + Y * cosval + num2;
		byte rgb[3];

		if (fx < 0 || fx >= src_width || fy < 0 || fy >= src_height)
		{
			rgb[0] = rgb[1] = rgb[2] = 1;
		}
		else if (int(fx) == fx && int(fy) == fy)
		{
			rgb[0] = *(srcData + src_pit*(int(fy)) + (int(fx)) *bitCount + 2);
			rgb[1] = *(srcData + src_pit*(int(fy)) + (int(fx)) *bitCount + 1);
			rgb[2] = *(srcData + src_pit*(int(fy)) + (int(fx)) *bitCount);
		}
		else
		{
			CubicInterpolation(param->src, fx, fy, bitCount, rgb);
		}
		*(dstData + dst_pit*Y + X*bitCount + 2) = rgb[0];
		*(dstData + dst_pit*Y + X*bitCount + 1) = rgb[1];
		*(dstData + dst_pit*Y + X*bitCount) = rgb[2];
	}



	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_ROTATE, 1, NULL);
	return 0;

}


UINT ImageProcess::autolevels(LPVOID p) {
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();

	int src_startIndex = param->startIndex;     //起始位置
	int src_endIndex = param->endIndex;

	byte* srcData = (byte*)param->src->GetBits();
	byte* dstData = (byte*)param->dst->GetBits();

	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	byte* newmin = param->newmin;
	byte* newmax = param->newmax;
	byte* oldmin = param->oldmin;
	byte* oldmax = param->oldmax;

	for (int i = src_startIndex; i <= src_endIndex; ++i)
	{
		byte color;

		int X = i % maxWidth;  //dst img对应像素点坐标
		int Y = i / maxWidth;

		for (int j = 0; j < 3; j++)
		{
			color = *(srcData + Y* pit + X * bitCount + j);

			if (color < newmin[j]) color = 0;
			else if (color > newmax[j]) color = 255;
			else color = (color - oldmin[j]) * (newmax[j] - newmin[j]) / (oldmax[j] - oldmin[j]) + newmin[j];
			*(srcData + Y* pit + X * bitCount + j) = color;
		}
	}

	
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_AUTO_LEVELS, 1, NULL);
	return 0;
}


UINT ImageProcess::autoWBalance(LPVOID p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	byte* pRealData = (byte*)param->src->GetBits();
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();


	//取得对应位置指针
	auto getCor = [&](byte* &cor, int &index) {
		int x = index % maxWidth,
			y = index / maxWidth;
		cor = pRealData + y * pit + x * bitCount;
	};
	//设置颜色值
	auto setCol = [&](byte* cor) {
		if (bitCount == 1) {

			*cor = param->rgb[0] * (*cor);

		}
		else {
			for (int i = 0; i < 3; i++) {
				*(cor + i) = param->rgb[i] * (*(cor + i));
			}
		}
	};
	for (int i = startIndex; i <= endIndex; i++)
	{
		byte* cor = nullptr;
		getCor(cor, i);
		setCol(cor);

	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_AUTO_WBALANCE, 1, NULL);
	return 0;
}


UINT ImageProcess::mix(CImage* c1, CImage* c2, double alpha) {
	Mat src1,src2,dst;
	

	CImageToMat(src1, c1);
	CImageToMat(src2, c2);

	resize(src2, src2, Size(src1.cols, src1.rows));

	addWeighted(src1, alpha, src2, 1 - alpha, 0.0, dst);

	MatToCImage(dst, c1);

	return 0;

}


UINT ImageProcess::bilateralFilter(CImage* c1) {
	Mat src, dst;
	CImageToMat(src, c1);
	cv::bilateralFilter(src, dst, 25, 25 * 2, 25 / 2);
	MatToCImage(dst, c1);
	return 0;
}
