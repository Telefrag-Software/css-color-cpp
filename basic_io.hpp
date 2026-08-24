#pragma once
#ifndef CSS_COLOR_BASIC_IO_HPP
#define CSS_COLOR_BASIC_IO_HPP

#include <string_view>
#include "const_str.hpp"
namespace css_colors::details {
	namespace parser {
		namespace tokenizer_impl {

			struct string_view_tokenizer :public std::string_view
			{
				using std::string_view::basic_string_view;
				size_t cur = 0;
				size_t cached_token_len = 0;
				inline constexpr bool impl_eof(size_t s = 0)const noexcept {
					return cur + s >= length();
				}
				inline constexpr char get()noexcept {
					return at(cur++);
				}
				inline constexpr void skip(size_t s)noexcept {
					cur += s;
				}
				inline constexpr char peek()const noexcept {
					return at(cur);
				}
				inline constexpr char peek(size_t s)const noexcept {
					return at(cur + s);
				}
				enum CHAR_TYPE {
					ALNUM = 0,
					PUNCT = 1,
					WHITESPACE = 2,
					UNKNOWN = 3
				};

				inline static constexpr CHAR_TYPE char_type(char ch)noexcept
				{
					if (const_isspace(ch))return WHITESPACE;
					if (const_isalnum(ch) || ch == '.' || ch == '+' || ch == '-' || ch == '%')return ALNUM;
					if (const_ispunct(ch))return PUNCT;
					return UNKNOWN;
				}
				inline constexpr void skip_token()noexcept {
					cur += cached_token_len;
					cached_token_len = 0;
				}
				inline constexpr void refresh_token()noexcept
				{
					while (!impl_eof() && char_type(peek()) == UNKNOWN)skip(1);
					if (impl_eof())return;
					switch (char_type(peek())) {
					case UNKNOWN:
						return;
					case PUNCT:
						cached_token_len = 1;
						return;
					case WHITESPACE:
						cached_token_len = 1;
						while (!impl_eof(1) && char_type(peek(1)) == WHITESPACE)skip(1);
						return;
					case ALNUM:
						while (!impl_eof(cached_token_len) && char_type(peek(cached_token_len)) == ALNUM)cached_token_len++;
						return;
					}
				}
				inline constexpr bool eof() noexcept {
					if (cached_token_len == 0)refresh_token();
					return cur == length();
				}
				inline constexpr bool skip_whitespace()noexcept {
					while (!eof() && next_token().empty())skip_token();
					return eof();
				}

				inline constexpr std::string_view next_token() noexcept {
					if (cached_token_len == 0)refresh_token();
					if (impl_eof())return std::string_view();
					if (const_isspace(at(cur)))return std::string_view();
					return substr(cur, cached_token_len);
				}
			};
			inline constexpr std::optional<real> to_int(const std::string_view& token) {
				if (token.empty())return std::nullopt;
				if (token.front() == '+' || token.front() == '-') {
					auto val = to_int(token.substr(1));
					if (!val.has_value())return std::nullopt;
					if (token.front() == '-')val.value() = -val.value();
					return val;
				}

				size_t cur = 0;
				real ret = 0;
				//bool digit_found = (const_isdigit(token.front()));
				while (cur != token.length() && token[cur] >= '0' && token[cur] <= '9') {
					ret = ret * 10 + (token[cur] - '0');
					cur++;
				}
				if (cur != token.length())return std::nullopt;
				return ret;
			}
			inline constexpr std::optional<real> to_real(const std::string_view& token)
			{
				if (token.empty())return std::nullopt;
				if (token.front() == '+' || token.front() == '-') {
					auto val = to_real(token.substr(1));
					if (!val.has_value())return std::nullopt;
					if (token.front() == '-')val.value() = -val.value();
					return val;
				}

				size_t cur = 0;
				real ret = 0;
				bool digit_found = (const_isdigit(token.front()));
				while (cur != token.length() && token[cur] >= '0' && token[cur] <= '9') {
					ret = ret * 10 + (token[cur] - '0');
					cur++;
				}
				if (cur != token.length() && token[cur] == '.') {
					cur++;
					if (cur != token.length() && const_isdigit(token[cur]))digit_found = true;
					real d = (real)0.1;
					while (cur != token.length() && token[cur] >= '0' && token[cur] <= '9') {
						ret = ret + (token[cur] - '0') * d;
						d = d / 10;
						cur++;
					}
				}
				if (!digit_found)return std::nullopt;

				if (cur != token.length() && const_tolower(token[cur]) == 'e') {
					cur++;
					auto e = to_int(token.substr(cur));
					if (!e.has_value())return std::nullopt;
					ret *= math::pow(10, e.value());
				}
				return ret;
			}
		};
		using tokenizer_impl::string_view_tokenizer;
		using tokenizer_impl::to_real;
		using tokenizer_impl::to_int;
	}
};
#endif /* CSS_COLOR_BASIC_IO_HPP */
