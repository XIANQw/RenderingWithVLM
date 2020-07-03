﻿#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct VertexPosNormalColor {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 noraml;
	DirectX::XMFLOAT4 color;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
};

struct VertexPosNormalTex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
};


