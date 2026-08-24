#pragma once
#ifndef CSS_COLORS_HPP
#define CSS_COLORS_HPP
#include "colorspace.hpp"
#include "io.hpp"
namespace css_colors {
	using details::color;
	using details::typed_color;
	using details::untyped_color;
	namespace colorspaces {
		using namespace details::colorspaces::registered_colorspaces;
	};

	using details::colorspaces::conv_to_dword::to_dword;
	using details::colorspaces::conv_to_dword::from_dword;

	using details::colorspaces::clamp;

	using details::parser::parse;
}
#endif // !CSS_COLORS_HPP
