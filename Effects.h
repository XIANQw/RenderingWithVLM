﻿//***************************************************************************************
// Effects.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 简易特效管理框架
// Simple effect management framework.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include "light.h"
#include "RenderStates.h"

class IEffect
{
public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	IEffect() = default;
	virtual ~IEffect() = default;
	// 不允许拷贝，允许移动
	IEffect(const IEffect&) = delete;
	IEffect& operator=(const IEffect&) = delete;
	IEffect(IEffect&&) = default;
	IEffect& operator=(IEffect&&) = default;

	// 更新并绑定常量缓冲区
	virtual void Apply(ID3D11DeviceContext* deviceContext) = 0;
};


class BasicEffect : public IEffect
{
public:

	BasicEffect();
	virtual ~BasicEffect() override;

	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

	// 获取单例
	static BasicEffect& Get();



	// 初始化所需资源
	bool InitAll(ID3D11Device* device);

	bool SetVSShader2D(ID3D11Device* device, const WCHAR* hlslFile);
	bool SetVSShader3D(ID3D11Device* device, const WCHAR* hlslFile);
	bool SetPSShader2D(ID3D11Device* device, const WCHAR* hlslFile);
	bool SetPSShader3D(ID3D11Device* device, const WCHAR* hlslFile);

	//
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext);
	// Alpha混合绘制
	void SetRenderAlphaBlend(ID3D11DeviceContext* deviceContext);
	// 无二次混合
	void SetRenderNoDoubleBlend(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 仅写入模板值
	void SetWriteStencilOnly(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 对指定模板值的区域进行绘制，采用默认状态
	void SetRenderDefaultWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 对指定模板值的区域进行绘制，采用Alpha混合
	void SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef);
	// 2D默认状态绘制
	void Set2DRenderDefault(ID3D11DeviceContext* deviceContext);
	// 2D混合绘制
	void Set2DRenderAlphaBlend(ID3D11DeviceContext* deviceContext);
	// 线框模式
	void SetWireFrameWode(ID3D11DeviceContext* deviceContext);

	//
	// 矩阵设置
	//

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);

	void XM_CALLCONV SetReflectionMatrix(DirectX::FXMMATRIX R);
	void XM_CALLCONV SetShadowMatrix(DirectX::FXMMATRIX S);
	void XM_CALLCONV SetRefShadowMatrix(DirectX::FXMMATRIX RefS);

	//
	// 光照、材质和纹理相关设置
	//

	// 各种类型灯光允许的最大数目
	static const int maxLights = 20;
	bool isShadow;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& pointLight);
	void SetSpotLight(size_t pos, const SpotLight& spotLight);
	void SetDirLightNums(int num);
	void SetPointLightNums(int num);
	void SetSpotLightNums(int num);

	void SetMaterial(const Material& material);

	void SetTexture(ID3D11ShaderResourceView* texture);
	void SetTexture3D(ID3D11ShaderResourceView* texture);
	void SetTexture2D(ID3D11ShaderResourceView* texture);
	void SetRWTexture3D(ID3D11UnorderedAccessView* texture);

	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos);
	
	// Volumetric Lightmap 设置
	void SetVLMWorldToUVScale(DirectX::XMFLOAT3 VLMWorldToUVScale);
	void SetVLMWorldToUVAdd(DirectX::XMFLOAT3 VLMWorldToUVAdd);
	void SetVLMIndirectionTextureSize(DirectX::XMFLOAT3 indirectionTextureSize);
	void SetVLMBrickSize(float brickSize);
	void SetVLMBrickTexelSize(DirectX::XMFLOAT3 VLMBrickTexelSize);
	void SetSHMode(int);
	void SetSphereSpeed(int SphereSpeed);

	//
	// 状态开关设置
	//
	void SetReflectionState(bool isOn);
	void SetShadowState(bool isOn);
	void SetTextureUsed(bool isOn);
	void SetSHUsed(bool isOn);
	void SetLightUsed(bool isOn);
	void SetDirLightUsed(bool isOn);
	void SetPointLightUsed(bool isOn);


	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext);

	void SetDebugName();


private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};


#endif