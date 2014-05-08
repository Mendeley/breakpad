set(BREAKPAD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Copy all entries in IN_LIST except those matching PATTERN
# to OUT_VAR
function(filter_list OUT_VAR IN_LIST PATTERN)
	foreach(ITEM ${IN_LIST})
		if (NOT (${ITEM} MATCHES ${PATTERN}))
			set(OUT_LIST ${OUT_LIST} ${ITEM})
		endif()
	endforeach()
	set(${OUT_VAR} ${OUT_LIST} PARENT_SCOPE)
endfunction()

if(WIN32)
        set(DEBUG_ACCESS_SDK_DIR "$ENV{VSINSTALLDIR}/DIA SDK")
        if (NOT (EXISTS "${DEBUG_ACCESS_SDK_DIR}"))
            message(FATAL_ERROR "Debug Access SDK not found in ${DEBUG_ACCESS_SDK_DIR}")
        endif()
        include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/windows/include ${DEBUG_ACCESS_SDK_DIR}/include)
        set(BREAKPADCOMMON_LIB_SRCS
                ${BREAKPAD_SRC_DIR}/common/windows/guid_string.cc
                ${BREAKPAD_SRC_DIR}/common/windows/string_utils.cc
        )
        filter_list(BREAKPADCOMMON_LIB_SRCS "${BREAKPADCOMMON_LIB_SRCS}" "_unittest")
        add_library(breakpad_common ${BREAKPADCOMMON_LIB_SRCS})
        target_link_libraries(breakpad_common Imagehlp)

        set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/windows)
        set(BREAKPAD_LIB_SRCS
		${CLIENT_SRC_DIR}/handler/exception_handler.cc
		${CLIENT_SRC_DIR}/crash_generation/client_info.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_server.cc
                ${CLIENT_SRC_DIR}/crash_generation/minidump_generator.cc
	)
        ide_group_sources("${BREAKPAD_LIB_SRCS}")
        add_library(breakpad ${BREAKPAD_LIB_SRCS})
        target_link_libraries(breakpad breakpad_common)

        find_library(DIA_SDK_GUID_LIB diaguids PATHS ${DEBUG_ACCESS_SDK_DIR}/lib)

        set(DUMPSYMS_SRCS
                # pdb_source_line_writer.cc is linked into the `dump_syms` target rather than
                # `breakpad_common` to avoid a runtime dependency on the DIA SDK libs
                # in the breakpad client library
                ${BREAKPAD_SRC_DIR}/common/windows/pdb_source_line_writer.cc
                ${BREAKPAD_SRC_DIR}/tools/windows/dump_syms/dump_syms.cc
        )
        add_executable(dump_syms ${DUMPSYMS_SRCS})
        target_link_libraries(dump_syms breakpad_common ${DIA_SDK_GUID_LIB})
elseif(APPLE)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/mac)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/mac/include)
	add_definitions(-DHAVE_MACH_O_NLIST_H)
	file(GLOB BREAKPADCOMMON_LIB_SRCS
		${BREAKPAD_SRC_DIR}/common/*.cc
		${BREAKPAD_SRC_DIR}/common/dwarf/*.cc
		${BREAKPAD_SRC_DIR}/common/mac/*.cc
		${BREAKPAD_SRC_DIR}/common/mac/*.mm
	)
	filter_list(BREAKPADCOMMON_LIB_SRCS "${BREAKPADCOMMON_LIB_SRCS}" "_unittest")
	add_library(breakpad_common ${BREAKPADCOMMON_LIB_SRCS})

        set(BREAKPAD_LIB_SRCS
			${BREAKPAD_SRC_DIR}/client/minidump_file_writer.cc
			${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
			${CLIENT_SRC_DIR}/crash_generation/crash_generation_server.cc
			${CLIENT_SRC_DIR}/handler/breakpad_nlist_64.cc
			${CLIENT_SRC_DIR}/handler/dynamic_images.cc
			${CLIENT_SRC_DIR}/handler/exception_handler.cc
			${CLIENT_SRC_DIR}/handler/minidump_generator.cc
	)
	add_library(breakpad ${BREAKPAD_LIB_SRCS})
	target_link_libraries(breakpad breakpad_common)

	set(DUMPSYMS_SRCS
		${BREAKPAD_SRC_DIR}/tools/mac/dump_syms/dump_syms_tool.mm
	)
	find_library(FOUNDATION_LIB Foundation REQUIRED)
        add_executable(dump_syms ${DUMPSYMS_SRCS})
        target_link_libraries(dump_syms breakpad_common ${FOUNDATION_LIB})
elseif(UNIX)
	add_definitions(-DHAVE_A_OUT_H)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/linux)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/linux/include)
	set(BREAKPADCOMMON_LIB_SRCS
		${BREAKPAD_SRC_DIR}/common/convert_UTF.c
		${BREAKPAD_SRC_DIR}/common/language.cc
		${BREAKPAD_SRC_DIR}/common/module.cc
		${BREAKPAD_SRC_DIR}/common/stabs_reader.cc
		${BREAKPAD_SRC_DIR}/common/stabs_to_module.cc
		${BREAKPAD_SRC_DIR}/common/dwarf_cfi_to_module.cc
		${BREAKPAD_SRC_DIR}/common/dwarf_line_to_module.cc
		${BREAKPAD_SRC_DIR}/common/dwarf_cu_to_module.cc
		${BREAKPAD_SRC_DIR}/common/dwarf/bytereader.cc
		${BREAKPAD_SRC_DIR}/common/dwarf/dwarf2diehandler.cc
		${BREAKPAD_SRC_DIR}/common/dwarf/dwarf2reader.cc
		${BREAKPAD_SRC_DIR}/common/linux/dump_symbols.cc
		${BREAKPAD_SRC_DIR}/common/linux/elf_symbols_to_module.cc
		${BREAKPAD_SRC_DIR}/common/linux/file_id.cc
		${BREAKPAD_SRC_DIR}/common/linux/guid_creator.cc
		${BREAKPAD_SRC_DIR}/common/linux/memory_mapped_file.cc
		${BREAKPAD_SRC_DIR}/common/linux/safe_readlink.cc
		${BREAKPAD_SRC_DIR}/common/string_conversion.cc
	)
	add_library(breakpad_common ${BREAKPADCOMMON_LIB_SRCS})

	set(BREAKPAD_LIB_SRCS
		${CLIENT_SRC_DIR}/../minidump_file_writer.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
		${CLIENT_SRC_DIR}/handler/exception_handler.cc
		${CLIENT_SRC_DIR}/log/log.cc
		${CLIENT_SRC_DIR}/minidump_writer/linux_dumper.cc
		${CLIENT_SRC_DIR}/minidump_writer/linux_ptrace_dumper.cc
		${CLIENT_SRC_DIR}/minidump_writer/minidump_writer.cc
	)
	add_library(breakpad ${BREAKPAD_LIB_SRCS})
	target_link_libraries(breakpad breakpad_common)

	set(DUMPSYMS_SRCS
		${BREAKPAD_SRC_DIR}/tools/linux/dump_syms/dump_syms.cc)
	add_executable(dump_syms ${DUMPSYMS_SRCS})
	target_link_libraries(dump_syms breakpad_common)
endif()

