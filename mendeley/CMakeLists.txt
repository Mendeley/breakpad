project(breakpad)

set(BREAKPAD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

set(UNIX_DUMPSYMS_COMMON_SRC
    ${BREAKPAD_SRC_DIR}/common/dwarf/bytereader.cc
    ${BREAKPAD_SRC_DIR}/common/dwarf/dwarf2diehandler.cc
    ${BREAKPAD_SRC_DIR}/common/dwarf/dwarf2reader.cc
    ${BREAKPAD_SRC_DIR}/common/dwarf/elf_reader.cc
    ${BREAKPAD_SRC_DIR}/common/dwarf_cfi_to_module.cc
    ${BREAKPAD_SRC_DIR}/common/dwarf_cu_to_module.cc
    ${BREAKPAD_SRC_DIR}/common/dwarf_line_to_module.cc
    ${BREAKPAD_SRC_DIR}/common/language.cc
    ${BREAKPAD_SRC_DIR}/common/module.cc
    ${BREAKPAD_SRC_DIR}/common/stabs_reader.cc
    ${BREAKPAD_SRC_DIR}/common/stabs_to_module.cc
)

if(WIN32)
	add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
        set(DEBUG_ACCESS_SDK_DIR "$ENV{VSINSTALLDIR}/DIA SDK")
        if (NOT (EXISTS "${DEBUG_ACCESS_SDK_DIR}"))
            # The Debug Access SDK is required to build the dump_syms tool.
            # It ships with the pro versions of Visual Studio 2008 and 2010 but not
            # the express versions. It is available separately as part of the Windows SDK
            # however.
            message(FATAL_ERROR "Debug Access SDK not found in ${DEBUG_ACCESS_SDK_DIR}")
        endif()
        include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/windows/include ${DEBUG_ACCESS_SDK_DIR}/include)
        set(BREAKPADCOMMON_LIB_SRCS
			${BREAKPAD_SRC_DIR}/common/windows/guid_string.cc
			${BREAKPAD_SRC_DIR}/common/windows/string_utils.cc
        )
        set(BREAKPADCOMMON_LIBS Imagehlp)

        set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/windows)
        set(BREAKPAD_LIB_SRCS
			${CLIENT_SRC_DIR}/handler/exception_handler.cc
			${CLIENT_SRC_DIR}/crash_generation/client_info.cc
			${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
			${CLIENT_SRC_DIR}/crash_generation/crash_generation_server.cc
			${CLIENT_SRC_DIR}/crash_generation/minidump_generator.cc
		)
        ide_group_sources("${BREAKPAD_LIB_SRCS}")

        set(DUMPSYMS_SRCS
                # pdb_source_line_writer.cc is linked into the `dump_syms` target rather than
                # `breakpad_common` to avoid a runtime dependency on the DIA SDK libs
                # in the breakpad client library
                ${BREAKPAD_SRC_DIR}/common/windows/pdb_source_line_writer.cc
                ${BREAKPAD_SRC_DIR}/tools/windows/dump_syms/dump_syms.cc
        )

        find_library(DIA_SDK_GUID_LIB diaguids PATHS ${DEBUG_ACCESS_SDK_DIR}/lib)
		set(DUMPSYMS_LIBS ${DIA_SDK_GUID_LIB})

elseif(APPLE)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/mac)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/mac/include)
	add_definitions(-DHAVE_MACH_O_NLIST_H)
	file(GLOB BREAKPADCOMMON_LIB_SRCS
		${BREAKPAD_SRC_DIR}/common/convert_UTF.c
		${BREAKPAD_SRC_DIR}/common/mac/MachIPC.mm
		${BREAKPAD_SRC_DIR}/common/mac/bootstrap_compat.cc
		${BREAKPAD_SRC_DIR}/common/mac/file_id.cc
		${BREAKPAD_SRC_DIR}/common/mac/macho_id.cc
		${BREAKPAD_SRC_DIR}/common/mac/macho_utilities.cc
		${BREAKPAD_SRC_DIR}/common/mac/macho_walker.cc
		${BREAKPAD_SRC_DIR}/common/mac/string_utilities.cc
		${BREAKPAD_SRC_DIR}/common/mac/arch_utilities.cc
		${BREAKPAD_SRC_DIR}/common/md5.cc
		${BREAKPAD_SRC_DIR}/common/string_conversion.cc
		${BREAKPAD_SRC_DIR}/common/path_helper.cc
	)

	set(BREAKPAD_LIB_SRCS
		${BREAKPAD_SRC_DIR}/client/minidump_file_writer.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_server.cc
		${CLIENT_SRC_DIR}/handler/breakpad_nlist_64.cc
		${CLIENT_SRC_DIR}/handler/dynamic_images.cc
		${CLIENT_SRC_DIR}/handler/exception_handler.cc
		${CLIENT_SRC_DIR}/handler/minidump_generator.cc
	)

	set(LIB_DUMPSYMS_SRCS ${UNIX_DUMPSYMS_COMMON_SRC}
		${BREAKPAD_SRC_DIR}/common/mac/dump_syms.cc
		${BREAKPAD_SRC_DIR}/common/mac/macho_reader.cc
    )

    set(DUMPSYMS_SRCS
		${BREAKPAD_SRC_DIR}/tools/mac/dump_syms/dump_syms_tool.cc
	)

	find_library(FOUNDATION_LIB Foundation REQUIRED)
	set(DUMPSYMS_LIBS ${FOUNDATION_LIB})

elseif(UNIX)
	add_definitions(-DHAVE_A_OUT_H)
	set(CLIENT_SRC_DIR ${BREAKPAD_SRC_DIR}/client/linux)
	include_directories(${BREAKPAD_SRC_DIR} ${BREAKPAD_SRC_DIR}/third_party/linux/include)
	set(BREAKPADCOMMON_LIB_SRCS
		${BREAKPAD_SRC_DIR}/common/convert_UTF.c
		${BREAKPAD_SRC_DIR}/common/linux/file_id.cc
		${BREAKPAD_SRC_DIR}/common/linux/guid_creator.cc
		${BREAKPAD_SRC_DIR}/common/linux/memory_mapped_file.cc
		${BREAKPAD_SRC_DIR}/common/linux/safe_readlink.cc
		${BREAKPAD_SRC_DIR}/common/string_conversion.cc
	)
	find_package(Threads)
	set(BREAKPADCOMMON_LIBS ${CMAKE_THREAD_LIBS_INIT})

	set(BREAKPAD_LIB_SRCS
		${BREAKPAD_SRC_DIR}/client/minidump_file_writer.cc
		${CLIENT_SRC_DIR}/crash_generation/crash_generation_client.cc
		${CLIENT_SRC_DIR}/handler/exception_handler.cc
		${CLIENT_SRC_DIR}/log/log.cc
		${CLIENT_SRC_DIR}/minidump_writer/linux_dumper.cc
		${CLIENT_SRC_DIR}/minidump_writer/linux_ptrace_dumper.cc
		${CLIENT_SRC_DIR}/minidump_writer/minidump_writer.cc
	)
	
	set(DUMPSYMS_SRCS ${UNIX_DUMPSYMS_COMMON_SRC}
		${BREAKPAD_SRC_DIR}/common/linux/dump_symbols.cc
		${BREAKPAD_SRC_DIR}/common/linux/elf_symbols_to_module.cc
    )

    set(DUMPSYMS_SRCS
		${BREAKPAD_SRC_DIR}/tools/linux/dump_syms/dump_syms.cc
	)
endif()

add_library(breakpad_common ${BREAKPADCOMMON_LIB_SRCS})
target_link_libraries(breakpad_common ${BREAKPADCOMMON_LIBS})

# breakpad - client library for capturing minidumps when a crash
# occurs
add_library(breakpad ${BREAKPAD_LIB_SRCS})
target_link_libraries(breakpad breakpad_common)

# dump_syms - Tool for producing cross-platform .sym files
# from a binary with debug info
add_library(dumpsyms STATIC ${LIB_DUMPSYMS_SRCS})
target_link_libraries(dumpsyms PUBLIC breakpad_common ${DUMPSYMS_LIBS})

add_executable(dump_syms ${DUMPSYMS_SRCS})
target_link_libraries(dump_syms breakpad_common dumpsyms ${DUMPSYMS_LIBS})

# minidump_stackwalk - A tool for producing a stacktrace
# from a minidump using debug symbol files produced by dump_syms
#
# This tool does not currently build on Windows. A version pre-built
# with Cygwin is available from http://stackoverflow.com/questions/11302258/build-google-breakpad-stackwalk
if (UNIX)
  set(LIBDISASM_DIR ${BREAKPAD_SRC_DIR}/third_party/libdisasm)
  set(LIBDISASM_SRCS
    ${LIBDISASM_DIR}/ia32_implicit.c
    ${LIBDISASM_DIR}/ia32_insn.c
    ${LIBDISASM_DIR}/ia32_invariant.c
    ${LIBDISASM_DIR}/ia32_modrm.c
    ${LIBDISASM_DIR}/ia32_invariant.c
    ${LIBDISASM_DIR}/ia32_opcode_tables.c
    ${LIBDISASM_DIR}/ia32_operand.c
    ${LIBDISASM_DIR}/ia32_reg.c
    ${LIBDISASM_DIR}/ia32_settings.c
    ${LIBDISASM_DIR}/x86_disasm.c
    ${LIBDISASM_DIR}/x86_format.c
    ${LIBDISASM_DIR}/x86_imm.c
    ${LIBDISASM_DIR}/x86_insn.c
    ${LIBDISASM_DIR}/x86_misc.c
    ${LIBDISASM_DIR}/x86_operand_list.c
  )
  add_library(disasm ${LIBDISASM_SRCS})

  set(PROCESSOR_SRC_DIR ${BREAKPAD_SRC_DIR}/processor)
  set(MINIDUMP_PROCESSOR_SRCS
    ${PROCESSOR_SRC_DIR}/basic_source_line_resolver.cc
    ${PROCESSOR_SRC_DIR}/basic_code_modules.cc
    ${PROCESSOR_SRC_DIR}/call_stack.cc
    ${PROCESSOR_SRC_DIR}/cfi_frame_info.cc
    ${PROCESSOR_SRC_DIR}/convert_old_arm64_context.cc
    ${PROCESSOR_SRC_DIR}/disassembler_x86.cc
    ${PROCESSOR_SRC_DIR}/exploitability.cc
    ${PROCESSOR_SRC_DIR}/exploitability_win.cc
    ${PROCESSOR_SRC_DIR}/exploitability_linux.cc
    #${PROCESSOR_SRC_DIR}/external_symbol_supplier.cc
    ${PROCESSOR_SRC_DIR}/logging.cc
    ${PROCESSOR_SRC_DIR}/minidump.cc
    ${PROCESSOR_SRC_DIR}/minidump_processor.cc
    ${PROCESSOR_SRC_DIR}/minidump_stackwalk.cc
    ${PROCESSOR_SRC_DIR}/pathname_stripper.cc
    ${PROCESSOR_SRC_DIR}/process_state.cc
    ${PROCESSOR_SRC_DIR}/proc_maps_linux.cc
    ${PROCESSOR_SRC_DIR}/simple_symbol_supplier.cc
    ${PROCESSOR_SRC_DIR}/stackwalker.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_amd64.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_arm.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_arm64.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_ppc.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_ppc64.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_sparc.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_x86.cc
    ${PROCESSOR_SRC_DIR}/stackwalker_mips.cc
    ${PROCESSOR_SRC_DIR}/source_line_resolver_base.cc
    ${PROCESSOR_SRC_DIR}/tokenize.cc
    ${PROCESSOR_SRC_DIR}/dump_object.cc
    ${PROCESSOR_SRC_DIR}/dump_context.cc
    ${PROCESSOR_SRC_DIR}/stack_frame_symbolizer.cc
    ${PROCESSOR_SRC_DIR}/stackwalk_common.cc
    ${PROCESSOR_SRC_DIR}/symbolic_constants_win.cc
  )
    set(MINIDUMP_STACKWALK_SRCS
        ${PROCESSOR_SRC_DIR}/minidump_stackwalk.cc
    )
    add_library(minidump_processor STATIC ${MINIDUMP_PROCESSOR_SRCS})
    target_include_directories(
        minidump_processor
        PUBLIC
            ${BREAKPAD_SRC_DIR}
    )
    target_link_libraries(minidump_processor PUBLIC disasm breakpad_common)

    add_executable(minidump_stackwalk ${MINIDUMP_STACKWALK_SRCS})
    target_link_libraries(minidump_stackwalk PRIVATE minidump_processor)
endif()

enable_testing()
add_subdirectory(test)

set_target_properties(
	breakpad_common
	breakpad
	dump_syms
	# ONLY on UNIX:
	#disasm
	#minidump_stackwalk
	PROPERTIES
		FOLDER "Externals/breakpad"
)


foreach(tgt breakpad_client breakpad_common disasm dumpsyms dump_syms minidump_processor minidump_stackwalk)
    if(TARGET ${tgt})
        set_target_properties(${tgt} PROPERTIES AUTOMOC OFF)
        target_compile_options(
            ${tgt}
            PRIVATE
                -Wno-redeclared-class-member
                -Wno-deprecated-declarations
                -Wno-tautological-compare
                -Wno-tautological-constant-out-of-range-compare
        )
    endif()
endforeach()

