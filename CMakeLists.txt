# The Board : C++ / WebGPU engine + React design tools
cmake_minimum_required(VERSION 3.16)
project(the_board VERSION 0.1.0 LANGUAGES CXX)

# ═══ SUNRISE_0 N4 — THE GUARD IS LIFTED, AND THE ROAD IS OPEN AGAIN ══════════
#
# SUNSET_2 put a FATAL_ERROR here refusing every non-Emscripten configure,
# because in 7T there was no native twin left to configure. 7T-Music exists
# to bring that twin back, so the refusal is gone and the native Dawn section
# below is live again. The Emscripten branch and the web presets went with
# the twin at tag web-sunset (docs/LAWS.md: WEB_SUNSET).
# `the-board-full-release` is the program; `the-board-full` is its
# diagnostic twin (PORT_2c).
#
# THE DAWN PIN: 56f332d7d8d03f36149f201ab8cce8aee187e8c6 (v20260814.182433) —
# the SAME generation third_party/emdawnwebgpu/PINNED.md pins, and the
# same one third_party/dawn_native_headers/PINNED.md generates from. That is the point: L37's one-generation law asked the native checkout
# to TRACK the web pin, and the unpinned C:/dev/dawn era is what made that a
# promise instead of a fact. Build recipe: docs/reference/DAWN_REFERENCE.md.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CONFIG AXIS ──────────────────────────────────────────────────────────────
# Debug and Release, and nothing else. The axis outlived the reason it was
# born with (SUNSET_0: Dawn's prebuilt static libraries came in exactly those
# two configurations, and nothing links them any more) because PORT_2c gave it
# a second one: Debug is THE DIAGNOSTIC TWIN, -O0 with SAFE_HEAP and the stack
# canary armed; Release is -O2. Multi-config generators choose per build via
# --config; single-config generators choose per tree via CMAKE_BUILD_TYPE.
#
# SUNRISE_0 N4 — THE AXIS GETS ITS FIRST REASON BACK. SUNSET_0 deleted the two
# `set(DAWN_CHECK_CONFIGS ...)` lines with the Dawn section, leaving the axis
# standing on PORT_2c alone. Restoring the Dawn section without them is worse
# than not restoring it: dawn_lib()'s `foreach(CFG ${DAWN_CHECK_CONFIGS})`
# iterates an EMPTY list, so every guard passes, every library path is accepted
# unverified, and a missing Dawn is discovered at link as unresolved symbols
# instead of at configure as a named absolute path. Verified: with the list
# empty this tree configured CLEANLY against a C:/dev/dawn that does not exist
# on this machine at all.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "The diagnostic axis (PORT_2c); also the configs Dawn provides" FORCE)
    set(DAWN_CHECK_CONFIGS Debug Release)
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Build type: Debug (diagnostic) or Release (-O2)" FORCE)
    endif()
    if(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
        message(FATAL_ERROR
            "CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}' — the axis is Debug or Release. PORT_2c reads it: Debug arms -O0 -g plus ASSERTIONS=2, STACK_OVERFLOW_CHECK=2 and SAFE_HEAP=1; Release pins -O2 and strips them. A third name would select neither. Native also links Dawn's prebuilt static libraries, which exist in exactly those two configurations.")
    endif()
    set(DAWN_CHECK_CONFIGS ${CMAKE_BUILD_TYPE})
endif()


set(DAWN_DIR "C:/dev/dawn" CACHE PATH "Dawn source directory")
set(DAWN_BUILD "C:/dev/dawn/out" CACHE PATH "Dawn build directory")

message(STATUS "")
message(STATUS "7T Visualizer (WebGPU/Dawn)")
message(STATUS "  Dawn: ${DAWN_DIR}")
message(STATUS "  Build: ${DAWN_BUILD}")
message(STATUS "")

# Dawn include directories
# N10 — the glfw include row is single-sourced so the sentry below can
# probe the marquee header under the same address the compiler is given.
set(DAWN_GLFW_INCLUDE "${DAWN_DIR}/third_party/glfw3/src/include")

set(DAWN_INCLUDES
    ${DAWN_DIR}/include
    ${DAWN_DIR}/src
    ${DAWN_BUILD}/gen/include
    ${DAWN_BUILD}/gen/src
    ${DAWN_GLFW_INCLUDE}
)

# ─── N10 — THE INCLUDE SENTRY ───────────────────────────────────────────
# The lib guard below names a missing LIBRARY by full concrete path before
# any compilation. Nothing did the same for INCLUDE directories, and the
# failure mode of a dangling include dir is WORSE than a missing lib: it
# does not fail at all when any ambient provider can satisfy the name.
# That is not hypothetical — it is how this tree built for the whole
# campaign. The tag-era row `third_party/glfw/include` does not exist at
# Dawn pin 56f332d7 (FetchContent roots glfw at third_party/glfw3/src),
# and the compile succeeded anyway because vcpkg's USER-WIDE MSBUILD
# INTEGRATION injected its own include path. /showIncludes confessed:
#
#   Note: including file:  C:\vcpkg\installed\x64-windows\include\GLFW/glfw3.h
#
# vcpkg headers against Dawn's pinned glfw3.lib — benign in practice (the
# world ran, dropped_submits=0) but an UNDECLARED MACHINE DEPENDENCY and a
# header/lib generation skew, which is the one-generation law's exact
# hazard (L37) wearing a build-system costume.
#
# The sentry walks every include dir and probes the marquee header, so a
# dangling row is named at configure and the vacuum an ambient provider
# would fill never opens. WHAT IT CANNOT SEE, stated plainly: injection
# ON TOP of valid dirs happens on MSBuild's side of the fence, invisible
# to CMake. That class has exactly one examiner — the Ninja lane, whose
# honesty is that it is not MSBuild and gets no vcpkg integration at all.
foreach(T7_INC_DIR ${DAWN_INCLUDES})
    if(NOT EXISTS "${T7_INC_DIR}")
        message(FATAL_ERROR
            "Missing Dawn include directory: ${T7_INC_DIR} — a dangling "
            "include dir does not fail the compile while an ambient provider "
            "(vcpkg MSBuild integration, a system include) can satisfy the "
            "names; it silently substitutes an unpinned generation. Fix the "
            "row, never the environment.")
    endif()
endforeach()
if(NOT EXISTS "${DAWN_GLFW_INCLUDE}/GLFW/glfw3.h")
    message(FATAL_ERROR
        "GLFW marquee header absent: ${DAWN_GLFW_INCLUDE}/GLFW/glfw3.h — "
        "the directory exists but does not hold glfw3.h, so the compiler "
        "would take GLFW from whatever the environment offers.")
endif()

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
# N6 ADD — pin-new at 56f332d7; in the five targets' closure, so dawn_native references it.
dawn_lib(LIB_DAWN_SHARED_UTILS "src/utils/$<CONFIG>/dawn_shared_utils.lib")

# GLFW from Dawn's third_party
dawn_lib(LIB_GLFW             "third_party/glfw3/src/src/$<CONFIG>/glfw3.lib")

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
dawn_lib(LIB_TINT_UTILS_REFLECTION      "src/tint/$<CONFIG>/tint_utils_reflection.lib")   # N6 ADD

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
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_ANALYSIS "src/tint/$<CONFIG>/tint_lang_spirv_writer_analysis.lib")   # N6 ADD
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
# N6 ADD — pin-new base-tier targets the tag list predates.
dawn_lib(LIB_ABSEIL_BASE_CPU_DETECT "third_party/abseil/absl/base/$<CONFIG>/absl_base_cpu_detect.lib")
dawn_lib(LIB_ABSEIL_HARDENING       "third_party/abseil/absl/base/$<CONFIG>/absl_hardening.lib")
dawn_lib(LIB_ABSEIL_TRACING         "third_party/abseil/absl/base/$<CONFIG>/absl_tracing_internal.lib")

# Optional but often needed
dawn_lib_optional(LIB_ABSEIL_MALLOC "third_party/abseil/absl/base/$<CONFIG>/absl_malloc_internal.lib")

# Strings
dawn_lib(LIB_ABSEIL_STRINGS         "third_party/abseil/absl/strings/$<CONFIG>/absl_strings.lib")
dawn_lib(LIB_ABSEIL_STR_INTERNAL    "third_party/abseil/absl/strings/$<CONFIG>/absl_strings_internal.lib")
# N6 DROP — absl_string_view.lib is absent from the pin's output in BOTH
# configs: string_view is header-only at 56f332d7 and no longer compiles to
# an artifact. No address exists to retarget to.
# N6 ADD — the cordz trio, cord's sampling machinery, pin-new here.
dawn_lib(LIB_ABSEIL_CORDZ_INFO      "third_party/abseil/absl/strings/$<CONFIG>/absl_cordz_info.lib")
dawn_lib(LIB_ABSEIL_CORDZ_HANDLE    "third_party/abseil/absl/strings/$<CONFIG>/absl_cordz_handle.lib")
dawn_lib(LIB_ABSEIL_CORDZ_FUNCTIONS "third_party/abseil/absl/strings/$<CONFIG>/absl_cordz_functions.lib")

# Optional string libs
dawn_lib_optional(LIB_ABSEIL_CORD           "third_party/abseil/absl/strings/$<CONFIG>/absl_cord.lib")
dawn_lib_optional(LIB_ABSEIL_CORD_INTERNAL  "third_party/abseil/absl/strings/$<CONFIG>/absl_cord_internal.lib")
dawn_lib_optional(LIB_ABSEIL_STR_FORMAT     "third_party/abseil/absl/strings/$<CONFIG>/absl_str_format_internal.lib")

# CRC — N6 ADD. absl_cord depends on absl_crc_cord_state, which is why this
# family appears in a closure the tag list never carried.
dawn_lib(LIB_ABSEIL_CRC32C          "third_party/abseil/absl/crc/$<CONFIG>/absl_crc32c.lib")
dawn_lib(LIB_ABSEIL_CRC_CORD_STATE  "third_party/abseil/absl/crc/$<CONFIG>/absl_crc_cord_state.lib")
dawn_lib(LIB_ABSEIL_CRC_INTERNAL    "third_party/abseil/absl/crc/$<CONFIG>/absl_crc_internal.lib")

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
# N6 ADD — absl_demangle_internal's Rust chain, pin-new. Ordered below
# DEMANGLE because each is the next one's dependency.
dawn_lib(LIB_ABSEIL_DEMANGLE_RUST   "third_party/abseil/absl/debugging/$<CONFIG>/absl_demangle_rust.lib")
dawn_lib(LIB_ABSEIL_DECODE_RUST     "third_party/abseil/absl/debugging/$<CONFIG>/absl_decode_rust_punycode.lib")
dawn_lib(LIB_ABSEIL_UTF8_CODE_POINT "third_party/abseil/absl/debugging/$<CONFIG>/absl_utf8_for_code_point.lib")

# N6 DROP — absl_borrowed_fixup_buffer.lib is absent from the pin's output in
# BOTH configs. The guard never reached this entry (it FATALs at the first
# miss, and absl_string_view above stopped it), so only the reconciliation
# could find it.

# Types — N6 ADD.
dawn_lib(LIB_ABSEIL_SOURCE_LOCATION "third_party/abseil/absl/types/$<CONFIG>/absl_source_location.lib")

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
    ${LIB_DAWN_SHARED_UTILS}

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
    ${LIB_TINT_LANG_SPIRV_WRITER_ANALYSIS}
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
    ${LIB_TINT_UTILS_REFLECTION}

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
    ${LIB_ABSEIL_CORDZ_INFO}
    ${LIB_ABSEIL_CORDZ_HANDLE}
    ${LIB_ABSEIL_CORDZ_FUNCTIONS}
    ${LIB_ABSEIL_CRC32C}
    ${LIB_ABSEIL_CRC_CORD_STATE}
    ${LIB_ABSEIL_CRC_INTERNAL}
    ${LIB_ABSEIL_INT128}
    ${LIB_ABSEIL_STACKTRACE}
    ${LIB_ABSEIL_SYMBOLIZE}
    ${LIB_ABSEIL_DEBUGGING}
    ${LIB_ABSEIL_DEMANGLE}
    ${LIB_ABSEIL_DEMANGLE_RUST}
    ${LIB_ABSEIL_DECODE_RUST}
    ${LIB_ABSEIL_UTF8_CODE_POINT}
    ${LIB_ABSEIL_SOURCE_LOCATION}
    ${LIB_ABSEIL_BASE}
    ${LIB_ABSEIL_BASE_CPU_DETECT}
    ${LIB_ABSEIL_HARDENING}
    ${LIB_ABSEIL_TRACING}
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
# ── SUNRISE_0 N7 — /Zc:preprocessor IS LOAD-BEARING, NOT HYGIENE ─────
# src/console/organ_registry.hpp forwards five variadic macros:
#     #define ORGAN_PARAM(...)  ORGAN_PARAM_NS(the_board, __VA_ARGS__)
# and 366 rows of organ_params.inc ride them. MSVC's TRADITIONAL
# preprocessor does not re-split __VA_ARGS__ on commas when it is
# forwarded into another macro — the whole pack arrives as ONE argument,
# so every enrolled dial reaches ORGAN_PARAM_NS with the wrong arity.
# /Zc:preprocessor selects the conforming preprocessor and the 366 rows
# expand as written. Jean's falsifier proved the one-flag cure.
#
# WHY IT SURFACES ONLY HERE. The web twin compiles under emcc, whose
# clang is conforming by default, so the ORGAN forwarders have always
# worked there and this was never a two-twin fact. It is native-only,
# which is why it belongs in the MSVC set and nowhere else.
#
# IT WAS AN ENVIRONMENT VARIABLE UNTIL NOW. The first native build carried
# it through the shell, which means the build was reproducible only for
# someone who knew to export it — L39's own failure shape, one layer up.
# The tree states its own requirement from here.
set(MSVC_COMPILE_OPTS /W3 /wd4251 /wd4275 /wd4244 /wd4267 /EHsc /Zc:preprocessor
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


# ═══════════════════════════════════════════════════════════════════════════════
# ACTIVE CARTRIDGE SELECTION
# ═══════════════════════════════════════════════════════════════════════════════
#
# Controls which render cartridge the the_board target includes.
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
# AN UNSET VALUE HERE IS A COMPILE ERROR, NEVER A FALLBACK. These values are
# pasted into enumerators (INCUBATE_DEMO into demos/matrix.hpp, T7_INSTRUMENTS
# into core/instruments.hpp). CMake drops an empty ENTRY but keeps an entry
# whose VALUE is empty, so a dial left unset arrives DEFINED AND EMPTY:
# #ifndef never fires and the paste names an enumerator that does not exist.
# The definitions themselves are attached below, beside the build stamp.

set(T7_RENDER_CARTRIDGE     "the_board"        CACHE STRING "Render cartridge for the 'the_board' target")
set(THE_BOARD_DEMO                    "full"             CACHE STRING "the_board demo sentence (a DemoCol column of demos/matrix.hpp; compile-time door, DEMO-1). Default IS the golden path.")
set(T7_INSTRUMENTS                    "off"              CACHE STRING "Recurring self-measurement (an InstrumentCol column of core/instruments.hpp): off | meter | full. Default off — the shipped frame pays for no reading nobody reads. 'meter' arms the frame meter + its [METER] table for a measurement session.")

# THE PIN (DRAW_0). The one dial here whose EMPTY IS LEGAL — and empty
# is the default. Empty means the world's master seed is DRAWN at boot
# from the wall clock (cartridge.hpp, boot_seed), so the web twin draws
# with no CMake change at all. A number PINS the world to that seed,
# exactly reproducible: -DT7_WORLD_SEED=42 restores the authored world
# (DEMO_SEED, demos/matrix.hpp).
#
# The paste law above does NOT apply to this one, and the guard below is
# why: an unset value here is never pasted at all. The define is dropped
# entirely, so the source reads #ifdef — not an empty paste that would
# name nothing.
set(T7_WORLD_SEED                     ""                 CACHE STRING "The world's master seed. EMPTY (the default) = DRAWN at boot from the wall clock — each visit is a new world. A number PINS the world to that seed and reproduces it exactly (42 is the authored world, DEMO_SEED in demos/matrix.hpp).")

set(T7_WORLD_SEED_DEFS)
if(NOT T7_WORLD_SEED STREQUAL "")
    set(T7_WORLD_SEED_DEFS T7_WORLD_SEED=${T7_WORLD_SEED})
endif()

message(STATUS "")
message(STATUS "Active cartridges:")
message(STATUS "  the_board -> render: ${T7_RENDER_CARTRIDGE} demo: ${THE_BOARD_DEMO}")
message(STATUS "  Instruments:    ${T7_INSTRUMENTS}")
if(T7_WORLD_SEED STREQUAL "")
    message(STATUS "  World seed:     drawn at boot (wall clock)")
else()
    message(STATUS "  World seed:     pinned to ${T7_WORLD_SEED}")
endif()
message(STATUS "")

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
# THE BOARD — Render cartridge development
# ═══════════════════════════════════════════════════════════════════════════════
#
# The render cartridge (the_board) driven by the BeatClock.
# Uses T7_RENDER_CARTRIDGE.
#
# Build:  cmake --build --preset the-board-full-release  (full chain: CLAUDE.md)
#

# Only the active cartridge's headers → scoped IntelliSense
file(GLOB_RECURSE DUAL_RENDER_HEADERS
    "src/cartridges/${T7_RENDER_CARTRIDGE}/*.hpp"
)

add_executable(the_board
    src/the_board.cpp
    src/external/stb_image.cpp
    ${DUAL_RENDER_HEADERS}
)

target_include_directories(the_board PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# SUNRISE_0 N4 — the native twin's includes and link list. Both come from
# the Dawn section above; neither exists under Emscripten, where the port
# supplies WebGPU and GLFW itself.
target_include_directories(the_board PRIVATE ${DAWN_INCLUDES})
target_link_libraries(the_board PRIVATE ${DAWN_LIBS})

if(MSVC)
    target_compile_options(the_board PRIVATE ${MSVC_COMPILE_OPTS})
    target_link_options(the_board PRIVATE ${MSVC_LINK_OPTS})
    target_compile_definitions(the_board PRIVATE ${MSVC_COMPILE_DEFS})
endif()

# ── PURSE_0 R2 — THE BUILD STAMP ─────────────────────────────────────
#
# The program states its own provenance at boot. The wasm sha names the
# ARTIFACT, and went with the web twin; this names the TREE.
#
# IT LIVES IN THE BUILD DIRECTORY, and that is the whole design decision.
# Every other generated file here is tracked and hand-run; a stamp cannot
# be, because it carries a timestamp — a tracked copy would dirty the tree
# on every build and hold the binding gate's S-6 witness ("porcelain
# clean; HEAD == pushed tip") red forever, and `git describe --dirty`
# would report dirt caused by its own output. tools/build_stamp.py's
# banner carries the full reasoning.
#
# TWO STEPS, AND EACH ANSWERS A DIFFERENT QUESTION. The configure-time run
# guarantees the file EXISTS before the first compile; the custom target is
# always out of date by construction, which is the only way to observe HEAD
# moving WITHOUT a reconfigure (R-E). The script writes only when the
# string differs, so an always-run step costs no relink.
#
# A PROVENANCE INSTRUMENT MUST NOT BE ABLE TO BREAK THE BUILD. With no
# Python interpreter the configure writes an honest "unknown" stamp once
# and no target is added: the program compiles and says it does not know,
# which is the one thing a stamp may say other than the truth.
set(T7_STAMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(T7_STAMP_FILE "${T7_STAMP_DIR}/build_stamp.gen.inc")
set(T7_STAMP_TOOL "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_stamp.py")
file(MAKE_DIRECTORY "${T7_STAMP_DIR}")

# TWO WAYS TO FIND THE INTERPRETER, because one of them is enough to lose
# the instrument silently. find_package is the correct road; find_program
# on both spellings is the net under it (Windows ships `python`, not
# `python3`, and this tree's own build chain is driven from a .bat).
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    set(T7_PYTHON "${Python3_EXECUTABLE}")
else()
    find_program(T7_PYTHON NAMES python3 python)
endif()

if(T7_PYTHON)
    execute_process(COMMAND "${T7_PYTHON}" "${T7_STAMP_TOOL}" "${T7_STAMP_FILE}"
                    RESULT_VARIABLE T7_STAMP_RC)
else()
    set(T7_STAMP_RC 1)
endif()

if(T7_STAMP_RC EQUAL 0)
    add_custom_target(t7_build_stamp
        BYPRODUCTS "${T7_STAMP_FILE}"
        COMMAND "${T7_PYTHON}" "${T7_STAMP_TOOL}" "${T7_STAMP_FILE}"
        COMMENT "build stamp (git describe + UTC; rewritten only on change)"
        VERBATIM)
    add_dependencies(the_board t7_build_stamp)
    message(STATUS "  Build stamp:    ${T7_STAMP_FILE}")
else()
    file(WRITE "${T7_STAMP_FILE}"
         "// GENERATED at configure time — the stamp tool did not run.\n"
         "inline constexpr const char* BUILD_STAMP = \"unknown\";\n")
    message(STATUS "  Build stamp:    unknown (the stamp tool did not run; it says so)")
endif()

target_include_directories(the_board PRIVATE "${T7_STAMP_DIR}")

# THE PROGRAM'S DIALS, RE-HOMED AT WEB_SUNSET. They were the program's on
# both twins and yet lived only in the web branch, so on native they reached
# nothing at all — `the-board-full-release-meter` set T7_INSTRUMENTS=meter and
# the compiler never heard it. Pre-existing defect, found by the W-recon,
# repaired here on the way out rather than deleted along with its only home.
target_compile_definitions(the_board PRIVATE
    INCUBATE_RENDER=${T7_RENDER_CARTRIDGE}
    INCUBATE_DEMO=${THE_BOARD_DEMO}
    T7_INSTRUMENTS=${T7_INSTRUMENTS}
    ${T7_WORLD_SEED_DEFS}   # empty ⇒ nothing pasted ⇒ the seed is drawn
)

add_custom_command(TARGET the_board POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:the_board>/assets
    COMMENT "Copying assets for the_board..."
)

# ═══════════════════════════════════════════════════════════════════════════
# PIVOT_0 E3 — DXC RUNTIME LOGISTICS (Windows native only)
#
# Tint->DXC needs dxcompiler.dll and dxil.dll BESIDE THE EXE at runtime.
# Without dxil.dll the compiler produces unsigned DXIL and the driver
# refuses it; without dxcompiler.dll the `use_dxc` toggle has nothing to
# call. Both must land in the exe directory after a clean build.
#
# P1 — ASSERT-AND-GUARD. Dawn lives OUT OF TREE (DAWN_DIR, default
# C:/dev/dawn) and this repo cannot open it, so whether that build carries
# DXC is a HYPOTHESIS, not a checked fact. The Windows SDK path below is a
# hypothesis too. Therefore: search, and if nothing is found, SAY SO and
# keep configuring. A missing pair must not fail the build — it must
# produce a boot that fails loudly with a log line pointing back here.
#
# Set T7_DXC_DIR at configure time to override the search entirely.
if(WIN32)
    set(T7_DXC_DIR "" CACHE PATH
        "Directory holding dxcompiler.dll and dxil.dll (empty = search)")

    set(T7_DXC_CANDIDATES "")
    if(T7_DXC_DIR)
        list(APPEND T7_DXC_CANDIDATES "${T7_DXC_DIR}")
    endif()
    # Dawn's own output first: if that build set DAWN_USE_BUILT_DXC, the
    # pair sits beside the libs already and matches the Dawn being linked.
    list(APPEND T7_DXC_CANDIDATES
        "${DAWN_BUILD}/Debug"
        "${DAWN_BUILD}/Release"
        "${DAWN_BUILD}"
    )
    # Then the Windows SDK, which ships both. Newest first; the version
    # segment is glob-expanded rather than pinned so a host with a
    # different SDK still resolves.
    file(GLOB T7_SDK_DXC_DIRS
        "C:/Program Files (x86)/Windows Kits/10/bin/*/x64")
    if(T7_SDK_DXC_DIRS)
        list(SORT T7_SDK_DXC_DIRS)
        list(REVERSE T7_SDK_DXC_DIRS)
        list(APPEND T7_DXC_CANDIDATES ${T7_SDK_DXC_DIRS})
    endif()

    set(T7_DXC_FOUND_DIR "")
    foreach(CAND IN LISTS T7_DXC_CANDIDATES)
        if(EXISTS "${CAND}/dxcompiler.dll" AND EXISTS "${CAND}/dxil.dll")
            set(T7_DXC_FOUND_DIR "${CAND}")
            break()
        endif()
    endforeach()

    if(T7_DXC_FOUND_DIR)
        message(STATUS "  DXC runtime: ${T7_DXC_FOUND_DIR} (dxcompiler.dll + dxil.dll)")
        add_custom_command(TARGET the_board POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${T7_DXC_FOUND_DIR}/dxcompiler.dll"
                "${T7_DXC_FOUND_DIR}/dxil.dll"
                $<TARGET_FILE_DIR:the_board>
            COMMENT "PIVOT_0: copying DXC runtime (dxcompiler.dll, dxil.dll)..."
        )
    else()
        # The honest else-branch P1 requires. Not FATAL_ERROR: a tree that
        # will not configure teaches nothing, and the Vulkan plan needs
        # neither dll.
        message(WARNING
            "PIVOT_0: dxcompiler.dll + dxil.dll NOT FOUND. Searched: "
            "${T7_DXC_CANDIDATES}. "
            "The CompilerPlan::D3D12_Dxc boot will fail at device or "
            "pipeline creation until both sit beside the exe. Fixes, in "
            "order of preference: rebuild Dawn with DAWN_USE_BUILT_DXC=ON; "
            "or pass -DT7_DXC_DIR=<dir> naming a directory holding both; "
            "or set kCompilerPlan to Vulkan in src/console/console.hpp, "
            "which needs neither.")
    endif()
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges/${T7_RENDER_CARTRIDGE}"
    PREFIX "Render Cartridge"
    FILES ${DUAL_RENDER_HEADERS}
)
