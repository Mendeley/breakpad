
set(BREAKPAD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

if(WIN32)
	# TODO - Windows build system
elseif(APPLE)
	# TODO - Mac build system
elseif(UNIX)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/linux)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/linux/include)
	set(CLIENT_LIB_SRCS
		${CLIENT_SRC_DIR}/handler/exception_handler.cc
		${CLIENT_SRC_DIR}/minidump_writer/linux_dumper.cc
		${CLIENT_SRC_DIR}/minidump_writer/minidump_writer.cc
		${CLIENT_SRC_DIR}/../minidump_file_writer.cc
		${BREAKPAD_SRC_DIR}/common/string_conversion.cc
		${BREAKPAD_SRC_DIR}/common/linux/file_id.cc
		${BREAKPAD_SRC_DIR}/common/linux/guid_creator.cc
		${BREAKPAD_SRC_DIR}/common/convert_UTF.c
	)
	add_library(breakpad ${CLIENT_LIB_SRCS})
endif()

