#pragma once
#include <string>
#include <d3d12.h>
#include <wrl.h>

namespace d3dUtil {

namespace WRL = Microsoft::WRL;
struct Texture {
	std::string	  name_;
	std::wstring  fileName_;
	WRL::ComPtr<ID3D12Resource> pResource_ = nullptr;
	WRL::ComPtr<ID3D12Resource> pUploadHeap_ = nullptr;
};

}