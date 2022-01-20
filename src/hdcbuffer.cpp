#include "hdcbuffer.hpp"

void pdfv::hdc::Renderer::clear() noexcept
{
	this->bmBuffer.clear();
}

[[nodiscard]] bool pdfv::hdc::Renderer::hasPage(std::size_t pageIdx) const noexcept
{
	return this->bmBuffer.find(pageIdx) != this->bmBuffer.end();
}
void pdfv::hdc::Renderer::putPage(std::size_t pageIdx, xy<int> pos, xy<int> size, std::function<Renderer::RenderT (void *)> render, void * renderArg)
{
	bool reRender{ true };
	if (auto it = this->bmBuffer.find(pageIdx); it != this->bmBuffer.end())
	{
		if (it->second.pos == pos && it->second.size == size)
		{
			reRender = false;
		}
	}

	if (reRender)
	{
		this->bmBuffer[pageIdx] = RenderStats(render(renderArg), pos, size);
	}
}

pdfv::hdc::Renderer::RenderT & pdfv::hdc::Renderer::getPage(std::size_t pageIdx)
{
	return this->bmBuffer.at(pageIdx).hrender;
}

