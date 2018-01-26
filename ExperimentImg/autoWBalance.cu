#ifndef  __AUTOWBALANCE_CU_
#define  __AUTOWBALANCE_CU_

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <time.h>
#include <iostream>
#define datasize 100

inline void checkCudaErrors(cudaError err) //cuda error handle function
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA Runtime API error:%s.\n", cudaGetErrorString(err));
		return;
	}
}

__global__ void AutoWBalance(int *In, int *Out, double rgb, int Width, int Height)
{
	//int window[9];
	int y = blockDim.y * blockIdx.y + threadIdx.y;
	int x = blockDim.x * blockIdx.x + threadIdx.x;
	if (x <= Width && x >= 0 && y <= Height && y >= 0)
	{

		Out[y* Width + x] = In[y* Width + x]*rgb;
	}
}

extern "C" void AutoWBalance_host(int *pixel,double rgb, int Width, int Height)
{
	int *pixelIn, *pixelOut;
	dim3 dimBlock(32, 32);
	dim3 dimGrid((Width + dimBlock.x - 1) / dimBlock.x, (Height + dimBlock.y -
		1) / dimBlock.y);
	checkCudaErrors(cudaMalloc((void**)&pixelIn, sizeof(int) * Width * Height));
	checkCudaErrors(cudaMalloc((void**)&pixelOut, sizeof(int) * Width * Height));

	checkCudaErrors(cudaMemcpy(pixelIn, pixel, sizeof(int) * Width * Height, cudaMemcpyHostToDevice));

	AutoWBalance << <dimGrid, dimBlock >> > (pixelIn, pixelOut,rgb, Width, Height);

	checkCudaErrors(cudaMemcpy(pixel, pixelOut, sizeof(int) * Width * Height, cudaMemcpyDeviceToHost));


	cudaFree(pixelIn);
	cudaFree(pixelOut);
}

#endif // ! __AUTOWBALANCE_KERNEL_CU_