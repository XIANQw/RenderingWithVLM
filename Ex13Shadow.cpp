﻿#include "Ex13Shadow.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

using namespace DirectX;



Ex13Shadow::Ex13Shadow(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_CameraMode(CameraMode::TPS),
	m_ShadowMat(),
	m_WoodCrateMat(){
}


Ex13Shadow::~Ex13Shadow() {

}

bool Ex13Shadow::Init() {
	if (!D3DApp::Init()) return false;

	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!InitResource()) return false;

	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::MODE_RELATIVE);

	return true;
}



void Ex13Shadow::OnResize() {
	D3DApp::OnResize();

	if (m_pCamera != nullptr) {
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	}
}



void Ex13Shadow::UpdateScene(float dt)
{
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();
	Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState();

	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	auto cam1st = std::dynamic_pointer_cast<FPSCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<TPSCamera>(m_pCamera);

	if (m_CameraMode == CameraMode::FPS || m_CameraMode == CameraMode::Free) {
		// FPS mode
		if (keyState.IsKeyDown(Keyboard::W)) {
			if (m_CameraMode == CameraMode::FPS) {
				cam1st->Walk(dt * 5.0f);
			}
			else {
				cam1st->MoveForward(dt * 5.0f);
			}
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			if (m_CameraMode == CameraMode::FPS) {
				cam1st->Walk(-dt * 5.0f);
			}
			else {
				cam1st->MoveForward(-dt * 5.0f);
			}
		}
		if (keyState.IsKeyDown(Keyboard::D)) cam1st->Strafe(dt * 5.0f);
		if (keyState.IsKeyDown(Keyboard::A)) cam1st->Strafe(-dt * 5.0f);

		// 限制摄像机在固定范围内 x(-8.9, 8.9), z(-8.9, 8.9), y(0, 8.9) y不能为负
		XMFLOAT3 adjustPos;
		// XMVectorClamp(V, min, max) 将V的每个分量限定在[min, max]范围
		XMStoreFloat3(&adjustPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
		cam1st->SetPosition(adjustPos);

		// 只有第一人称才是摄像机和箱子都移动
		if (m_CameraMode == CameraMode::FPS) m_WoodCrate.GetTransform().SetPosition(adjustPos);

		if (mouseState.positionMode == Mouse::MODE_RELATIVE) {
			cam1st->Pitch(mouseState.y * dt * 2.5f);
			cam1st->RotateY(mouseState.x * dt * 2.5f);
		}
	}
	else if (m_CameraMode == CameraMode::Observe) {
		// 更新摄像机目标位置
		cam3rd->SetTarget(m_WoodCrate.GetTransform().GetPosition());
		// 摄像机绕目标旋转
		cam3rd->RotateX(mouseState.y * dt * 2.5f);
		cam3rd->RotateY(mouseState.x * dt * 2.5f);
		cam3rd->Approach(mouseState.scrollWheelValue / 120 * 1.0f);
	}
	else if (m_CameraMode == CameraMode::TPS) {
		if (keyState.IsKeyDown(Keyboard::W)) {
			m_WoodCrate.GetTransform().Translate(m_WoodCrate.GetTransform().GetForwardAxis(), dt * 5.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			m_WoodCrate.GetTransform().Translate(m_WoodCrate.GetTransform().GetForwardAxis(), -dt * 5.0f);
		}
		if (keyState.IsKeyDown(Keyboard::D))
			m_WoodCrate.GetTransform().Translate(m_WoodCrate.GetTransform().GetRightAxis(), dt * 5.0f);
		if (keyState.IsKeyDown(Keyboard::A))
			m_WoodCrate.GetTransform().Translate(m_WoodCrate.GetTransform().GetRightAxis(), -dt * 5.0f);
		cam3rd->SetTarget(m_WoodCrate.GetTransform().GetPosition());
		// 摄像机绕目标旋转
		cam3rd->RotateX(mouseState.y * dt * 2.5f);
		cam3rd->RotateY(mouseState.x * dt * 2.5f);
		cam3rd->Approach(mouseState.scrollWheelValue / 120 * 1.0f);
	}

	// 更新观察矩阵
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());

	// 重置滚轮
	m_pMouse->ResetScrollWheelValue();

	// 切换到FPS模式
	if (keyState.IsKeyDown(Keyboard::D1) && m_CameraMode != CameraMode::FPS) {
		if (!cam1st) {
			cam1st.reset(new FPSCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}

		cam1st->LookTo(m_WoodCrate.GetTransform().GetPosition(),
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_CameraMode = CameraMode::FPS;
	}
	// 切换到TPS模式
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CameraMode != CameraMode::Observe) {
		if (!cam3rd) {
			cam3rd.reset(new TPSCamera);
			cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam3rd;
		}
		XMFLOAT3 target = m_WoodCrate.GetTransform().GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistMinMax(3.0f, 20.0f);

		m_CameraMode = CameraMode::Observe;
	}

	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3) && m_CameraMode != CameraMode::Free) {
		if (!cam1st) {
			cam1st.reset(new FPSCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}
		XMFLOAT3 pos = m_WoodCrate.GetTransform().GetPosition();
		XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		pos.y += 3;
		cam1st->LookTo(pos, to, up);
		m_CameraMode = CameraMode::Free;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D4) && m_CameraMode != CameraMode::TPS) {
		if (!cam3rd) {
			cam3rd.reset(new TPSCamera);
			cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam3rd;
		}
		XMFLOAT3 target = m_WoodCrate.GetTransform().GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistMinMax(3.0f, 20.0f);

		m_CameraMode = CameraMode::TPS;
	}

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape)) {
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}

}



void Ex13Shadow::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	/***********************
		绘制不透明的反射物体
	************************/
	m_BasicEffect.SetReflectionState(true);
	m_BasicEffect.SetRenderDefaultWithStencil(m_pd3dImmediateContext.Get(), 1);

	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	for (auto & wall : m_Walls) {
		wall.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}

	/***************************
		绘制不透明反射物体的阴影
	****************************/
	m_WoodCrate.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 1);
	
	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// 恢复到原来的状态
	m_BasicEffect.SetShadowState(false);
	m_WoodCrate.SetMaterial(m_WoodCrateMat);

	m_BasicEffect.SetReflectionState(false);
	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());

	for (auto& wall : m_Walls)
		wall.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	/************************
	6. 绘制不透明正常物体的阴影
	*************************/
	m_WoodCrate.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);	// 反射关闭，阴影开启
	m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);

	m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetShadowState(false);		// 阴影关闭
	m_WoodCrate.SetMaterial(m_WoodCrateMat);



	HR(m_pSwapChain->Present(0, 0));
}


bool Ex13Shadow::InitResource()
{

	/*******************
		初始化纹理
	********************/
	ComPtr<ID3D11ShaderResourceView> texture;
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	m_WoodCrateMat = material;
	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	// 读取木箱
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	m_WoodCrate.SetBuffer<VertexPosNormalTex, DWORD>(m_pd3dDevice.Get(), Geometry::CreateBox());
	m_WoodCrate.GetTransform().SetPosition(0.0f, 0.01f, 5.0f);
	m_WoodCrate.SetTexture(texture.Get());
	m_WoodCrate.SetMaterial(material);


	// 初始化地板
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.GetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(), 
		Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetMaterial(material);
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);

	/*************************************
						g_Walls[0]
					初始化墙壁 (0, 3, 10)
				   ________________
				 / |             / |
				/  |		    /  |
			   /___|___________/   |
			   |   |_ _ _ _ _ _|_ _|
			   |  /		       |  / g_Walls[1]
   (-10, 3, 0) | /			   | / (10, 3, 0)
   g_Walls[3]  |/______________|/
				  (0, 3, -10)
				 width g_Walls[2]
	*************************************/
	m_Walls.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.GetAddressOf()));
	for (int i = 0; i < m_Walls.size(); i++) {
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
		m_Walls[i].SetBuffer(m_pd3dDevice.Get(),
			Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
		Transform& transform = m_Walls[i].GetTransform();
		transform.SetRotation(-XM_PIDIV2, XM_PIDIV2 * i, 0.0f);
		transform.SetPosition(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
		m_Walls[i].SetTexture(texture.Get());
	}
	/**************
		初始化摄像机	
	***************/	
	m_CameraMode = CameraMode::TPS;
	auto camera = std::shared_ptr<TPSCamera>(new TPSCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetDistance(5.0f);
	camera->SetDistMinMax(2.0f, 14.0f);

	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());

	m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
	// 稍微高一点位置以显示阴影
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
	m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));


	/******************
	* 初始化默认光照	  *
	*******************/
	// 环境光
	DirectionalLight dirLight;
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	// 灯光
	PointLight pointLight;
	pointLight.position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	pointLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	pointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 25.0f;
	m_BasicEffect.SetPointLight(0, pointLight);

	return true;
}
