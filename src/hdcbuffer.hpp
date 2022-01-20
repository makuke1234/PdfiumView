#pragma once

#include "common.hpp"

#include <functional>
#include <unordered_map>

namespace pdfv::hdc
{
	class Renderer
	{
	public:
		using RenderT = w::GDI<HBITMAP>;

		struct RenderStats
		{
			RenderT hrender;
			xy<int> pos, size;
		};

	private:
		std::unordered_map<std::size_t, RenderStats> bmBuffer;

	public:

		void clear() noexcept;

		[[nodiscard]] bool hasPage(std::size_t pageIdx) const noexcept;
		void putPage(std::size_t pageIdx, xy<int> pos, xy<int> size, std::function<RenderT (void *)> render, void * renderArg);

		RenderT & getPage(std::size_t pageIdx);
	};
}
