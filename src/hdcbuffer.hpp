#pragma once

#include "common.hpp"

#include <functional>
#include <unordered_map>

namespace pdfv::hdc
{
	class Renderer
	{
	public:
		using RenderT = w::SafeGDI<HBITMAP>;

		struct RenderStats
		{
			RenderT hrender;
			xy<int> pos, size;
		};

	private:
		std::unordered_map<std::size_t, RenderStats> bmBuffer;

	public:

		/**
		 * @brief Clears the render buffer
		 * 
		 */
		void clear() noexcept;

		/**
		 * @brief Determines whether page asked for has been already rendered or not
		 * 
		 * @param pageIdx Page index to search
		 * @return true Page has been rendered before and is available
		 */
		[[nodiscard]] bool hasPage(std::size_t pageIdx) const noexcept;
		/**
		 * @brief Put new page to render buffer, only re-renders if position and/or size is different
		 * 
		 * @param pageIdx Page index to render
		 * @param pos Render position
		 * @param size Render size
		 * @param render Rendering function, returns RenderT object
		 * @param renderArg Argument to the rendering function
		 */
		void putPage(std::size_t pageIdx, xy<int> pos, xy<int> size, std::function<RenderT (void *)> render, void * renderArg);

		/**
		 * @param pageIdx Page index
		 * @return RenderT& Reference to requested page's render object
		 */
		RenderT & getPage(std::size_t pageIdx);
	};
}
