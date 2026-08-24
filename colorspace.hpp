#pragma once
#ifndef CSS_COLOR_COLORSPACE_HPP
#define CSS_COLOR_COLORSPACE_HPP

#include <type_traits>
#include "colorspace_common.hpp"
#include "spaces/srgb.hpp"
#include "spaces/xyz.hpp"
#include "spaces/hsl.hpp"
#include "spaces/linear_rgb.hpp"
#include "spaces/hsv.hpp"
#include "spaces/hwb.hpp"
#include "spaces/lab.hpp"
#include "spaces/lch.hpp"
#include "spaces/oklab.hpp"
#include "spaces/oklch.hpp"
#include "spaces/a98_rgb.hpp"
#include "spaces/prophoto.hpp"
#include "spaces/rec2020.hpp"

namespace css_colors::details {
	namespace colorspaces {
		struct unknown_colorspace {};
		REGISTER_COLORSPACE(unknown_colorspace);

		struct null_colorspace;
		REGISTER_COLORSPACE(null_colorspace);

		constexpr uint8_t color_function_cnt = close_counter<color_function_counter>::size;//COUNTER_VAL(color_function_counter, colorfunc_guard);
		constexpr uint8_t predefined_colorspace_cnt = close_counter<predefined_colorspace_counter>::size;// COUNTER_VAL(predefined_colorspace_counter, predefined_colorspace_guard);
#undef REGISTER_COLORFUNCTION
#undef REGISTER_COLORSPACE
#undef COUNTER_INC
#undef COUNTER_VAL

		using untyped_color = std::pair<vec, real>;

		template<class colorspace>
		struct typed_color :public untyped_color {
			constexpr typed_color(const untyped_color& c)noexcept:untyped_color(c) {}
			using untyped_color::untyped_color;
		};

		struct color :public std::pair<uint8_t, untyped_color> {
			template<typename colorspace>
			inline constexpr color(const typed_color<colorspace>& typed_color) noexcept :
				std::pair<uint8_t, untyped_color>(to_index_v<colorspace>, typed_color) {}
			inline constexpr color(uint8_t kind, const untyped_color& data) noexcept :
				std::pair<uint8_t, untyped_color>(kind, data) {}
			//using std::pair<uint8_t, untyped_color>::pair;
			inline constexpr color()noexcept :std::pair<uint8_t, untyped_color>({ to_index_v<null_colorspace> ,untyped_color{} }) {};
			inline constexpr bool is_null()const noexcept { return first == to_index_v<null_colorspace>; }
			inline constexpr bool is_unknown()const noexcept { return first == to_index_v<unknown_colorspace>; }
			inline constexpr operator bool()const noexcept { return first < to_index_v<unknown_colorspace>; }

			template<class colorspace>
			constexpr std::optional<typed_color<colorspace>> as_typed()const;
			template<class colorspace>
			constexpr color as()const;
			constexpr color as(uint8_t type)const;
		};
		inline static constexpr color nullcolor = color(typed_color<null_colorspace>{});
		inline static constexpr color unknowncolor = color(typed_color<unknown_colorspace>{});

		inline constexpr typed_color<srgb> u32_to_color(uint32_t u32)
		{
			typed_color<srgb> ret;
			ret.first = vec{ static_cast<real>((u32 >> 24) & 0xFF),static_cast<real>((u32 >> 16) & 0xFF),static_cast<real>((u32 >> 8) & 0xFF) };
			ret.second = static_cast<real>(u32 & 0xFF) / 255;
			return ret;
		}
		template <class colorspace,typename = void>
		struct is_legacy :std::bool_constant<false> {};
		template <class colorspace>
		struct is_legacy<colorspace,std::void_t<decltype(colorspace::legacy)>> :std::bool_constant<colorspace::legacy> {};

		template<class colorspace>
		constexpr bool is_legacy_v = is_legacy<colorspace>::value;

		template <class colorspace, typename = void>
		struct output_legacy :std::bool_constant<is_legacy_v<colorspace>> {};
		template <class colorspace>
		struct output_legacy<colorspace, std::void_t<decltype(colorspace::out_legacy)>> :std::bool_constant<colorspace::out_legacy> {};
		template<class colorspace>
		constexpr bool output_legacy_v = output_legacy<colorspace>::value;

		template<uint8_t colorspace = 0>
		inline constexpr bool dynamic_output_legacy_v(uint8_t id) {
			if (colorspace == id)return output_legacy_v<to_colorspace_t<colorspace>>;
			return dynamic_output_legacy_v<colorspace + 1>(id);
		}
		template<>
		inline constexpr bool dynamic_output_legacy_v<to_index_v<unknown_colorspace>>(uint8_t) 
		{
			return false;
		}
		template<class colorspace>
		inline constexpr typed_color<colorspace> clamp(const typed_color<colorspace>& color) {
			typed_color<colorspace> ret = color;
			if constexpr (parser::should_clamp_v<std::tuple_element_t<0, typename colorspace::arg_type>>) {
				ret.first[0] = parser::clamp_element< std::tuple_element_t<0, typename colorspace::arg_type>>::clamp(color.first[0]);
			}
			if constexpr (parser::should_clamp_v<std::tuple_element_t<1, typename colorspace::arg_type>>) {
				ret.first[1] = parser::clamp_element< std::tuple_element_t<1, typename colorspace::arg_type>>::clamp(color.first[1]);
			}
			if constexpr (parser::should_clamp_v<std::tuple_element_t<2, typename colorspace::arg_type>>) {
				ret.first[2] = parser::clamp_element< std::tuple_element_t<2, typename colorspace::arg_type>>::clamp(color.first[2]);
			}
			return ret;
		};
		template<uint8_t I = 0>
		struct clamp_helper {
			using colorspace = to_colorspace_t<I>;
			inline static constexpr color dclamp(const color& c)
			{
				if (I == c.first) {
					return color(c.first, clamp<colorspace>(c.second));
				}
				else {
					return clamp_helper<I + 1>::dclamp(c);
				}
			}
		};
		template<>
		struct clamp_helper<to_index_v<unknown_colorspace>> {
			inline static constexpr color dclamp(const color&)
			{
				return nullcolor;
			}
		};
		inline constexpr color clamp(const color& c) {
			return clamp_helper<>::dclamp(c);
		};

	}
	using colorspaces::untyped_color;
	using colorspaces::typed_color;
	using colorspaces::color;
	using colorspaces::nullcolor;
	using colorspaces::unknowncolor;
	
}
#include "colorspace_convert.hpp"
#endif /* CSS_COLOR_COLORSPACE_HPP */
