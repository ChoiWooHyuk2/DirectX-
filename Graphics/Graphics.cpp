#include "Graphics.h"



bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	if (!InitializeDirectX(hwnd, width, height))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	return true;
}

void Graphics::RenderFrame()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f};//색상설정
	this->deviceConstext->ClearRenderTargetView(this->rederTargetView.Get(), bgcolor);
	this->deviceConstext->ClearDepthStencilView(this->depthStencilView.Get(),D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,0);

	this->deviceConstext->IASetInputLayout(this->vertexshader.GetInputLayout());
	this->deviceConstext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceConstext->RSSetState(this->rasterizerState.Get());
	
	this->deviceConstext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	this->deviceConstext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());

	this->deviceConstext->VSSetShader(vertexshader.GetShader(),NULL,0);
	this->deviceConstext->PSSetShader(pixelshader.GetShader(),NULL,0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//삼각형
	this->deviceConstext->PSSetShaderResources(0, 1, this->myTexture.GetAddressOf());
	this->deviceConstext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	this->deviceConstext->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	this->deviceConstext->DrawIndexed(6, 0, 0);



	//글씨 그리기
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), L"HELLO WORLD", DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f,  DirectX::XMFLOAT2(0.0f,0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();

	this->swapchain->Present(1, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd, int width, int height)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		ErrorLogger::Log("IDXGI 어뎁터를 찾을 수 없습니다.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc.Width = width;
	scd.BufferDesc.Height = height;
	scd.BufferDesc.RefreshRate.Numerator = 60; //재생 분자 대략 1초에 60FPS 라는뜻
	scd.BufferDesc.RefreshRate.Denominator = 1; //분모
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //리소스데이터 형식
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //스캔라인 순서
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //주어진 모니터 해상도에 맞게 이미지가 늘어나는 방법

	scd.SampleDesc.Count = 1; //다중 샘플링 매개변수 설정
	scd.SampleDesc.Quality = 0; //그래픽 퀄리티

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //지표면 및 자원 생성 옵션 //현재설정 : [표면 또는 리소스를 출력 렌더링 대상으로 사용합니다.]
	scd.BufferCount = 1; //스왑체인의 버퍼 수
	scd.OutputWindow = hwnd; //출력 창에 대한 핸들
	scd.Windowed = TRUE; //출력이 윈도우 모드인지 여부
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //디스플레이 화면에서 픽셀을 처리하기 위한 옵션
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //스왑체인 동작에 대한 옵션

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(
		adapters[0].pAdapter, //IDXGI 어뎁터
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL, //소프트웨어 드라이브 타입
		NULL, //런타임 레이어에 대한 플레그
		NULL, //기능 레벨 배열
		0, //배열의 기능 레벨
		D3D11_SDK_VERSION,//SDK 버전
		&scd, //스왑체인 설명
		this->swapchain.GetAddressOf(),
		this->device.GetAddressOf(),
		NULL, //지원되는 기능 레벨
		this->deviceConstext.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "스왑체인 생성에 실패했습니다.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D>	backBuffer;

	hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "버퍼가져오기 실패.");
		return false;
	}
	//리소스를 엑세스 하기 위한 렌더타겟뷰

	hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->rederTargetView.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "랜더타겟뷰 생성 실패.");
		return false;
	}

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "깊이 스텐실 버퍼 생성에 실패했습니다.");
		return false;
	}

	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "깊이 스텐실 뷰를 생성에 실패했습니다.");
		return false;
	}
	this->deviceConstext->OMSetRenderTargets(1, this->rederTargetView.GetAddressOf(), this->depthStencilView.Get());

	//깊이 스텐실 상태 생성
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "깊이 스텐실 상태 생성에 실패했습니다.");
		return false;
	}

	//뷰포트 생성
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//뷰포트 설정
	this->deviceConstext->RSSetViewports(1, &viewport);

	//래스터 라이저 생성
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID; //채우기모드
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;//3d 할때 앞에서는 보이고 뒤에서는 안보이게 하는 이미지 처리가 이거임
	hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "래스터 라이더 스테이트 생성을 실패했습니다.");
		return false;
	}

	spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceConstext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\fonts\\myfile.spritefont");

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "샘플 스테이트 생성을 실패했습니다.");
		return false;
	}
	return true; 
}

bool Graphics::InitializeShaders()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layout);


	if (!vertexshader.Initialize(this->device,L"x64\\Debug\\vertexshader.cso",layout ,numElements))
		return false;

	if (!pixelshader.Initialize(this->device, L"x64\\Debug\\pixelshader.cso"))
		return false;

	return true;
}

bool Graphics::InitializeScene()
{	 //화면 중앙이 0임
	//텍스쳐
	Vertex v[] =
	{
		Vertex(-0.5f, -0.5f, 1.0f, 0.0f, 1.0f), //Bottom Left [0]
		Vertex(-0.5f,  0.5f, 1.0f, 0.0f, 0.0f), //Top LEFT    [1]
		Vertex( 0.5f,  0.5f, 1.0f, 1.0f, 0.0f), //Top Right   [2]
		Vertex( 0.5f, -0.5f, 1.0f, 1.0f, 1.0f), //Bottom Right[3]

	};

	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(v);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;

	HRESULT hr = this->device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, this->vertexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "정점버퍼를 생성하는데 실패했습니다.");
		return false;
	}

	//인덱스버퍼
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD)*ARRAYSIZE(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = indices;
	hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, indicesBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "인덱스 버퍼 생성에 실패했습니다");
		return hr;
	}

	hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\char_idle.png", nullptr, myTexture.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "WIC 텍스쳐 생성에 실패했습니다.");
		return false;
	}
	return true;
}

Graphics::Graphics()
{
}


Graphics::~Graphics()
{
}
