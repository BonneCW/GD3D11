// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Please direct any bugs or questions to SDKFeedback@nvidia.com

#include "../pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fft_512x512.h"

HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);


void radix008A(CSFFT512x512_Plan* fft_plan,
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV_Dst,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV_Src,
			   UINT thread_count,
			   UINT istride)
{
    // Setup execution configuration
	UINT grid = thread_count / COHERENCY_GRANULARITY;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> pd3dImmediateContext = fft_plan->pd3dImmediateContext.Get();

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cs_srvs = {pSRV_Src.Get()};
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> cs_uavs = {pUAV_Dst.Get()};
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs.GetAddressOf(), (UINT*)(cs_uavs.GetAddressOf()));

	// Shader
	if (istride > 1)
		pd3dImmediateContext->CSSetShader(fft_plan->pRadix008A_CS.Get(), nullptr, 0);
	else
		pd3dImmediateContext->CSSetShader(fft_plan->pRadix008A_CS2.Get(), nullptr, 0);

	// Execute
	pd3dImmediateContext->Dispatch(grid, 1, 1);

	// Unbind resource
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs.ReleaseAndGetAddressOf());

	cs_uavs = nullptr;
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs.GetAddressOf(), (UINT*)(cs_uavs.GetAddressOf()));
}

void fft_512x512_c2c(CSFFT512x512_Plan* fft_plan,
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV_Dst,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV_Dst,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV_Src)
{
	const UINT thread_count = fft_plan->slices * (512 * 512) / 8;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV_Tmp = fft_plan->pUAV_Tmp.Get();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV_Tmp = fft_plan->pSRV_Tmp.Get();
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> pd3dContext = fft_plan->pd3dImmediateContext.Get();
	Microsoft::WRL::ComPtr<ID3D11Buffer> cs_cbs;

	UINT istride = 512 * 512 / 8;
	cs_cbs = fft_plan->pRadix008A_CB[0].Get();
	pd3dContext->CSSetConstantBuffers(0, 1, cs_cbs.GetAddressOf());
	radix008A(fft_plan, pUAV_Tmp.Get(), pSRV_Src.Get(), thread_count, istride);

	istride /= 8;
	cs_cbs = fft_plan->pRadix008A_CB[1].Get();
	pd3dContext->CSSetConstantBuffers(0, 1, cs_cbs.GetAddressOf());
	radix008A(fft_plan, pUAV_Dst.Get(), pSRV_Tmp.Get(), thread_count, istride);

	istride /= 8;
	cs_cbs = fft_plan->pRadix008A_CB[2].Get();
	pd3dContext->CSSetConstantBuffers(0, 1, cs_cbs.GetAddressOf());
	radix008A(fft_plan, pUAV_Tmp.Get(), pSRV_Dst.Get(), thread_count, istride);

	istride /= 8;
	cs_cbs = fft_plan->pRadix008A_CB[3].Get();
	pd3dContext->CSSetConstantBuffers(0, 1, cs_cbs.GetAddressOf());
	radix008A(fft_plan, pUAV_Dst.Get(), pSRV_Tmp.Get(), thread_count, istride);

	istride /= 8;
	cs_cbs = fft_plan->pRadix008A_CB[4].Get();
	pd3dContext->CSSetConstantBuffers(0, 1, cs_cbs.GetAddressOf());
	radix008A(fft_plan, pUAV_Tmp.Get(), pSRV_Dst.Get(), thread_count, istride);

	istride /= 8;
	cs_cbs = fft_plan->pRadix008A_CB[5].Get();
	pd3dContext->CSSetConstantBuffers(0, 1, cs_cbs.GetAddressOf());
	radix008A(fft_plan, pUAV_Dst.Get(), pSRV_Tmp.Get(), thread_count, istride);
}

void create_cbuffers_512x512(CSFFT512x512_Plan* plan, Microsoft::WRL::ComPtr<ID3D11Device1> pd3dDevice, UINT slices)
{
	// Create 6 cbuffers for 512x512 transform.

	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = 32;//sizeof(float) * 5;
	cb_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cb_data;
	cb_data.SysMemPitch = 0;
	cb_data.SysMemSlicePitch = 0;

	struct CB_Structure
	{
		UINT thread_count;
		UINT ostride;
		UINT istride;
		UINT pstride;
		float phase_base;
	};

	// Buffer 0
	const UINT thread_count = slices * (512 * 512) / 8;
	UINT ostride = 512 * 512 / 8;
	UINT istride = ostride;
	double phase_base = -XM_2PI / (512.0 * 512.0);
	
	CB_Structure cb_data_buf0 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf0;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, plan->pRadix008A_CB[0].GetAddressOf());
	assert(plan->pRadix008A_CB[0].Get());

	// Buffer 1
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf1 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf1;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, plan->pRadix008A_CB[1].GetAddressOf());
	assert(plan->pRadix008A_CB[1].Get());

	// Buffer 2
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf2 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf2;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, plan->pRadix008A_CB[2].GetAddressOf());
	assert(plan->pRadix008A_CB[2].Get());

	// Buffer 3
	istride /= 8;
	phase_base *= 8.0;
	ostride /= 512;
	
	CB_Structure cb_data_buf3 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf3;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, plan->pRadix008A_CB[3].GetAddressOf());
	assert(plan->pRadix008A_CB[3].Get());

	// Buffer 4
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf4 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf4;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, plan->pRadix008A_CB[4].GetAddressOf());
	assert(plan->pRadix008A_CB[4].Get());

	// Buffer 5
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf5 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf5;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, plan->pRadix008A_CB[5].GetAddressOf());
	assert(plan->pRadix008A_CB[5].Get());
}

void fft512x512_create_plan(CSFFT512x512_Plan* plan, Microsoft::WRL::ComPtr<ID3D11Device1> pd3dDevice, UINT slices)
{
	plan->slices = slices;

	// Context
	pd3dDevice->GetImmediateContext1(plan->pd3dImmediateContext.GetAddressOf());
	assert(plan->pd3dImmediateContext.Get());

	// Compute shaders
	Microsoft::WRL::ComPtr<ID3DBlob> pBlobCS;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlobCS2;

    CompileShaderFromFile(L"CSFFT\\fft_512x512_c2c.hlsl", "Radix008A_CS", "cs_5_0", pBlobCS.GetAddressOf());
    CompileShaderFromFile(L"CSFFT\\fft_512x512_c2c.hlsl", "Radix008A_CS2", "cs_5_0", pBlobCS2.GetAddressOf());
	assert(pBlobCS.Get());
	assert(pBlobCS2.Get());

    pd3dDevice->CreateComputeShader(pBlobCS->GetBufferPointer(), pBlobCS->GetBufferSize(), nullptr, plan->pRadix008A_CS.GetAddressOf());
    pd3dDevice->CreateComputeShader(pBlobCS2->GetBufferPointer(), pBlobCS2->GetBufferSize(), nullptr, plan->pRadix008A_CS2.GetAddressOf());
	assert(plan->pRadix008A_CS.Get());
	assert(plan->pRadix008A_CS2.Get());
    
	// Constants
	// Create 6 cbuffers for 512x512 transform
	create_cbuffers_512x512(plan, pd3dDevice.Get(), slices);

	// Temp buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = sizeof(float) * 2 * (512 * slices) * 512;
    buf_desc.Usage = D3D11_USAGE_DEFAULT;
    buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    buf_desc.CPUAccessFlags = 0;
    buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    buf_desc.StructureByteStride = sizeof(float) * 2;

	pd3dDevice->CreateBuffer(&buf_desc, nullptr, plan->pBuffer_Tmp.GetAddressOf());
	assert(plan->pBuffer_Tmp.Get());

	// Temp undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = (512 * slices) * 512;
	uav_desc.Buffer.Flags = 0;

	pd3dDevice->CreateUnorderedAccessView(plan->pBuffer_Tmp.Get(), &uav_desc, plan->pUAV_Tmp.GetAddressOf());
	assert(plan->pUAV_Tmp.Get());

	// Temp shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = (512 * slices) * 512;

	pd3dDevice->CreateShaderResourceView(plan->pBuffer_Tmp.Get(), &srv_desc, plan->pSRV_Tmp.GetAddressOf());
	assert(plan->pSRV_Tmp.Get());
}

void fft512x512_destroy_plan(CSFFT512x512_Plan* plan)
{
}