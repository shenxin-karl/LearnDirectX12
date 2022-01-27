#pragma once
#include "dx12libStd.h"
#include <memory>

namespace dx12lib {



class CommandList {
public:
	CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem);
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept;
	HRESULT close();
	void setViewports(const D3D12_VIEWPORT &viewport);
	void setViewprots(const std::vector<D3D12_VIEWPORT> &viewports);
	void setScissorRects(const D3D12_RECT &rect);
	void setScissorRects(const std::vector<D3D12_RECT> &rects);
	void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget);
private:
	D3D12_COMMAND_LIST_TYPE                 _cmdListType;
	std::weak_ptr<Device>                   _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList>  _pCommandList;
};

}