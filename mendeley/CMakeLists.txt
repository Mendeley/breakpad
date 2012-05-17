
set(BREAKPAD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

if(WIN32)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/windows)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/windows/include)
	set(CLIENT_LIB_SRCS
		${CLIENT_SRC_DIR}/handler/exception_handler.cc
		${CLIENT_SRC_DIR}/crash_generation/client_info.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_server.cc
		${CLIENT_SRC_DIR}/crash_generation/minidump_generator.cc
		${BREAKPAD_SRC_DIR}/common/windows/guid_string.cc
	)
	add_library(breakpad ${CLIENT_LIB_SRCS})
elseif(APPLE)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/mac)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/mac/include)
	set(CLIENT_LIB_SRCS
			${BREAKPAD_SRC_DIR}/client/minidump_file_writer.cc
			${BREAKPAD_SRC_DIR}/common/convert_UTF.c
			${BREAKPAD_SRC_DIR}/common/mac/bootstrap_compat.cc
			${BREAKPAD_SRC_DIR}/common/mac/MachIPC.mm
			${BREAKPAD_SRC_DIR}/common/mac/file_id.cc
			${BREAKPAD_SRC_DIR}/common/mac/macho_id.cc
			${BREAKPAD_SRC_DIR}/common/mac/macho_utilities.cc
			${BREAKPAD_SRC_DIR}/common/mac/macho_walker.cc
			${BREAKPAD_SRC_DIR}/common/mac/string_utilities.cc
			${BREAKPAD_SRC_DIR}/common/md5.cc
			${BREAKPAD_SRC_DIR}/common/string_conversion.cc
			${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
			${CLIENT_SRC_DIR}/crash_generation/crash_generation_server.cc
			${CLIENT_SRC_DIR}/handler/breakpad_nlist_64.cc
			${CLIENT_SRC_DIR}/handler/dynamic_images.cc
			${CLIENT_SRC_DIR}/handler/exception_handler.cc
			${CLIENT_SRC_DIR}/handler/minidump_generator.cc
	)
	add_library(breakpad ${CLIENT_LIB_SRCS})
	target_link_libraries(breakpad)
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

