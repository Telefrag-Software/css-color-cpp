#pragma once
#ifndef CSS_COLOR_IO_HPP
#define CSS_COLOR_IO_HPP

#include <iostream>
#include <charconv>
#include "colorspace.hpp"
#include "parser.hpp"
namespace css_colors::details {
	namespace io_helpers_impl {
		using namespace colorspaces;

		template <class colorspace, typename = void>
		struct has_funcname :std::bool_constant<false> {};
		template <class colorspace>
		struct has_funcname<colorspace, std::void_t<decltype(colorspace::default_color_function_name)>> :std::bool_constant<true> {};
		template<class colorspace>
		constexpr bool has_funcname_v = has_funcname<colorspace>::value;

		template <class colorspace, typename = void>
		struct has_multi_funcname :std::bool_constant<false> {};
		template <class colorspace>
		struct has_multi_funcname<colorspace, std::enable_if_t<std::is_array_v<decltype(colorspace::default_color_function_name)>>> :std::bool_constant<true> {};
		template<class colorspace>
		constexpr bool has_multi_funcname_v = has_multi_funcname<colorspace>::value;

		template<class colorspace>
		constexpr bool has_single_funcname_v = has_funcname_v<colorspace> && !has_multi_funcname_v<colorspace>;

		template <class colorspace, typename = void>
		struct has_predefined :std::bool_constant<false> {};
		template <class colorspace>
		struct has_predefined<colorspace, std::void_t<decltype(colorspace::default_predefined_name)>> :std::bool_constant<true> {};
		template<class colorspace>
		constexpr bool has_predefined_v = has_predefined<colorspace>::value;

		template<class colorspace>
		constexpr bool has_default_output_format_v = has_funcname_v<colorspace> || has_predefined_v<colorspace>;


		template<class colorspace, int pos = 0, typename = void >
		struct print_prefix {
			//inline constexpr static std::ostream& print(std::ostream& os) {
			//	static_assert(false, "Unserializable color");
			//	return os;
			//}
		};
		template<class colorspace, int pos>
		struct print_prefix <colorspace, pos, std::enable_if_t<has_single_funcname_v<colorspace>>> {
			inline constexpr static std::ostream& print(std::ostream& os) {
				return os << colorspace::default_color_function_name << "(";
			}
		};
		template<class colorspace, int pos>
		struct print_prefix <colorspace, pos, std::enable_if_t<has_multi_funcname_v<colorspace>>> {
			inline constexpr static std::ostream& print(std::ostream& os) {
				return os << colorspace::default_color_function_name[pos] << "(";
			}
		};
		template<class colorspace, int pos>
		struct print_prefix <colorspace, pos, std::enable_if_t<!has_funcname_v<colorspace>&& has_predefined_v<colorspace>>> {
			inline constexpr static std::ostream& print(std::ostream& os) {
				return os << "color(" << colorspace::default_predefined_name << " ";
			}
		};
		template<int pos, uint8_t I = 0>
		struct print_prefix_type {
			inline constexpr static std::ostream& print(std::ostream& os, uint8_t type) {
				if (I == type) {
					return print_prefix<to_colorspace_t<I>, pos>::print(os);
				}
				return print_prefix_type<pos, I + 1>::print(os, type);
			}
		};
		template<int pos>
		struct print_prefix_type <pos, to_index_v<unknown_colorspace>> {
			inline constexpr static std::ostream& print(std::ostream& os, uint8_t) {
				return os;
			}
		};

		template <class colorspace, typename = void>
		struct output_type {
			using type = colorspace;
			inline constexpr static uint8_t index = to_index_v<type>;
		};
		template <class colorspace>
		struct output_type<colorspace, std::void_t<typename colorspace::serialize_as>> {
			using type = typename colorspace::serialize_as;
			inline constexpr static uint8_t index = to_index_v<type>;
		};
		template<uint8_t I = 0>
		struct get_serialize_type {
			inline constexpr static uint8_t get(uint8_t type) {
				if (I == type) {
					return output_type<to_colorspace_t<I>>::index;
				}
				return get_serialize_type<I + 1>::get(type);
			}
		};
		template<>
		struct get_serialize_type<to_index_v<unknown_colorspace>> {
			inline constexpr static uint8_t get(uint8_t) {
				return 255;
			}
		};
		inline uint8_t get_serialize_type_v(uint8_t type) {
			return get_serialize_type<0>::get(type);
		}
		template<class colorspace>
		using output_type_t = typename output_type<colorspace>::type;

		template<bool legacy> constexpr std::string_view untyped_separator = " ";
		template<> constexpr std::string_view untyped_separator<true> = ", ";
		template<bool legacy> constexpr std::string_view untyped_separator_alpha = " / ";
		template<> constexpr std::string_view untyped_separator_alpha<true> = ", ";
		template<bool legacy>
		inline std::ostream& print_untyped(std::ostream& os, const untyped_color& color) {
			os << color.first[0] << untyped_separator<legacy> << color.first[1] << untyped_separator<legacy> << color.first[2];
			if (color.second != 1) {
				os << untyped_separator_alpha<legacy> << color.second;
			}
			return os;
		}
		inline std::ostream& operator << (std::ostream& os, const untyped_color& color) {
			return print_untyped<false>(os, color);	//as a default
		}
		template<class colorspace>
		inline std::ostream& operator << (std::ostream& os, const typed_color<colorspace>& color) {
			print_prefix<colorspace>::print(os);
			print_untyped<is_legacy_v<colorspace>>(os, static_cast<const untyped_color&>(color));
			os << ")";
		}
		inline void u8_to_hexstr(char result[2], uint8_t d) {
			constexpr char lookup[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
			result[0] = lookup[d / 16];
			result[1] = lookup[d % 16];
		}
		inline std::ostream& operator << (std::ostream& os, const color& c) {
			if (!c) {
				return (os << "[Invalid Color]");
			}
			else {
				if (os.flags() & std::ostream::hex) {
					auto bytes = colorspaces::conv_to_dword::to_dword(c, "rgba").value_or(std::array<uint8_t, 4>{0, 0, 0, 0});
					std::array<char, 10> result;
					result[0] = '#';
					result[9] = '\0';
					for (size_t i = 0; i < 4; i++) {
						u8_to_hexstr(result.data() + 2 * i + 1, bytes[i]);
					}
					return os << result.data();
				}
				auto stype_id = get_serialize_type_v(c.first);
				//std::cout << "###"<<(int)stype_id << std::endl;
				if (stype_id == c.first) {
					if (c.second.second >= 1) {
						print_prefix_type<1>::print(os, c.first);
					}
					else {
						print_prefix_type<0>::print(os, c.first);
					}
					if (dynamic_output_legacy_v(c.first)) {
						print_untyped<true>(os,c.second);
					}
					else {
						print_untyped<false>(os, c.second);
					}
					os << ")";
					return os;
				}
				else {
					return os << c.as(stype_id);
				}
			}
		}
		template<size_t id = 0>
		constexpr size_t longest_namedcolor_name = math::max(longest_namedcolor_name<id + 1>, named_colors::named[id].first.length());
		template<>
		constexpr size_t longest_namedcolor_name<named_colors::named.size()> = 0;
		template<uint8_t func_id = 0>
		struct function_name_info {
			inline constexpr static bool check(const std::string_view& sv)
			{
				if (color_function<func_id>::name == sv)return true;
				return function_name_info<func_id + 1>::check(sv);
			}
			inline constexpr static size_t max_len = math::max<size_t>(color_function<func_id>::name.length(), function_name_info<func_id + 1>::max_len);
		};
		template<>
		struct function_name_info <color_function_cnt> {
			inline constexpr static bool check(const std::string_view& sv)
			{
				if ("color" == sv)return true;
				else return false;
			}
			inline constexpr static size_t max_len = std::string_view("color").length();
		};

		inline std::istream& operator >> (std::istream& is, color& c) {
			size_t read_n = 0;
			std::string cache;
			cache.reserve(function_name_info<0>::max_len + 1);
			while (isspace(is.peek()))is.ignore();

			if (is.peek() == '#') {
				//easiest to tell
				cache.push_back((char)is.get());
				read_n++;
				while (!is.eof() && cache.size() < 10) {
					char chr = (char)is.peek();
					if (!isxdigit(chr))break;
					cache.push_back((char)is.get()); read_n++;
				}
				c = parser::parse_color(cache.c_str());
				if (c)return is;
			}
			else {
				while (!is.eof() && cache.size() < math::max(function_name_info<0>::max_len, longest_namedcolor_name<0>) && isalpha(is.peek())) {
					cache.push_back((char)is.get());
					read_n++;
				}
				if (function_name_info<0>::check(cache)) {
					//is function-like color.
					while (!is.eof() && isblank(is.peek())) {
						is.ignore();
						read_n++;
					}

					if (is.peek() == '(') {
						while (isprint(is.peek()) || isblank(is.peek())) {
							cache.push_back((char)is.get());
							read_n++;
							if (cache.back() == ')')break;
						}
						if (cache.back() == ')') {
							c = parser::parse_color(cache.c_str());
							if (c)return is;
						}
					}
				}
				else {
					//named color. check now.
					c = parser::parse_color(cache.c_str());
					if (c)return is;
				}
			}
			for (size_t i = 0; i < read_n; i++)is.unget();
			c = nullcolor;
			return is;
		}
	}
	namespace colorspaces {
		using io_helpers_impl::operator<<;
		using io_helpers_impl::operator>>;
	}
}
#endif /* CSS_COLOR_IO_HPP */
