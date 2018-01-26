#ifndef  __ROTATE_CU_
#define  __ROTATE_CU_

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <time.h>
#include <iostream>
#define datasize 100

__device__ inline float BOUND(float val, float min, float max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

__device__ inline float Sinxx(float value) {
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

inline void checkCudaErrors(cudaError err) //cuda error handle function
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA Runtime API error:%s.\n", cudaGetErrorString(err));
		return;
	}
}

__global__ void Rotate(int *In, int *Out, int Width, int Height,int nWidth,int nHeight, double rotate_arc)
{
	
	double sinval = sin(rotate_arc);
	double cosval = cos(rotate_arc);
	double num1 = -0.5* nWidth *cosval - 0.5* nHeight *sinval + 0.5* Width;
	double num2 = 0.5*nWidth* sinval - 0.5 * nHeight *cosval + 0.5 * Height;

	int y = blockDim.y * blockIdx.y + threadIdx.y;
	int x = blockDim.x * blockIdx.x + threadIdx.x;
	if (x <= nWidth && x >= 0 && y <= nHeight && y >= 0)
	{
		float fx = x * cosval + y * sinval + num1;
		float fy = -x * sinval + y * cosval + num2;
		int rgb;
		if (fx < 0 || fx >= Width || fy < 0 || fy >= Height)
		{
			rgb = 1;
		}
		else if (int(fx) == fx && int(fy) == fy) //可在src img找到对应像素点
		{
			rgb = In[int(fy)*Width + int(fx)];
		}
		else
		{
			//CubicInterpolation(In, fx, fy, Width, Height,rgb);
			int X, Y;
			X = floor(fx);
			Y = floor(fy);

			int xx[4], yy[4]; //邻域坐标单位距离
			xx[0] = -1;  xx[1] = 0; xx[2] = 1; xx[3] = 2;
			yy[0] = -1;  yy[1] = 0; yy[2] = 1; yy[3] = 2;
			//保证合法
			if ((X - 1) < 0) xx[0] = 0;
			if ((X + 1) > (Width - 1)) xx[2] = 0;
			if ((X + 2) > (Width - 1)) xx[3] = ((xx[2] == 0) ? 0 : 1);

			if ((Y - 1) < 0) yy[0] = 0;
			if ((Y + 1) > (Height - 1)) yy[2] = 0;
			if ((Y + 2) > (Height - 1)) yy[3] = ((yy[2] == 0) ? 0 : 1);

			//相邻像素的像素值
			int aby[4][4];
			for (int i = 0; i < 4; i++)
			{
				int pbySrcBase = Y + yy[i];
				for (int j = 0; j < 4; j++)
				{
					int pbySrc = pbySrcBase*Width + (X + xx[j]);
					aby[i][j] = In[pbySrc];
				}
			}

			float u, v;
			u = fx - X;
			v = fy - Y;
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
			float af[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					af[i] += afv[j] * aby[j][i];
				}
			}
			rgb = (int)(BOUND((afu[0] * af[0] + afu[1] * af[1] + afu[2] * af[2] +
				afu[3] * af[3]), 0, 255));
		}
		Out[y* nWidth + x] = rgb;
	}
}

extern "C" void Rotate_host(int *pixel, int *npixel, int Width, int Height, int nWidth, int nHeight, double rotate_arc)
{
	int *pixelIn, *pixelOut;
	dim3 dimBlock(32, 32);
	dim3 dimGrid((nWidth + dimBlock.x - 1) / dimBlock.x, (nHeight + dimBlock.y -
		1) / dimBlock.y);
	checkCudaErrors(cudaMalloc((void**)&pixelIn, sizeof(int) * Width * Height));
	checkCudaErrors(cudaMalloc((void**)&pixelOut, sizeof(int) * nWidth * nHeight));

	checkCudaErrors(cudaMemcpy(pixelIn, pixel, sizeof(int) * Width * Height, cudaMemcpyHostToDevice));

	Rotate << <dimGrid, dimBlock >> > (pixelIn, pixelOut, Width,Height,nWidth, nHeight, rotate_arc);

	checkCudaErrors(cudaMemcpy(npixel, pixelOut, sizeof(int) * nWidth * nHeight, cudaMemcpyDeviceToHost));


	cudaFree(pixelIn);
	cudaFree(pixelOut);
}

#endif // ! __ZOOM_KERNEL_CU_