cmake_minimum_required(VERSION 3.5...4.2.1 FATAL_ERROR)

set(LIBRARY_NAME "Parser")

project(CSSColorParser VERSION 1.0.0 LANGUAGES CXX)

add_library(${LIBRARY_NAME} INTERFACE)

target_sources(${LIBRARY_NAME}
	INTERFACE
		FILE_SET HEADERS
		TYPE HEADERS
		BASE_DIRS
			${CMAKE_CURRENT_SOURCE_DIR}
		FILES
			basic_io.hpp
			colorspace.hpp
			colorspace_common.hpp
			colorspace_convert.hpp
			const_math.hpp
			const_str.hpp
			css_color.hpp
			element_types.hpp
			io.hpp
			mat.hpp
			named_colors.hpp
			parser.hpp
			utilities.hpp
			spaces/a98_rgb.hpp
			spaces/displayp3.hpp
			spaces/hsl.hpp
			spaces/hsv.hpp
			spaces/hwb.hpp
			spaces/lab.hpp
			spaces/lch.hpp
			spaces/linear_rgb.hpp
			spaces/oklab.hpp
			spaces/oklch.hpp
			spaces/prophoto.hpp
			spaces/rec2020.hpp
			spaces/srgb.hpp
			spaces/xyz.hpp
)

target_include_directories(${LIBRARY_NAME}
	INTERFACE
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

include(GNUInstallDirs)

set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(VERSION_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(PROJECT_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(NAMESPACE "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	"${VERSION_CONFIG}" COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	"cmake/Config.cmake.in"
	"${PROJECT_CONFIG}"
	INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)

set(INSTALL_TARGETS
	${LIBRARY_NAME}
)

install(
	TARGETS ${INSTALL_TARGETS}
	EXPORT "${TARGETS_EXPORT_NAME}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	FILE_SET HEADERS
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(
	FILES "${PROJECT_CONFIG}" "${VERSION_CONFIG}"
	DESTINATION "${CONFIG_INSTALL_DIR}"
)

install(
	EXPORT "${TARGETS_EXPORT_NAME}"
	NAMESPACE "${NAMESPACE}"
	DESTINATION "${CONFIG_INSTALL_DIR}"
)
