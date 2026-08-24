#pragma once
#ifndef CSS_COLOR_ELEMENT_TYPES_HPP
#define CSS_COLOR_ELEMENT_TYPES_HPP

#include <algorithm>
#include <string_view>
#include <cctype>
#include <optional>
#include "const_math.hpp"
#include "const_str.hpp"
#include "basic_io.hpp"
namespace css_colors::details {
	namespace parser {
		namespace element_types {
			struct integer;
			struct degree;

			struct percentage;

			constexpr int no_lower_bound = std::numeric_limits<int>::min();
			constexpr int no_upper_bound = std::numeric_limits<int>::max();

			template<bool t_allow_percentage, int t_factor_num = 1, int t_factor_den = 1,
				int clamp_lb = std::numeric_limits<int>::min(),
				int clamp_ub = std::numeric_limits<int>::max(),
				int default_val = 0
			>
			struct common_range {
				static constexpr bool allow_percentage = t_allow_percentage;
				static constexpr real factor = t_factor_num / t_factor_num;
				static constexpr std::pair<real, real> clamp_range{
					clamp_lb == no_lower_bound ? -std::numeric_limits<real>::infinity() : clamp_lb,
					clamp_ub == no_upper_bound ? std::numeric_limits<real>::infinity() : clamp_ub
				};
				static constexpr real default_value = default_val;
			};
			template<bool t_allow_percentage, int t_factor_num, int t_factor_den, int default_val>
			struct common_range <t_allow_percentage, t_factor_num, t_factor_den, no_lower_bound, no_upper_bound, default_val> {
				static constexpr bool allow_percentage = t_allow_percentage;
				static constexpr real factor = static_cast<real>(t_factor_num) / t_factor_den;
				static constexpr real default_value = default_val;
			};

			using byte_range = common_range<true, 255, 1, no_lower_bound, 255, 0>;
			using unbounded_percentage_range = common_range<true, 1, 1, no_lower_bound, no_upper_bound, 0>;
			using alpha_range = common_range<true, 1, 1, 0, 1, 1>;
			using percent_100_range = common_range<true, 100, 1, 0, 100, 0>;
			using percent_1_range = common_range<true, 1, 1, 0, 1, 0>;

			template<class range>
			struct number {
				inline constexpr static real scale = range::factor;
			};

			using byte = number<byte_range>;
			using unbounded_percentage = number<unbounded_percentage_range>;
			using percent_100 = number<percent_100_range>;
			using percent_1 = number<percent_1_range>;

			using predefined_colorspace_args = std::tuple<unbounded_percentage, unbounded_percentage, unbounded_percentage>;

		}
		template<class element_type, typename = void>
		struct clamp_element {
			inline constexpr static real clamp(const real& val) { return val; }
		};
		template<class range>
		struct clamp_element<element_types::number<range>, std::void_t<decltype(range::clamp_range)>> {
			inline constexpr static real clamp(const real& val) {
				return math::min(math::max(val, range::clamp_range.first), range::clamp_range.second);
			}
		};
		template<>
		struct clamp_element<element_types::percentage> {
			inline constexpr static real clamp(const real& val) {
				return math::min(math::max(val, (real)0), (real)1);
			}
		};
		template<class element_type, typename E = void>
		struct parse_element_helper {
			inline static constexpr std::optional<real> parse(const std::string_view& token) {
				return std::nullopt;
			}
		};
		template<class type, typename = void>
		inline constexpr bool should_clamp_v = false;
		template<class range>
		inline constexpr bool should_clamp_v<element_types::number<range>, std::void_t<decltype(range::clamp_range)>> = true;
		template<>
		inline constexpr bool should_clamp_v<element_types::percentage> = true;

		template<class range>
		struct parse_element_helper <element_types::number<range>, void> {
			inline static constexpr std::optional<real> parse(const std::string_view& token) {
				if (const_iequal(token, "none"))return range::default_value;
				real val = 0;
				if (range::allow_percentage && token.back() == '%') {
					auto r = to_real(token.substr(0, token.length() - 1));
					if (!r.has_value())return std::nullopt;
					val = r.value() * range::factor / 100;
				}
				else {
					auto r = to_real(token);
					if (!r.has_value())return std::nullopt;
					val = r.value();
				}
				return val;
			}
		};
		template<>
		struct parse_element_helper <element_types::degree, void> {
			inline static constexpr std::optional<real> parse(const std::string_view& token) {
				if (const_iequal(token, "none"))return static_cast<real>(0);
				real val = static_cast<real>(0);
				if (const_iequal(token.substr(0, token.length() - 3), "deg"))return parse(token.substr(0, token.length() - 3));
				auto r = to_real(token);
				if (!r.has_value())return std::nullopt;
				val = math::fmod_wrap<360>(r.value());
				return val;
			}
		};

		template<>
		struct parse_element_helper <element_types::percentage, void> {
			inline static constexpr std::optional<real> parse(const std::string_view& token) {
				if (const_iequal(token, "none"))return static_cast<real>(0);
				//real val = static_cast<real>(0);
				if (token.back() != '%')return std::nullopt;
				auto r = to_real(token.substr(0, token.length() - 1));
				if (!r)return std::nullopt;
				//val = r.value() / 100;

				return r.value() / 100;
			}
		};

		template<class element_type>
		inline constexpr std::optional<real> parse_element(string_view_tokenizer& tokens) {
			auto ret = parse_element_helper<element_type>::parse(tokens.next_token());
			if (ret)tokens.skip_token(); else return std::nullopt;
			real val = ret.value();
			if constexpr (should_clamp_v<element_type>) {
				val = clamp_element<element_type>::clamp(val);
			}
			return val;
		}
	}
}
#endif /* CSS_COLOR_ELEMENT_TYPES_HPP */
