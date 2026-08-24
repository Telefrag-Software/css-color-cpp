#pragma once
#ifndef CSS_COLOR_COLORSPACE_CONVERT_HPP
#define CSS_COLOR_COLORSPACE_CONVERT_HPP

#include "colorspace.hpp"
namespace css_colors::details {
	namespace colorspaces {
		namespace converter_impl {

			template<class T>
			inline constexpr size_t depth_v = depth_v<typename T::base_type> +1;
			template<>
			inline constexpr size_t depth_v<srgb> = 0;		//use srgb as root.

			struct nullconv_t {
				//as a placeholder when Target==Source
				inline static constexpr vec conv(const vec& v) {
					return v;
				}
			};
			template <class convertion, typename = void>
			constexpr bool is_matrix_v = false;
			template <class convertion>
			constexpr bool is_matrix_v<convertion, std::void_t<decltype(convertion::conv_mat)>> = true;

			template <class convertion, typename = void>
			constexpr bool is_func_v = false;
			template <class convertion>
			constexpr bool is_func_v<convertion, std::void_t<decltype(convertion::conv)>> = !is_matrix_v<convertion>;

			template<class convertion>
			constexpr bool is_null_v = !is_matrix_v<convertion> && !is_func_v<convertion>;

			template <class convertion, typename = void>
			struct next_convertion { using type = nullconv_t; };
			template <class convertion>
			struct next_convertion<convertion, std::void_t<typename convertion::next_converter>> { using type = typename convertion::next_converter; };
			template <class convertion>
			using next_convertion_t = typename next_convertion<convertion>::type;
			template<class convertion>
			constexpr bool has_next_convertion_v = !std::is_same_v<nullconv_t, next_convertion_t<convertion>>;

			template<class convertion>
			struct matrix_wrapper {
				inline static constexpr vec conv(const vec& v) {
					return convertion::conv_mat * v;
				}
			};
			template<class convertion>
			using convert_wrapper_t = std::conditional_t<is_matrix_v<convertion>, matrix_wrapper<convertion>, convertion>;
			template<class convertion, typename = void>
			struct invoke_convertion {
				inline constexpr static vec invoke(const vec& v) {
					return convert_wrapper_t<convertion>::conv(v);
				}
			};
			template<class convertion>
			struct invoke_convertion<convertion, std::enable_if_t<has_next_convertion_v<convertion>>> {
				inline constexpr static vec invoke(const vec& v) {
					return invoke_convertion<next_convertion_t<convertion>>::invoke(convert_wrapper_t<convertion>::conv(v));
				}
			};

			template<class convertion, class next_convertion, class E = void>
			struct converter;
			template<class convertion, class next_convertion>
			struct converter<convertion, next_convertion, std::enable_if_t<is_null_v<next_convertion>>> :convertion {};

			template<class convertion, class next_convertion>
			struct converter<convertion, next_convertion, typename std::enable_if_t<is_matrix_v<convertion>&& is_matrix_v<next_convertion>>>
			{
				inline constexpr static mat conv_mat = next_convertion::conv_mat * convertion::conv_mat;
				using next_converter = next_convertion_t<next_convertion>;
			};
			template<class convertion, class next_convertion>
			struct converter<convertion, next_convertion, typename std::enable_if_t<is_func_v<convertion> || (is_matrix_v<convertion> && is_func_v<next_convertion>)>>
				:convertion
			{
				using next_converter = next_convertion;
			};

			template<class From, class To, class Enable = void>
			constexpr bool is_ancestor_v = false;
			template<class From, class To>
			constexpr bool is_ancestor_v < From, To, std::enable_if_t<depth_v<From> < depth_v<To> && !std::is_same_v<From, typename To::base_type>>> = is_ancestor_v<From, typename To::base_type>;
			template<class To>
			constexpr bool is_ancestor_v<typename To::base_type, To, void> = true;

			template<class From, class To, class Enable = void>
			struct next_convert_step {
				using type = typename From::base_type;
			};
			template<class From, class To>
			struct next_convert_step<From, To, std::enable_if_t<is_ancestor_v<From, To> && !std::is_same_v<From, typename To::base_type>>> {
				using type = typename next_convert_step<From, typename To::base_type>::type;
			};
			template<class From, class To>
			struct next_convert_step<From, To, std::enable_if_t<std::is_same_v<From, typename To::base_type>>> {
				using type = To;
			};
			template<class From, class To>
			using next_convert_step_t = typename next_convert_step<From, To>::type;

			template<class From, class To, class Enable = void>
			struct basic_convert;
			template<class From, class To>
			struct basic_convert<From, To, std::enable_if_t<std::is_same_v<From, typename To::base_type>>> {
				using type = typename To::from_base;
			};
			template<class From, class To>
			struct basic_convert<From, To, std::enable_if_t<std::is_same_v<typename From::base_type, To>>> {
				using type = typename From::to_base;
			};
			template<class From, class To>
			using basic_convert_t = typename basic_convert<From, To>::type;

			template<class From, class To>
			struct convert_path;
			template<class From>
			struct convert_path<From, From>
			{
				using type = nullconv_t;
			};

			template<class From, class To>
			struct convert_path
			{
				using type = converter<basic_convert_t<From, next_convert_step_t<From, To>>, typename convert_path<next_convert_step_t<From, To>, To>::type>;
			};
			template<class From, class To>
			inline constexpr static vec convert_and_clamp(const vec& in) {
				//using type = typename convert_path<From, To>::type;
				auto converted = invoke_convertion<typename convert_path<From, To>::type>::invoke(in);
				if constexpr (parser::should_clamp_v<std::tuple_element_t<0, typename To::arg_type>>) {
					converted[0] = parser::clamp_element<std::tuple_element_t<0, typename To::arg_type>>::clamp(converted[0]);
				}
				if constexpr (parser::should_clamp_v<std::tuple_element_t<1, typename To::arg_type>>) {
					converted[1] = parser::clamp_element<std::tuple_element_t<1, typename To::arg_type>>::clamp(converted[1]);
				}
				if constexpr (parser::should_clamp_v<std::tuple_element_t<2, typename To::arg_type>>) {
					converted[2] = parser::clamp_element<std::tuple_element_t<2, typename To::arg_type>>::clamp(converted[2]);
				}
				return converted;

			};
			template<class To, uint8_t from_id = 0>
			struct dynamic_convert_helper {
				inline static constexpr std::optional<typed_color<To>> conv(const color& c) {
					if (c.first == from_id) {
						//return typed_color<To>(convert_and_clamp<typename to_colorspace<from_id>::type, To>(c.second.first), c.second.second);
						return typed_color<To>(invoke_convertion<typename convert_path<typename to_colorspace<from_id>::type, To>::type>::invoke(c.second.first), c.second.second);
					}
					else {
						return dynamic_convert_helper<To, from_id + 1>::conv(c);
					}
				}
			};
			template<class To>
			struct dynamic_convert_helper<To, to_index_v<unknown_colorspace>> {
				inline static constexpr std::optional<typed_color<To>> conv(const color&) {
					return std::nullopt;
				}
			};

			template<uint8_t colorspace = 0>
			struct to_dynamic_type {
				inline static constexpr color conv(const color& c, uint8_t target) {
					if (target == colorspace) {
						auto converted = dynamic_convert_helper<typename to_colorspace<colorspace>::type>::conv(c);
						if (!converted)return nullcolor;
						return converted.value();
					}
					return to_dynamic_type<colorspace + 1>::conv(c, target);
				}
			};
			template<>
			struct to_dynamic_type<to_index_v<unknown_colorspace>> {
				inline static constexpr color conv(const color&, uint8_t) {
					return nullcolor;
				}
			};
		}
		template<class From, class To>
		using convert = typename converter_impl::convert_path<From, To>::type;
		template<class To>
		using dynamic_convert = converter_impl::dynamic_convert_helper<To>;

		using dyn2dyn_convert = converter_impl::to_dynamic_type<0>;


		template<class colorspace>
		inline constexpr std::optional<typed_color<colorspace>> color::as_typed()const {
			return dynamic_convert<colorspace>::conv(*this);
		}
		template<class colorspace>
		inline constexpr color color::as() const {
			auto typed = as_typed<colorspace>();
			if (typed)return color(typed.value());
			else return nullcolor;
		}

		inline constexpr color color::as(uint8_t type) const {
			return dyn2dyn_convert::conv(*this, type);
		}
		namespace conv_to_dword {
			inline constexpr uint8_t fit_in_byte(const real& x) {
				return static_cast<uint8_t>(math::max<real>(math::min<real>(math::round<real>(255 * x), 255), 0));
			}
			inline constexpr std::optional<std::array<uint8_t, 4>> to_dword(const color& c,const char shuffle[4]) {
				auto rgb_data = c.as_typed<srgb>();
				if (!rgb_data)return std::nullopt;
				std::array<uint8_t, 4> ret = { 0,0,0,0 };
				for (size_t i = 0; i < 4; i++) {
					switch (const_tolower(shuffle[i])) {
					case 'r':ret[i]= fit_in_byte(255 * rgb_data.value().first[0]); break;
					case 'g':ret[i] = fit_in_byte(255 * rgb_data.value().first[1]); break;
					case 'b':ret[i] = fit_in_byte(255 * rgb_data.value().first[2]); break;
					case 'a':ret[i]= fit_in_byte(255 * rgb_data.value().second); break;
					case 'x':ret[i] = 255; break;
					default:ret[i] = 0;
					}
				}
				return ret;
			}
			inline constexpr color from_dword(const std::array<uint8_t,4>& data, const char shuffle[4]) {
				typed_color<srgb> ret = { vec{0,0,0},real(1.0) };
				for (size_t i = 0; i < 4; i++) {
					switch (const_tolower(shuffle[i])) {
					case 'r':ret.first[0] = data[i]; break;
					case 'g':ret.first[1] = data[i]; break;
					case 'b':ret.first[2] = data[i]; break;
					case 'a':ret.second = static_cast<real>(data[i] / 255.); break;
					}
				}
				return ret;
			}
		}
	}
}

#endif /* CSS_COLOR_COLORSPACE_CONVERT_HPP */
