#pragma once
#pragma once
#define NOISE 0.2
struct ThreadParam
{
	CImage * src;
	CImage * dst;
	int startIndex;
	int endIndex;
	int maxSpan;//为模板中心到边缘的距离
	double *rgb;
	double k;
	double angel;
	double alpha;
	union
	{
		double scale_rate;
		double rotate_arc;
	}u;
	byte*  newmin;
	byte*  newmax;
	byte*  oldmin;
	byte*  oldmax;
};

static bool GetValue(int p[], int size, int &value);
void Histogram(CImage *img, long hist[3][256]);
void getBounderValue(long hist[3][256], byte* newmin, byte* newmax, byte* oldmin, byte* oldmax, double lowCut, double hightCut, long PixelAmount);

class ImageProcess
{
public:
	static UINT medianFilter(LPVOID  param);
	static UINT addNoise(LPVOID param);
	static UINT rotate(LPVOID  p);
	static UINT zoomUp(LPVOID  p);
	static UINT autolevels(LPVOID p);
	static UINT autoWBalance(LPVOID p);
	static UINT mix(CImage* c1, CImage* c2, double alpha);
	static UINT bilateralFilter(CImage* c1);
};