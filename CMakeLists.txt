# 7T-Pawns : C++ / WebGPU engine + React design tools
cmake_minimum_required(VERSION 3.16)
project(7t_visualizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CONFIG AXIS ──────────────────────────────────────────────────────────────
# 7t links Dawn's prebuilt static libraries, which exist in exactly two
# configurations: Debug and Release. The config set here mirrors that fact.
# Multi-config generators (Visual Studio) choose per build via --config;
# single-config generators (Ninja trees) choose per tree via CMAKE_BUILD_TYPE.
# DAWN_CHECK_CONFIGS is the list the library guards verify at configure time.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Configs provided by the Dawn build" FORCE)
    set(DAWN_CHECK_CONFIGS Debug Release)
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Build type: Debug or Release (the configs Dawn provides)" FORCE)
    endif()
    if(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
        message(FATAL_ERROR
            "CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}' — the axis is Debug or Release for both twins. Native links Dawn's prebuilt static libraries, which exist in exactly those two configurations; the web twin links no Dawn at all and pins -O2 in its own compile and link options regardless of config, so there the two names are a convention this check keeps honest.")
    endif()
    set(DAWN_CHECK_CONFIGS ${CMAKE_BUILD_TYPE})
endif()

# ═══════════════════════════════════════════════════════════════════════════════
# ACTIVE CARTRIDGE SELECTION
# ═══════════════════════════════════════════════════════════════════════════════
#
# Controls which render cartridge the incubator target includes.
# IntelliSense will only index it — no noise from other cartridges.
#
# Change here to switch focus. No need to edit .cpp files.
#
#   Render cartridges:   src/cartridges/<name>/cartridge.hpp
#
#   Available render cartridges: the_board
#
# The analysis side is the BeatClock (src/analysis/beat_clock.hpp,
# CUT_1c) — no analysis cartridge selection exists anymore.
#
# ABOVE THE EMSCRIPTEN GUARD, deliberately. Both twins paste these values
# into enumerators (INCUBATE_DEMO into demos/matrix.hpp, T7_INSTRUMENTS into
# core/instruments.hpp). CMake drops an empty ENTRY but keeps an entry whose
# VALUE is empty, so a dial left unset arrives DEFINED AND EMPTY: #ifndef
# never fires and the paste names an enumerator that does not exist. An
# unset value here is a compile error in every TU, never a fallback.

set(INCUBATOR_DUAL_RENDER_CARTRIDGE   "the_board"        CACHE STRING "Render cartridge for the 'incubator_dual' target")
set(THE_BOARD_DEMO                    "full"             CACHE STRING "the_board demo sentence (a DemoCol column of demos/matrix.hpp; compile-time door, DEMO-1). Default IS the golden path.")
set(T7_INSTRUMENTS                    "off"              CACHE STRING "Recurring self-measurement (an InstrumentCol column of core/instruments.hpp): off | meter | full. Default off — the shipped frame pays for no reading nobody reads. 'meter' arms the frame meter + its [METER] table for a measurement session.")

message(STATUS "")
message(STATUS "Active cartridges:")
message(STATUS "  incubator_dual -> render: ${INCUBATOR_DUAL_RENDER_CARTRIDGE} demo: ${THE_BOARD_DEMO}")
message(STATUS "  Instruments:    ${T7_INSTRUMENTS}")
message(STATUS "")

# ═══════════════════════════════════════════════════════════════════════════════
# DAWN CONFIGURATION — NATIVE ONLY (PORT_1c)
# The web build gets its WebGPU from the emdawnwebgpu port and its GLFW
# from contrib.glfw3; none of the Dawn/Tint/abseil static libraries
# below exist there, and dawn_lib() would FATAL_ERROR on their absence.
# ═══════════════════════════════════════════════════════════════════════════════
if(NOT EMSCRIPTEN)

set(DAWN_DIR "C:/dev/dawn" CACHE PATH "Dawn source directory")
set(DAWN_BUILD "C:/dev/dawn/out" CACHE PATH "Dawn build directory")

message(STATUS "")
message(STATUS "7T Visualizer (WebGPU/Dawn)")
message(STATUS "  Dawn: ${DAWN_DIR}")
message(STATUS "  Build: ${DAWN_BUILD}")
message(STATUS "")

# Dawn include directories
set(DAWN_INCLUDES
    ${DAWN_DIR}/include
    ${DAWN_DIR}/src
    ${DAWN_BUILD}/gen/include
    ${DAWN_BUILD}/gen/src
    ${DAWN_DIR}/third_party/glfw/include
)

# ═══════════════════════════════════════════════════════════════════════════════
# LIBRARY DISCOVERY FUNCTIONS
# ═══════════════════════════════════════════════════════════════════════════════

# Verifies the library exists for every config in DAWN_CHECK_CONFIGS at
# configure time; the $<CONFIG> in PATH resolves per build.
function(dawn_lib VAR PATH)
    foreach(CFG ${DAWN_CHECK_CONFIGS})
        string(REPLACE "$<CONFIG>" "${CFG}" CONCRETE "${DAWN_BUILD}/${PATH}")
        if(NOT EXISTS "${CONCRETE}")
            message(FATAL_ERROR "Missing Dawn library: ${CONCRETE}")
        endif()
    endforeach()
    set(${VAR} "${DAWN_BUILD}/${PATH}" PARENT_SCOPE)
endfunction()

# An optional present in some configs but not all is an error: the optional
# set must be identical across configs, or the link truth differs per config.
function(dawn_lib_optional VAR PATH)
    set(FOUND_ALL TRUE)
    set(FOUND_ANY FALSE)
    foreach(CFG ${DAWN_CHECK_CONFIGS})
        string(REPLACE "$<CONFIG>" "${CFG}" CONCRETE "${DAWN_BUILD}/${PATH}")
        if(EXISTS "${CONCRETE}")
            set(FOUND_ANY TRUE)
        else()
            set(FOUND_ALL FALSE)
        endif()
    endforeach()
    if(FOUND_ALL)
        set(${VAR} "${DAWN_BUILD}/${PATH}" PARENT_SCOPE)
        message(STATUS "  Found optional: ${PATH}")
    elseif(FOUND_ANY)
        message(FATAL_ERROR
            "Optional Dawn library exists in some configs but not all: ${PATH}")
    else()
        set(${VAR} "" PARENT_SCOPE)
    endif()
endfunction()

message(STATUS "Discovering Dawn libraries...")

# ═══════════════════════════════════════════════════════════════════════════════
# CORE DAWN LIBRARIES (Required)
# Per docs/past docs/DAWN_REFERENCE.md - these are the required core libraries
# ═══════════════════════════════════════════════════════════════════════════════

dawn_lib(LIB_DAWN_NATIVE      "src/dawn/native/$<CONFIG>/dawn_native.lib")
dawn_lib(LIB_DAWN_PROC        "src/dawn/$<CONFIG>/dawn_proc.lib")
dawn_lib(LIB_DAWN_GLFW        "src/dawn/glfw/$<CONFIG>/dawn_glfw.lib")
dawn_lib(LIB_DAWN_COMMON      "src/dawn/common/$<CONFIG>/dawn_common.lib")
dawn_lib(LIB_DAWN_PLATFORM    "src/dawn/platform/$<CONFIG>/dawn_platform.lib")

# Dawn utils (contains SystemHandle)
dawn_lib(LIB_DAWN_SYSTEM_UTILS "src/dawn/utils/$<CONFIG>/dawn_system_utils.lib")

# GLFW from Dawn's third_party
dawn_lib(LIB_GLFW             "third_party/glfw/src/$<CONFIG>/glfw3.lib")

# ═══════════════════════════════════════════════════════════════════════════════
# TINT LIBRARIES (Shader Compiler)
# ═══════════════════════════════════════════════════════════════════════════════

# Tint API
dawn_lib(LIB_TINT_API                   "src/tint/$<CONFIG>/tint_api.lib")
dawn_lib(LIB_TINT_API_COMMON            "src/tint/$<CONFIG>/tint_api_common.lib")

# Tint utils (foundational)
dawn_lib(LIB_TINT_UTILS                 "src/tint/$<CONFIG>/tint_utils.lib")
dawn_lib(LIB_TINT_UTILS_ICE             "src/tint/$<CONFIG>/tint_utils_ice.lib")
dawn_lib(LIB_TINT_UTILS_DIAGNOSTIC      "src/tint/$<CONFIG>/tint_utils_diagnostic.lib")
dawn_lib(LIB_TINT_UTILS_SYMBOL          "src/tint/$<CONFIG>/tint_utils_symbol.lib")
dawn_lib(LIB_TINT_UTILS_STRCONV         "src/tint/$<CONFIG>/tint_utils_strconv.lib")
dawn_lib(LIB_TINT_UTILS_CONTAINERS      "src/tint/$<CONFIG>/tint_utils_containers.lib")
dawn_lib(LIB_TINT_UTILS_MACROS          "src/tint/$<CONFIG>/tint_utils_macros.lib")
dawn_lib(LIB_TINT_UTILS_MATH            "src/tint/$<CONFIG>/tint_utils_math.lib")
dawn_lib(LIB_TINT_UTILS_MEMORY          "src/tint/$<CONFIG>/tint_utils_memory.lib")
dawn_lib(LIB_TINT_UTILS_TEXT            "src/tint/$<CONFIG>/tint_utils_text.lib")
dawn_lib(LIB_TINT_UTILS_TEXT_GENERATOR  "src/tint/$<CONFIG>/tint_utils_text_generator.lib")
dawn_lib(LIB_TINT_UTILS_RTTI            "src/tint/$<CONFIG>/tint_utils_rtti.lib")
dawn_lib(LIB_TINT_UTILS_BYTES           "src/tint/$<CONFIG>/tint_utils_bytes.lib")
dawn_lib(LIB_TINT_UTILS_SYSTEM          "src/tint/$<CONFIG>/tint_utils_system.lib")

# Tint core language
dawn_lib(LIB_TINT_LANG_CORE             "src/tint/$<CONFIG>/tint_lang_core.lib")
dawn_lib(LIB_TINT_LANG_CORE_CONSTANT    "src/tint/$<CONFIG>/tint_lang_core_constant.lib")
dawn_lib(LIB_TINT_LANG_CORE_TYPE        "src/tint/$<CONFIG>/tint_lang_core_type.lib")
dawn_lib(LIB_TINT_LANG_CORE_INTRINSIC   "src/tint/$<CONFIG>/tint_lang_core_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_CORE_IR          "src/tint/$<CONFIG>/tint_lang_core_ir.lib")
dawn_lib(LIB_TINT_LANG_CORE_IR_TRANSFORM "src/tint/$<CONFIG>/tint_lang_core_ir_transform.lib")

# Additional Tint core IR libs (may be needed)
dawn_lib_optional(LIB_TINT_LANG_CORE_IR_ANALYSIS "src/tint/$<CONFIG>/tint_lang_core_ir_analysis.lib")
dawn_lib_optional(LIB_TINT_LANG_CORE_IR_TYPE     "src/tint/$<CONFIG>/tint_lang_core_ir_type.lib")

# Tint WGSL (source language)
dawn_lib(LIB_TINT_LANG_WGSL             "src/tint/$<CONFIG>/tint_lang_wgsl.lib")
dawn_lib(LIB_TINT_LANG_WGSL_AST         "src/tint/$<CONFIG>/tint_lang_wgsl_ast.lib")
dawn_lib(LIB_TINT_LANG_WGSL_PROGRAM     "src/tint/$<CONFIG>/tint_lang_wgsl_program.lib")
dawn_lib(LIB_TINT_LANG_WGSL_SEM         "src/tint/$<CONFIG>/tint_lang_wgsl_sem.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER      "src/tint/$<CONFIG>/tint_lang_wgsl_reader.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER_PARSER "src/tint/$<CONFIG>/tint_lang_wgsl_reader_parser.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER_LOWER "src/tint/$<CONFIG>/tint_lang_wgsl_reader_lower.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER_PROGRAM_TO_IR "src/tint/$<CONFIG>/tint_lang_wgsl_reader_program_to_ir.lib")
dawn_lib(LIB_TINT_LANG_WGSL_RESOLVER    "src/tint/$<CONFIG>/tint_lang_wgsl_resolver.lib")
dawn_lib(LIB_TINT_LANG_WGSL_INSPECTOR   "src/tint/$<CONFIG>/tint_lang_wgsl_inspector.lib")
dawn_lib(LIB_TINT_LANG_WGSL_IR          "src/tint/$<CONFIG>/tint_lang_wgsl_ir.lib")
dawn_lib(LIB_TINT_LANG_WGSL_INTRINSIC   "src/tint/$<CONFIG>/tint_lang_wgsl_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER      "src/tint/$<CONFIG>/tint_lang_wgsl_writer.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_AST_PRINTER "src/tint/$<CONFIG>/tint_lang_wgsl_writer_ast_printer.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_COMMON "src/tint/$<CONFIG>/tint_lang_wgsl_writer_common.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM "src/tint/$<CONFIG>/tint_lang_wgsl_writer_ir_to_program.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_RAISE "src/tint/$<CONFIG>/tint_lang_wgsl_writer_raise.lib")

# Tint HLSL (D3D12 backend - required on Windows)
dawn_lib(LIB_TINT_LANG_HLSL             "src/tint/$<CONFIG>/tint_lang_hlsl.lib")
dawn_lib(LIB_TINT_LANG_HLSL_IR          "src/tint/$<CONFIG>/tint_lang_hlsl_ir.lib")
dawn_lib(LIB_TINT_LANG_HLSL_TYPE        "src/tint/$<CONFIG>/tint_lang_hlsl_type.lib")
dawn_lib(LIB_TINT_LANG_HLSL_INTRINSIC   "src/tint/$<CONFIG>/tint_lang_hlsl_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER      "src/tint/$<CONFIG>/tint_lang_hlsl_writer.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER_COMMON "src/tint/$<CONFIG>/tint_lang_hlsl_writer_common.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER_PRINTER "src/tint/$<CONFIG>/tint_lang_hlsl_writer_printer.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER_RAISE "src/tint/$<CONFIG>/tint_lang_hlsl_writer_raise.lib")

# Tint SPIRV (Vulkan backend)
dawn_lib(LIB_TINT_LANG_SPIRV            "src/tint/$<CONFIG>/tint_lang_spirv.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_IR         "src/tint/$<CONFIG>/tint_lang_spirv_ir.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_TYPE       "src/tint/$<CONFIG>/tint_lang_spirv_type.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_INTRINSIC  "src/tint/$<CONFIG>/tint_lang_spirv_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER     "src/tint/$<CONFIG>/tint_lang_spirv_writer.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_COMMON "src/tint/$<CONFIG>/tint_lang_spirv_writer_common.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_PRINTER "src/tint/$<CONFIG>/tint_lang_spirv_writer_printer.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_RAISE "src/tint/$<CONFIG>/tint_lang_spirv_writer_raise.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER     "src/tint/$<CONFIG>/tint_lang_spirv_reader.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER_COMMON "src/tint/$<CONFIG>/tint_lang_spirv_reader_common.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER_LOWER "src/tint/$<CONFIG>/tint_lang_spirv_reader_lower.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER_PARSER "src/tint/$<CONFIG>/tint_lang_spirv_reader_parser.lib")

# Optional SPIRV validation
dawn_lib_optional(LIB_TINT_LANG_SPIRV_VALIDATE "src/tint/$<CONFIG>/tint_lang_spirv_validate.lib")

# Tint null backend (required for Dawn)
dawn_lib(LIB_TINT_LANG_NULL_WRITER        "src/tint/$<CONFIG>/tint_lang_null_writer.lib")
dawn_lib(LIB_TINT_LANG_NULL_WRITER_COMMON "src/tint/$<CONFIG>/tint_lang_null_writer_common.lib")
dawn_lib(LIB_TINT_LANG_NULL_WRITER_RAISE  "src/tint/$<CONFIG>/tint_lang_null_writer_raise.lib")

# ═══════════════════════════════════════════════════════════════════════════════
# ABSEIL LIBRARIES (Google C++ utilities - Dawn dependency)
# ═══════════════════════════════════════════════════════════════════════════════

# Core abseil
dawn_lib(LIB_ABSEIL_BASE            "third_party/abseil/absl/base/$<CONFIG>/absl_base.lib")
dawn_lib(LIB_ABSEIL_THROW           "third_party/abseil/absl/base/$<CONFIG>/absl_throw_delegate.lib")
dawn_lib(LIB_ABSEIL_SPINLOCK        "third_party/abseil/absl/base/$<CONFIG>/absl_spinlock_wait.lib")
dawn_lib(LIB_ABSEIL_RAW_LOG         "third_party/abseil/absl/base/$<CONFIG>/absl_raw_logging_internal.lib")
dawn_lib(LIB_ABSEIL_LOG_SEVERITY    "third_party/abseil/absl/base/$<CONFIG>/absl_log_severity.lib")

# Optional but often needed
dawn_lib_optional(LIB_ABSEIL_MALLOC "third_party/abseil/absl/base/$<CONFIG>/absl_malloc_internal.lib")

# Strings
dawn_lib(LIB_ABSEIL_STRINGS         "third_party/abseil/absl/strings/$<CONFIG>/absl_strings.lib")
dawn_lib(LIB_ABSEIL_STR_INTERNAL    "third_party/abseil/absl/strings/$<CONFIG>/absl_strings_internal.lib")
dawn_lib(LIB_ABSEIL_STRING_VIEW     "third_party/abseil/absl/strings/$<CONFIG>/absl_string_view.lib")

# Optional string libs
dawn_lib_optional(LIB_ABSEIL_CORD           "third_party/abseil/absl/strings/$<CONFIG>/absl_cord.lib")
dawn_lib_optional(LIB_ABSEIL_CORD_INTERNAL  "third_party/abseil/absl/strings/$<CONFIG>/absl_cord_internal.lib")
dawn_lib_optional(LIB_ABSEIL_STR_FORMAT     "third_party/abseil/absl/strings/$<CONFIG>/absl_str_format_internal.lib")

# Numeric
dawn_lib(LIB_ABSEIL_INT128          "third_party/abseil/absl/numeric/$<CONFIG>/absl_int128.lib")

# Synchronization
dawn_lib(LIB_ABSEIL_SYNCHRONIZATION "third_party/abseil/absl/synchronization/$<CONFIG>/absl_synchronization.lib")
dawn_lib(LIB_ABSEIL_GRAPHCYCLES     "third_party/abseil/absl/synchronization/$<CONFIG>/absl_graphcycles_internal.lib")
dawn_lib(LIB_ABSEIL_KERNEL_TIMEOUT  "third_party/abseil/absl/synchronization/$<CONFIG>/absl_kernel_timeout_internal.lib")

# Time
dawn_lib(LIB_ABSEIL_TIME            "third_party/abseil/absl/time/$<CONFIG>/absl_time.lib")
dawn_lib(LIB_ABSEIL_TIME_ZONE       "third_party/abseil/absl/time/$<CONFIG>/absl_time_zone.lib")
dawn_lib(LIB_ABSEIL_CIVIL_TIME      "third_party/abseil/absl/time/$<CONFIG>/absl_civil_time.lib")

# Debugging
dawn_lib(LIB_ABSEIL_STACKTRACE      "third_party/abseil/absl/debugging/$<CONFIG>/absl_stacktrace.lib")
dawn_lib(LIB_ABSEIL_SYMBOLIZE       "third_party/abseil/absl/debugging/$<CONFIG>/absl_symbolize.lib")
dawn_lib(LIB_ABSEIL_DEMANGLE        "third_party/abseil/absl/debugging/$<CONFIG>/absl_demangle_internal.lib")
dawn_lib(LIB_ABSEIL_DEBUGGING       "third_party/abseil/absl/debugging/$<CONFIG>/absl_debugging_internal.lib")

# Additional debugging support (for BorrowedFixupBuffer)
dawn_lib(LIB_ABSEIL_BORROWED_FIXUP "third_party/abseil/absl/debugging/$<CONFIG>/absl_borrowed_fixup_buffer.lib")

# Hash
dawn_lib(LIB_ABSEIL_HASH            "third_party/abseil/absl/hash/$<CONFIG>/absl_hash.lib")
dawn_lib(LIB_ABSEIL_CITY            "third_party/abseil/absl/hash/$<CONFIG>/absl_city.lib")

# Container support
dawn_lib_optional(LIB_ABSEIL_RAW_HASH_SET   "third_party/abseil/absl/container/$<CONFIG>/absl_raw_hash_set.lib")
dawn_lib_optional(LIB_ABSEIL_HASHTABLEZ     "third_party/abseil/absl/container/$<CONFIG>/absl_hashtablez_sampler.lib")

# Profiling
dawn_lib_optional(LIB_ABSEIL_EXPONENTIAL "third_party/abseil/absl/profiling/$<CONFIG>/absl_exponential_biased.lib")

# ═══════════════════════════════════════════════════════════════════════════════
# SPIRV-TOOLS
# ═══════════════════════════════════════════════════════════════════════════════

dawn_lib(LIB_SPIRV_TOOLS        "third_party/spirv-tools/source/$<CONFIG>/SPIRV-Tools.lib")
dawn_lib(LIB_SPIRV_TOOLS_OPT    "third_party/spirv-tools/source/opt/$<CONFIG>/SPIRV-Tools-opt.lib")


# ═══════════════════════════════════════════════════════════════════════════════
# LINK LIBRARIES
#
# IMPORTANT: Link order matters for static libraries!
# Libraries must come AFTER the libraries that depend on them.
# So we go: Application -> High-level -> Mid-level -> Low-level -> System
#
# Defined before any target so all executables can reference DAWN_LIBS.
# ═══════════════════════════════════════════════════════════════════════════════

# Collect optional libraries (only add if they exist)
set(OPTIONAL_LIBS)
foreach(lib
    LIB_TINT_LANG_CORE_IR_ANALYSIS
    LIB_TINT_LANG_CORE_IR_TYPE
    LIB_TINT_LANG_SPIRV_VALIDATE
    LIB_ABSEIL_MALLOC
    LIB_ABSEIL_CORD
    LIB_ABSEIL_CORD_INTERNAL
    LIB_ABSEIL_STR_FORMAT
    LIB_ABSEIL_RAW_HASH_SET
    LIB_ABSEIL_HASHTABLEZ
    LIB_ABSEIL_EXPONENTIAL
)
    if(${lib})
        list(APPEND OPTIONAL_LIBS ${${lib}})
    endif()
endforeach()

set(DAWN_LIBS
    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 1: Dawn Core (highest level)
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_DAWN_NATIVE}
    ${LIB_DAWN_PROC}
    ${LIB_DAWN_GLFW}
    ${LIB_DAWN_COMMON}
    ${LIB_DAWN_PLATFORM}
    ${LIB_DAWN_SYSTEM_UTILS}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 2: Tint API (shader compilation entry point)
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_TINT_API}
    ${LIB_TINT_API_COMMON}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 3: Tint Language Backends (WGSL in, HLSL/SPIRV out)
    # ─────────────────────────────────────────────────────────────────────────
    # WGSL (source language)
    ${LIB_TINT_LANG_WGSL}
    ${LIB_TINT_LANG_WGSL_AST}
    ${LIB_TINT_LANG_WGSL_PROGRAM}
    ${LIB_TINT_LANG_WGSL_SEM}
    ${LIB_TINT_LANG_WGSL_READER}
    ${LIB_TINT_LANG_WGSL_READER_PARSER}
    ${LIB_TINT_LANG_WGSL_READER_LOWER}
    ${LIB_TINT_LANG_WGSL_READER_PROGRAM_TO_IR}
    ${LIB_TINT_LANG_WGSL_RESOLVER}
    ${LIB_TINT_LANG_WGSL_INSPECTOR}
    ${LIB_TINT_LANG_WGSL_IR}
    ${LIB_TINT_LANG_WGSL_INTRINSIC}
    ${LIB_TINT_LANG_WGSL_WRITER}
    ${LIB_TINT_LANG_WGSL_WRITER_AST_PRINTER}
    ${LIB_TINT_LANG_WGSL_WRITER_COMMON}
    ${LIB_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM}
    ${LIB_TINT_LANG_WGSL_WRITER_RAISE}

    # HLSL (D3D12 backend - Windows)
    ${LIB_TINT_LANG_HLSL}
    ${LIB_TINT_LANG_HLSL_IR}
    ${LIB_TINT_LANG_HLSL_TYPE}
    ${LIB_TINT_LANG_HLSL_INTRINSIC}
    ${LIB_TINT_LANG_HLSL_WRITER}
    ${LIB_TINT_LANG_HLSL_WRITER_COMMON}
    ${LIB_TINT_LANG_HLSL_WRITER_PRINTER}
    ${LIB_TINT_LANG_HLSL_WRITER_RAISE}

    # SPIRV (Vulkan backend)
    ${LIB_TINT_LANG_SPIRV}
    ${LIB_TINT_LANG_SPIRV_IR}
    ${LIB_TINT_LANG_SPIRV_TYPE}
    ${LIB_TINT_LANG_SPIRV_INTRINSIC}
    ${LIB_TINT_LANG_SPIRV_WRITER}
    ${LIB_TINT_LANG_SPIRV_WRITER_COMMON}
    ${LIB_TINT_LANG_SPIRV_WRITER_PRINTER}
    ${LIB_TINT_LANG_SPIRV_WRITER_RAISE}
    ${LIB_TINT_LANG_SPIRV_READER}
    ${LIB_TINT_LANG_SPIRV_READER_COMMON}
    ${LIB_TINT_LANG_SPIRV_READER_LOWER}
    ${LIB_TINT_LANG_SPIRV_READER_PARSER}

    # Null backend (required)
    ${LIB_TINT_LANG_NULL_WRITER}
    ${LIB_TINT_LANG_NULL_WRITER_COMMON}
    ${LIB_TINT_LANG_NULL_WRITER_RAISE}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 4: Tint Core (shared infrastructure)
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_TINT_LANG_CORE}
    ${LIB_TINT_LANG_CORE_IR}
    ${LIB_TINT_LANG_CORE_IR_TRANSFORM}
    ${LIB_TINT_LANG_CORE_CONSTANT}
    ${LIB_TINT_LANG_CORE_TYPE}
    ${LIB_TINT_LANG_CORE_INTRINSIC}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 5: Tint Utilities (lowest level Tint code)
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_TINT_UTILS}
    ${LIB_TINT_UTILS_ICE}
    ${LIB_TINT_UTILS_DIAGNOSTIC}
    ${LIB_TINT_UTILS_SYMBOL}
    ${LIB_TINT_UTILS_STRCONV}
    ${LIB_TINT_UTILS_CONTAINERS}
    ${LIB_TINT_UTILS_MACROS}
    ${LIB_TINT_UTILS_MATH}
    ${LIB_TINT_UTILS_MEMORY}
    ${LIB_TINT_UTILS_TEXT}
    ${LIB_TINT_UTILS_TEXT_GENERATOR}
    ${LIB_TINT_UTILS_RTTI}
    ${LIB_TINT_UTILS_BYTES}
    ${LIB_TINT_UTILS_SYSTEM}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 6: SPIRV-Tools
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_SPIRV_TOOLS_OPT}
    ${LIB_SPIRV_TOOLS}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 7: Abseil (foundational C++ utilities)
    # Order: high-level -> low-level
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_ABSEIL_HASH}
    ${LIB_ABSEIL_CITY}
    ${LIB_ABSEIL_SYNCHRONIZATION}
    ${LIB_ABSEIL_GRAPHCYCLES}
    ${LIB_ABSEIL_KERNEL_TIMEOUT}
    ${LIB_ABSEIL_TIME}
    ${LIB_ABSEIL_TIME_ZONE}
    ${LIB_ABSEIL_CIVIL_TIME}
    ${LIB_ABSEIL_STRINGS}
    ${LIB_ABSEIL_STR_INTERNAL}
    ${LIB_ABSEIL_STRING_VIEW}
    ${LIB_ABSEIL_INT128}
    ${LIB_ABSEIL_STACKTRACE}
    ${LIB_ABSEIL_BORROWED_FIXUP}
    ${LIB_ABSEIL_SYMBOLIZE}
    ${LIB_ABSEIL_DEBUGGING}
    ${LIB_ABSEIL_DEMANGLE}
    ${LIB_ABSEIL_BASE}
    ${LIB_ABSEIL_SPINLOCK}
    ${LIB_ABSEIL_RAW_LOG}
    ${LIB_ABSEIL_LOG_SEVERITY}
    ${LIB_ABSEIL_THROW}

    # Optional abseil libs
    ${OPTIONAL_LIBS}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 8: GLFW
    # ─────────────────────────────────────────────────────────────────────────
    ${LIB_GLFW}

    # ─────────────────────────────────────────────────────────────────────────
    # LEVEL 9: Windows System Libraries (lowest level)
    # ─────────────────────────────────────────────────────────────────────────
    dxgi.lib
    d3d12.lib
    dxguid.lib
    user32.lib
    kernel32.lib
    onecore.lib
    gdi32.lib
    shell32.lib
    winmm.lib
)

# Shared MSVC options (applied per-target below)
set(MSVC_COMPILE_OPTS /W3 /wd4251 /wd4275 /wd4244 /wd4267 /EHsc
    "$<$<CONFIG:Release>:/Z7>")
# Release links produce a full PDB with codegen unchanged: /OPT:REF and
# /OPT:ICF are the plain-Release defaults that /DEBUG would otherwise
# disable. The PDB never ships; it is archived beside the tagged exe.
set(MSVC_LINK_OPTS
    "$<$<CONFIG:Release>:/DEBUG:FULL>"
    "$<$<CONFIG:Release>:/OPT:REF>"
    "$<$<CONFIG:Release>:/OPT:ICF>"
    "$<$<CONFIG:Release>:/INCREMENTAL:NO>")
set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)

endif() # NOT EMSCRIPTEN — end of the native Dawn section

# ═══════════════════════════════════════════════════════════════════════════════
# BUILD INFO
# ═══════════════════════════════════════════════════════════════════════════════

message(STATUS "")
message(STATUS "Configuration complete:")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")


# ═══════════════════════════════════════════════════════════════════════════════
# INCUBATOR DUAL — Render cartridge development
# ═══════════════════════════════════════════════════════════════════════════════
#
# The render cartridge (the_board) driven by the BeatClock.
# Uses INCUBATOR_DUAL_RENDER_CARTRIDGE.
#
# Build:  cmake --build . --target incubator_dual
# Run:    ./incubator_dual.exe
#

# Only the active cartridge's headers → scoped IntelliSense
file(GLOB_RECURSE DUAL_RENDER_HEADERS
    "src/cartridges/${INCUBATOR_DUAL_RENDER_CARTRIDGE}/*.hpp"
)

add_executable(incubator_dual
    src/incubator_dual.cpp
    src/external/stb_image.cpp
    ${DUAL_RENDER_HEADERS}
)

target_include_directories(incubator_dual PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${DAWN_INCLUDES}
)

target_link_libraries(incubator_dual PRIVATE ${DAWN_LIBS})

if(MSVC)
    target_compile_options(incubator_dual PRIVATE ${MSVC_COMPILE_OPTS})
    target_link_options(incubator_dual PRIVATE ${MSVC_LINK_OPTS})
    target_compile_definitions(incubator_dual PRIVATE
        ${MSVC_COMPILE_DEFS}
        INCUBATE_RENDER=${INCUBATOR_DUAL_RENDER_CARTRIDGE}
        INCUBATE_DEMO=${THE_BOARD_DEMO}
        T7_INSTRUMENTS=${T7_INSTRUMENTS}
    )
endif()

if(EMSCRIPTEN)
    # ── PORT_1c: the web twin ────────────────────────────────────────────
    # GLFW verbatim via contrib.glfw3 (emscripten ports registry,
    # >= 3.1.55). WebGPU via emdawnwebgpu; the default is Emscripten's
    # built-in "remote" port (--use-port=emdawnwebgpu, auto-downloads a
    # pinned package). Override with a downloaded port file from a Dawn
    # release: -DEMDAWNWEBGPU_PORT=path/to/emdawnwebgpu.port.py
    # (the source-tree port file is NOT usable un-built — it checks for
    # generated headers; use the release package or the remote port).
    set(EMDAWNWEBGPU_PORT "emdawnwebgpu" CACHE STRING
        "emdawnwebgpu port for --use-port: the built-in remote port name, or a path to emdawnwebgpu.port.py from a Dawn release")
    set(T7_WEB_PORTS
        "--use-port=contrib.glfw3"
        "--use-port=${EMDAWNWEBGPU_PORT}"
    )

    # PORT_2c — THE DIAGNOSTIC TWIN. -O2 strips the checks that would
    # have NAMED the PORT_2a stack overflow in one line instead of
    # costing a hypothesis; Debug arms them. The split rides the CONFIG
    # AXIS that already exists (Debug|Release, enforced at the head of
    # this file) rather than a new dial — one fact, one home. Debug is
    # slow by construction: SAFE_HEAP instruments every load and store.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(T7_WEB_OPT -O0 -g)
        set(T7_WEB_DIAG
            "-sASSERTIONS=2"           # verbose runtime checks + stack trace on abort
            "-sSTACK_OVERFLOW_CHECK=2" # full canary + guard page; names the overflow
            "-sSAFE_HEAP=1"            # traps out-of-bounds / misaligned heap access
        )
    else()
        set(T7_WEB_OPT -O2)
        set(T7_WEB_DIAG)
    endif()

    # Identical defines to the native target (MSVC block above does not
    # apply under the emscripten toolchain).
    target_compile_definitions(incubator_dual PRIVATE
        INCUBATE_RENDER=${INCUBATOR_DUAL_RENDER_CARTRIDGE}
        INCUBATE_DEMO=${THE_BOARD_DEMO}
        T7_INSTRUMENTS=${T7_INSTRUMENTS}
    )
    target_compile_options(incubator_dual PRIVATE ${T7_WEB_PORTS} ${T7_WEB_OPT})
    # ONE PRELOAD, AND IT IS PROGRAM (EXHIBIT_0 U3). The MEMFS mapping
    # left is the shader: the loader's 2nd search entry is
    # "src/cartridges/the_board/realization/world.wgsl" and it hits this
    # explicit file mapping verbatim from CWD "/" (verified
    # renderer.hpp:1250-1251). world.wgsl is rebuilt when the PROGRAM
    # changes, so it belongs in the bundle the program ships as.
    #
    # The assets@/assets mapping is GONE. It put the entire exhibition
    # inside the_board.data, which meant every repainted wall and every
    # new canvas cost a link — the compiler woken by a curator. The
    # paintings and the soundtrack are now plain files beside the
    # program, assembled by tools/web_dist.py and fetched by URL at
    # runtime (gallery.hpp, the __EMSCRIPTEN__ forms). Nothing else read
    # /assets: the gallery scan was its only consumer on this twin.
    # PORT_2a — THE STACK. Emscripten's default STACK_SIZE dropped from
    # 5MB to 64KB in 3.1.27; Windows gives the native twin 1MB. At -O2
    # stack-overflow checking is OFF, so an overflow does not abort — it
    # writes past the stack into the heap and dlmalloc's metadata is the
    # first casualty, which is why the fault surfaces inside $malloc from
    # unrelated call sites. 4MB restores native parity with headroom.
    # Costs address space, not runtime: with ALLOW_MEMORY_GROWTH the
    # stack is a reservation carved out of the initial memory.
    # EXHIBIT_0 U2 — THE NETWORK IS A BYTE SOURCE. -sFETCH arms
    # emscripten_fetch, which is how the web twin reads exhibition.json
    # and each painting: the bundle carries the program, the network
    # carries the exhibition. Native is unaffected — this whole block is
    # behind if(EMSCRIPTEN).
    target_link_options(incubator_dual PRIVATE
        ${T7_WEB_PORTS}
        "-sALLOW_MEMORY_GROWTH=1"
        "-sSTACK_SIZE=4MB"
        "-sFETCH"
        ${T7_WEB_OPT}
        ${T7_WEB_DIAG}
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges/the_board/realization/world.wgsl@/src/cartridges/the_board/realization/world.wgsl"
    )
    set_target_properties(incubator_dual PROPERTIES
        OUTPUT_NAME "the_board"
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/web"
    )
else()
    add_custom_command(TARGET incubator_dual POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:incubator_dual>/assets
        COMMENT "Copying assets for incubator_dual..."
    )
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges/${INCUBATOR_DUAL_RENDER_CARTRIDGE}"
    PREFIX "Render Cartridge"
    FILES ${DUAL_RENDER_HEADERS}
)
