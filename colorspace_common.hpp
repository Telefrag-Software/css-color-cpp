#pragma once
#ifndef CSS_COLOR_COLORSPACE_COMMON_HPP
#define CSS_COLOR_COLORSPACE_COMMON_HPP

#include <tuple>
#include <type_traits>
#include "mat.hpp"
#include "element_types.hpp"
namespace css_colors::details::colorspaces {
	
	namespace const_counter {
		struct null_node {};

		template<typename context,typename node, typename tail, typename E = void>
		struct next_node {
			using type = null_node;
		};
		template<typename context, typename node, typename tail>
		using next_node_t = typename next_node<context,node, tail>::type;

		template<typename context, typename head, typename tail, typename E = void>
		struct last_element_in_range {
			using type = typename last_element_in_range<context, next_node_t<context, head, tail>, tail>::type;
		};
		template<typename context,typename head, typename tail>
		struct last_element_in_range<context,head, tail, std::enable_if_t< 
			std::is_same<next_node_t<context,head, tail>, tail>::value || std::is_same<next_node_t<context,head, tail>, null_node>::value>>
		{
			using type = head;
		};
		template<typename context,typename tail>
		using last_element_before = typename last_element_in_range<context,null_node, tail>::type;


		template<typename context,typename elem, typename tail, typename E = void>
		inline constexpr bool is_before = is_before<context, elem, last_element_before<context, tail>>;
		template<typename context,typename elem>
		inline constexpr bool is_before<context,elem, elem, void> = true;
		template<typename context,typename elem>
		inline constexpr bool is_before<context,elem, null_node, typename std::enable_if<!std::is_same<elem, null_node>::value>::type> = false;
		template<typename context,typename tail>
		inline constexpr size_t list_size_before =
			(!std::is_same_v<tail, null_node> && !std::is_same_v<last_element_before<context, tail>, null_node>)
			? list_size_before<context, last_element_before<context, tail>>+1 : 0;

		template<class counter, typename T = void>
		struct close_counter {
			inline constexpr static uint8_t size = list_size_before<counter, T>;
		};
	}
	using const_counter::close_counter;

	struct colorset_list_counter;
	
	template<class colorspace, typename = void> 
	struct to_index {
		inline constexpr static uint8_t value = 255;
	};
	template<class colorspace>
	struct to_index<colorspace, std::void_t<typename colorspace::real_type>> {
		inline constexpr static uint8_t value = to_index<typename colorspace::real_type>::value;
	};
	template<class colorspace>
	inline constexpr uint8_t to_index_v = to_index<colorspace>::value;
	template<uint8_t> struct to_colorspace;
	template<uint8_t id> using to_colorspace_t = typename to_colorspace<id>::type;
	struct color_function_counter;
	template<uint8_t> struct color_function;

	struct predefined_colorspace_counter;
	template<uint8_t> struct predefined_colorspace;

}
#define COUNTER_VAL(context,element) (const_counter::list_size_before<context,element>)
#define COUNTER_INC(context,element)\
namespace const_counter{\
	template<typename tail>\
	struct next_node<context,last_element_before<context,element>,tail,std::enable_if_t<!is_before<context,tail,element>>>\
	{\
		using type = element;\
	};\
}
#define REGISTER_COLORSPACE(colorspace) \
namespace registered_colorspaces{\
	using colorspaces::colorspace;\
}\
template<>\
struct to_index<colorspace,void>{inline constexpr static uint8_t value =  COUNTER_VAL(colorset_list_counter,colorspace);};\
template<>\
struct to_colorspace<COUNTER_VAL(colorset_list_counter,colorspace)>\
{\
	using type = colorspace;\
};\
COUNTER_INC(colorset_list_counter,colorspace);

#define REGISTER_COLORFUNCTION(colorfunction,color_space)\
struct COLFUN##colorfunction##_helper{};\
template<>\
struct color_function<COUNTER_VAL(color_function_counter,COLFUN##colorfunction##_helper)> {\
	inline constexpr static std::string_view name = #colorfunction;\
	using colorspace = color_space;\
};\
COUNTER_INC(color_function_counter,COLFUN##colorfunction##_helper);

#define REGISTER_PERDEFINED(predefined_name,ref_name,color_space)\
struct PREDEFINED##ref_name##_helper{};\
template<>\
struct predefined_colorspace<COUNTER_VAL(predefined_colorspace_counter,PREDEFINED##ref_name##_helper)> {\
	inline constexpr static std::string_view name = #predefined_name;\
	using colorspace = color_space;\
};\
COUNTER_INC(predefined_colorspace_counter,PREDEFINED##ref_name##_helper);
//REGISTER_COLORFUNCTION(abc)
#endif /* CSS_COLOR_COLORSPACE_COMMON_HPP */
