# 7T-Pawns : C++ / WebGPU engine + React design tools
cmake_minimum_required(VERSION 3.16)
project(7t_visualizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ═══════════════════════════════════════════════════════════════════════════════
# DAWN CONFIGURATION
# ═══════════════════════════════════════════════════════════════════════════════

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
    ${DAWN_DIR}/third_party/glfw/include
)

# ═══════════════════════════════════════════════════════════════════════════════
# LIBRARY DISCOVERY FUNCTIONS
# ═══════════════════════════════════════════════════════════════════════════════

# Function to find a required Dawn library
function(dawn_lib VAR PATH)
    set(FULL "${DAWN_BUILD}/${PATH}")
    if(NOT EXISTS "${FULL}")
        message(FATAL_ERROR "Missing Dawn library: ${FULL}")
    endif()
    set(${VAR} "${FULL}" PARENT_SCOPE)
endfunction()

# Function to find an optional Dawn library (won't fail if missing)
function(dawn_lib_optional VAR PATH)
    set(FULL "${DAWN_BUILD}/${PATH}")
    if(EXISTS "${FULL}")
        set(${VAR} "${FULL}" PARENT_SCOPE)
        message(STATUS "  Found optional: ${PATH}")
    else()
        set(${VAR} "" PARENT_SCOPE)
    endif()
endfunction()

message(STATUS "Discovering Dawn libraries...")

# ═══════════════════════════════════════════════════════════════════════════════
# CORE DAWN LIBRARIES (Required)
# Per DAWN_REFERENCE.md - these are the required core libraries
# ═══════════════════════════════════════════════════════════════════════════════

dawn_lib(LIB_DAWN_NATIVE      "src/dawn/native/Debug/dawn_native.lib")
dawn_lib(LIB_DAWN_PROC        "src/dawn/Debug/dawn_proc.lib")
dawn_lib(LIB_DAWN_GLFW        "src/dawn/glfw/Debug/dawn_glfw.lib")
dawn_lib(LIB_DAWN_COMMON      "src/dawn/common/Debug/dawn_common.lib")
dawn_lib(LIB_DAWN_PLATFORM    "src/dawn/platform/Debug/dawn_platform.lib")

# Dawn utils (contains SystemHandle)
dawn_lib(LIB_DAWN_SYSTEM_UTILS "src/dawn/utils/Debug/dawn_system_utils.lib")

# GLFW from Dawn's third_party
dawn_lib(LIB_GLFW             "third_party/glfw/src/Debug/glfw3.lib")

# ═══════════════════════════════════════════════════════════════════════════════
# TINT LIBRARIES (Shader Compiler)
# ═══════════════════════════════════════════════════════════════════════════════

# Tint API
dawn_lib(LIB_TINT_API                   "src/tint/Debug/tint_api.lib")
dawn_lib(LIB_TINT_API_COMMON            "src/tint/Debug/tint_api_common.lib")

# Tint utils (foundational)
dawn_lib(LIB_TINT_UTILS                 "src/tint/Debug/tint_utils.lib")
dawn_lib(LIB_TINT_UTILS_ICE             "src/tint/Debug/tint_utils_ice.lib")
dawn_lib(LIB_TINT_UTILS_DIAGNOSTIC      "src/tint/Debug/tint_utils_diagnostic.lib")
dawn_lib(LIB_TINT_UTILS_SYMBOL          "src/tint/Debug/tint_utils_symbol.lib")
dawn_lib(LIB_TINT_UTILS_STRCONV         "src/tint/Debug/tint_utils_strconv.lib")
dawn_lib(LIB_TINT_UTILS_CONTAINERS      "src/tint/Debug/tint_utils_containers.lib")
dawn_lib(LIB_TINT_UTILS_MACROS          "src/tint/Debug/tint_utils_macros.lib")
dawn_lib(LIB_TINT_UTILS_MATH            "src/tint/Debug/tint_utils_math.lib")
dawn_lib(LIB_TINT_UTILS_MEMORY          "src/tint/Debug/tint_utils_memory.lib")
dawn_lib(LIB_TINT_UTILS_TEXT            "src/tint/Debug/tint_utils_text.lib")
dawn_lib(LIB_TINT_UTILS_TEXT_GENERATOR  "src/tint/Debug/tint_utils_text_generator.lib")
dawn_lib(LIB_TINT_UTILS_RTTI            "src/tint/Debug/tint_utils_rtti.lib")
dawn_lib(LIB_TINT_UTILS_BYTES           "src/tint/Debug/tint_utils_bytes.lib")
dawn_lib(LIB_TINT_UTILS_SYSTEM          "src/tint/Debug/tint_utils_system.lib")

# Tint core language
dawn_lib(LIB_TINT_LANG_CORE             "src/tint/Debug/tint_lang_core.lib")
dawn_lib(LIB_TINT_LANG_CORE_CONSTANT    "src/tint/Debug/tint_lang_core_constant.lib")
dawn_lib(LIB_TINT_LANG_CORE_TYPE        "src/tint/Debug/tint_lang_core_type.lib")
dawn_lib(LIB_TINT_LANG_CORE_INTRINSIC   "src/tint/Debug/tint_lang_core_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_CORE_IR          "src/tint/Debug/tint_lang_core_ir.lib")
dawn_lib(LIB_TINT_LANG_CORE_IR_TRANSFORM "src/tint/Debug/tint_lang_core_ir_transform.lib")

# Additional Tint core IR libs (may be needed)
dawn_lib_optional(LIB_TINT_LANG_CORE_IR_ANALYSIS "src/tint/Debug/tint_lang_core_ir_analysis.lib")
dawn_lib_optional(LIB_TINT_LANG_CORE_IR_BINARY   "src/tint/Debug/tint_lang_core_ir_binary.lib")
dawn_lib_optional(LIB_TINT_LANG_CORE_IR_TYPE     "src/tint/Debug/tint_lang_core_ir_type.lib")

# Tint WGSL (source language)
dawn_lib(LIB_TINT_LANG_WGSL             "src/tint/Debug/tint_lang_wgsl.lib")
dawn_lib(LIB_TINT_LANG_WGSL_AST         "src/tint/Debug/tint_lang_wgsl_ast.lib")
dawn_lib(LIB_TINT_LANG_WGSL_PROGRAM     "src/tint/Debug/tint_lang_wgsl_program.lib")
dawn_lib(LIB_TINT_LANG_WGSL_SEM         "src/tint/Debug/tint_lang_wgsl_sem.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER      "src/tint/Debug/tint_lang_wgsl_reader.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER_PARSER "src/tint/Debug/tint_lang_wgsl_reader_parser.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER_LOWER "src/tint/Debug/tint_lang_wgsl_reader_lower.lib")
dawn_lib(LIB_TINT_LANG_WGSL_READER_PROGRAM_TO_IR "src/tint/Debug/tint_lang_wgsl_reader_program_to_ir.lib")
dawn_lib(LIB_TINT_LANG_WGSL_RESOLVER    "src/tint/Debug/tint_lang_wgsl_resolver.lib")
dawn_lib(LIB_TINT_LANG_WGSL_INSPECTOR   "src/tint/Debug/tint_lang_wgsl_inspector.lib")
dawn_lib(LIB_TINT_LANG_WGSL_IR          "src/tint/Debug/tint_lang_wgsl_ir.lib")
dawn_lib(LIB_TINT_LANG_WGSL_INTRINSIC   "src/tint/Debug/tint_lang_wgsl_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER      "src/tint/Debug/tint_lang_wgsl_writer.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_AST_PRINTER "src/tint/Debug/tint_lang_wgsl_writer_ast_printer.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_COMMON "src/tint/Debug/tint_lang_wgsl_writer_common.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM "src/tint/Debug/tint_lang_wgsl_writer_ir_to_program.lib")
dawn_lib(LIB_TINT_LANG_WGSL_WRITER_RAISE "src/tint/Debug/tint_lang_wgsl_writer_raise.lib")

# Tint HLSL (D3D12 backend - required on Windows)
dawn_lib(LIB_TINT_LANG_HLSL             "src/tint/Debug/tint_lang_hlsl.lib")
dawn_lib(LIB_TINT_LANG_HLSL_IR          "src/tint/Debug/tint_lang_hlsl_ir.lib")
dawn_lib(LIB_TINT_LANG_HLSL_TYPE        "src/tint/Debug/tint_lang_hlsl_type.lib")
dawn_lib(LIB_TINT_LANG_HLSL_INTRINSIC   "src/tint/Debug/tint_lang_hlsl_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER      "src/tint/Debug/tint_lang_hlsl_writer.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER_COMMON "src/tint/Debug/tint_lang_hlsl_writer_common.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER_PRINTER "src/tint/Debug/tint_lang_hlsl_writer_printer.lib")
dawn_lib(LIB_TINT_LANG_HLSL_WRITER_RAISE "src/tint/Debug/tint_lang_hlsl_writer_raise.lib")

# Optional HLSL validation
dawn_lib_optional(LIB_TINT_LANG_HLSL_VALIDATE "src/tint/Debug/tint_lang_hlsl_validate.lib")

# Tint SPIRV (Vulkan backend)
dawn_lib(LIB_TINT_LANG_SPIRV            "src/tint/Debug/tint_lang_spirv.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_IR         "src/tint/Debug/tint_lang_spirv_ir.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_TYPE       "src/tint/Debug/tint_lang_spirv_type.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_INTRINSIC  "src/tint/Debug/tint_lang_spirv_intrinsic.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER     "src/tint/Debug/tint_lang_spirv_writer.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_COMMON "src/tint/Debug/tint_lang_spirv_writer_common.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_PRINTER "src/tint/Debug/tint_lang_spirv_writer_printer.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_WRITER_RAISE "src/tint/Debug/tint_lang_spirv_writer_raise.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER     "src/tint/Debug/tint_lang_spirv_reader.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER_COMMON "src/tint/Debug/tint_lang_spirv_reader_common.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER_LOWER "src/tint/Debug/tint_lang_spirv_reader_lower.lib")
dawn_lib(LIB_TINT_LANG_SPIRV_READER_PARSER "src/tint/Debug/tint_lang_spirv_reader_parser.lib")

# Optional SPIRV validation
dawn_lib_optional(LIB_TINT_LANG_SPIRV_VALIDATE "src/tint/Debug/tint_lang_spirv_validate.lib")

# Tint null backend (required for Dawn)
dawn_lib(LIB_TINT_LANG_NULL_WRITER        "src/tint/Debug/tint_lang_null_writer.lib")
dawn_lib(LIB_TINT_LANG_NULL_WRITER_COMMON "src/tint/Debug/tint_lang_null_writer_common.lib")
dawn_lib(LIB_TINT_LANG_NULL_WRITER_RAISE  "src/tint/Debug/tint_lang_null_writer_raise.lib")

# ═══════════════════════════════════════════════════════════════════════════════
# ABSEIL LIBRARIES (Google C++ utilities - Dawn dependency)
# ═══════════════════════════════════════════════════════════════════════════════

# Core abseil
dawn_lib(LIB_ABSEIL_BASE            "third_party/abseil/absl/base/Debug/absl_base.lib")
dawn_lib(LIB_ABSEIL_THROW           "third_party/abseil/absl/base/Debug/absl_throw_delegate.lib")
dawn_lib(LIB_ABSEIL_SPINLOCK        "third_party/abseil/absl/base/Debug/absl_spinlock_wait.lib")
dawn_lib(LIB_ABSEIL_RAW_LOG         "third_party/abseil/absl/base/Debug/absl_raw_logging_internal.lib")
dawn_lib(LIB_ABSEIL_LOG_SEVERITY    "third_party/abseil/absl/base/Debug/absl_log_severity.lib")

# Optional but often needed
dawn_lib_optional(LIB_ABSEIL_MALLOC "third_party/abseil/absl/base/Debug/absl_malloc_internal.lib")

# Strings
dawn_lib(LIB_ABSEIL_STRINGS         "third_party/abseil/absl/strings/Debug/absl_strings.lib")
dawn_lib(LIB_ABSEIL_STR_INTERNAL    "third_party/abseil/absl/strings/Debug/absl_strings_internal.lib")
dawn_lib(LIB_ABSEIL_STRING_VIEW     "third_party/abseil/absl/strings/Debug/absl_string_view.lib")

# Optional string libs
dawn_lib_optional(LIB_ABSEIL_CORD           "third_party/abseil/absl/strings/Debug/absl_cord.lib")
dawn_lib_optional(LIB_ABSEIL_CORD_INTERNAL  "third_party/abseil/absl/strings/Debug/absl_cord_internal.lib")
dawn_lib_optional(LIB_ABSEIL_STR_FORMAT     "third_party/abseil/absl/strings/Debug/absl_str_format_internal.lib")

# Numeric
dawn_lib(LIB_ABSEIL_INT128          "third_party/abseil/absl/numeric/Debug/absl_int128.lib")

# Synchronization
dawn_lib(LIB_ABSEIL_SYNCHRONIZATION "third_party/abseil/absl/synchronization/Debug/absl_synchronization.lib")
dawn_lib(LIB_ABSEIL_GRAPHCYCLES     "third_party/abseil/absl/synchronization/Debug/absl_graphcycles_internal.lib")
dawn_lib(LIB_ABSEIL_KERNEL_TIMEOUT  "third_party/abseil/absl/synchronization/Debug/absl_kernel_timeout_internal.lib")

# Time
dawn_lib(LIB_ABSEIL_TIME            "third_party/abseil/absl/time/Debug/absl_time.lib")
dawn_lib(LIB_ABSEIL_TIME_ZONE       "third_party/abseil/absl/time/Debug/absl_time_zone.lib")
dawn_lib(LIB_ABSEIL_CIVIL_TIME      "third_party/abseil/absl/time/Debug/absl_civil_time.lib")

# Debugging
dawn_lib(LIB_ABSEIL_STACKTRACE      "third_party/abseil/absl/debugging/Debug/absl_stacktrace.lib")
dawn_lib(LIB_ABSEIL_SYMBOLIZE       "third_party/abseil/absl/debugging/Debug/absl_symbolize.lib")
dawn_lib(LIB_ABSEIL_DEMANGLE        "third_party/abseil/absl/debugging/Debug/absl_demangle_internal.lib")
dawn_lib(LIB_ABSEIL_DEBUGGING       "third_party/abseil/absl/debugging/Debug/absl_debugging_internal.lib")

# Additional debugging support (for BorrowedFixupBuffer)
dawn_lib(LIB_ABSEIL_BORROWED_FIXUP "third_party/abseil/absl/debugging/Debug/absl_borrowed_fixup_buffer.lib")

# Hash
dawn_lib(LIB_ABSEIL_HASH            "third_party/abseil/absl/hash/Debug/absl_hash.lib")
dawn_lib(LIB_ABSEIL_CITY            "third_party/abseil/absl/hash/Debug/absl_city.lib")
dawn_lib_optional(LIB_ABSEIL_LOW_LEVEL_HASH "third_party/abseil/absl/hash/Debug/absl_low_level_hash.lib")

# Container support
dawn_lib_optional(LIB_ABSEIL_RAW_HASH_SET   "third_party/abseil/absl/container/Debug/absl_raw_hash_set.lib")
dawn_lib_optional(LIB_ABSEIL_HASHTABLEZ     "third_party/abseil/absl/container/Debug/absl_hashtablez_sampler.lib")

# Status (often needed)
dawn_lib_optional(LIB_ABSEIL_STATUS     "third_party/abseil/absl/status/Debug/absl_status.lib")
dawn_lib_optional(LIB_ABSEIL_STATUSOR   "third_party/abseil/absl/status/Debug/absl_statusor.lib")

# Profiling
dawn_lib_optional(LIB_ABSEIL_EXPONENTIAL "third_party/abseil/absl/profiling/Debug/absl_exponential_biased.lib")

# ═══════════════════════════════════════════════════════════════════════════════
# SPIRV-TOOLS
# ═══════════════════════════════════════════════════════════════════════════════

dawn_lib(LIB_SPIRV_TOOLS        "third_party/spirv-tools/source/Debug/SPIRV-Tools.lib")
dawn_lib(LIB_SPIRV_TOOLS_OPT    "third_party/spirv-tools/source/opt/Debug/SPIRV-Tools-opt.lib")


# ═══════════════════════════════════════════════════════════════════════════════
# ACTIVE CARTRIDGE SELECTION
# ═══════════════════════════════════════════════════════════════════════════════
#
# Controls which cartridges the incubator targets include.
# IntelliSense will only index these — no noise from other cartridges.
#
# Change here to switch focus. No need to edit .cpp files.
#
#   Render cartridges:   src/cartridges/<name>/cartridge.hpp
#   Analysis cartridges: src/analysis/<name>/canvas.hpp
#
#   Available render cartridges: the_board
#     (backup_board exists on disk as a frozen REFERENCE TEXT, not a build
#      target — see src/cartridges/backup_board/README.md. the_chord retired:
#      testbed, purpose served — see audit/LADDER.md sibling-prune entry.)
#
# Set INCUBATOR_ONLY=ON during development to hide the main target entirely.
# The main target pulls ALL cartridges and pollutes IntelliSense.

# Per-target cartridge selection. incubator_dual carries the render
# cartridge; the_lab shares the analysis selection.
set(INCUBATOR_ANALYSIS_CARTRIDGE      "canvas_1"         CACHE STRING "Analysis cartridge for the 'the_lab' target")

set(INCUBATOR_DUAL_RENDER_CARTRIDGE   "the_board"        CACHE STRING "Render cartridge for the 'incubator_dual' target")
set(THE_BOARD_DEMO                    "full"             CACHE STRING "the_board demo sentence (a DemoCol column of demos/matrix.hpp; compile-time door, DEMO-1). Default IS the golden path.")
set(INCUBATOR_DUAL_ANALYSIS_CARTRIDGE "canvas_1"         CACHE STRING "Analysis cartridge for the 'incubator_dual' target")
option(INCUBATOR_ONLY "Build only incubator targets (cleaner IntelliSense)" ON)

# ── Self-heal a stale cartridge selection ──────────────────────────────────────
# A CACHE value persists in an existing build directory and is NOT overwritten by
# the defaults above (a `set(... CACHE ...)` without FORCE is a no-op once the
# entry exists). So a build dir configured before a cartridge was renamed or
# removed — e.g. the old 'polyphony_basic' — keeps pointing at a directory that no
# longer exists, and the build fails with a confusing "cannot find file". Detect
# that and reset to the current default, so a plain reconfigure recovers without
# anyone having to clear the cache by hand. A still-valid selection is left alone,
# so an intentional -D choice of another (existing) cartridge is preserved.
foreach(_sel INCUBATOR_ANALYSIS_CARTRIDGE
             INCUBATOR_DUAL_ANALYSIS_CARTRIDGE)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/${${_sel}}/canvas.hpp")
        message(STATUS "Cartridge '${${_sel}}' (cached in ${_sel}) not found; resetting to 'canvas_1'.")
        set(${_sel} "canvas_1" CACHE STRING "Analysis cartridge" FORCE)
    endif()
endforeach()

message(STATUS "")
message(STATUS "Active cartridges:")
message(STATUS "  incubator_dual -> render: ${INCUBATOR_DUAL_RENDER_CARTRIDGE} analysis: ${INCUBATOR_DUAL_ANALYSIS_CARTRIDGE}")
message(STATUS "  the_lab        -> analysis: ${INCUBATOR_ANALYSIS_CARTRIDGE}")
message(STATUS "  Incubator only: ${INCUBATOR_ONLY}")
message(STATUS "")

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
    LIB_TINT_LANG_CORE_IR_BINARY
    LIB_TINT_LANG_CORE_IR_TYPE
    LIB_TINT_LANG_HLSL_VALIDATE
    LIB_TINT_LANG_SPIRV_VALIDATE
    LIB_ABSEIL_MALLOC
    LIB_ABSEIL_CORD
    LIB_ABSEIL_CORD_INTERNAL
    LIB_ABSEIL_STR_FORMAT
    LIB_ABSEIL_LOW_LEVEL_HASH
    LIB_ABSEIL_RAW_HASH_SET
    LIB_ABSEIL_HASHTABLEZ
    LIB_ABSEIL_STATUS
    LIB_ABSEIL_STATUSOR
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
set(MSVC_COMPILE_OPTS /W3 /wd4251 /wd4275 /wd4244 /wd4267 /EHsc)
set(MSVC_COMPILE_DEFS NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS __WINDOWS_MM__)

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
# INCUBATOR DUAL — Analysis + Render cartridge development
# ═══════════════════════════════════════════════════════════════════════════════
#
# Pairs an analysis cartridge with a render cartridge (the_board).
# Uses INCUBATOR_DUAL_ANALYSIS_CARTRIDGE and INCUBATOR_DUAL_RENDER_CARTRIDGE.
#
# Build:  cmake --build . --target incubator_dual
# Run:    ./incubator_dual.exe [optional_midi_file]
#

# Only the active cartridges' headers → scoped IntelliSense
file(GLOB_RECURSE DUAL_RENDER_HEADERS
    "src/cartridges/${INCUBATOR_DUAL_RENDER_CARTRIDGE}/*.hpp"
)
file(GLOB_RECURSE DUAL_ANALYSIS_HEADERS CONFIGURE_DEPENDS
    "src/analysis/${INCUBATOR_DUAL_ANALYSIS_CARTRIDGE}/*.hpp"
)

add_executable(incubator_dual
    src/incubator_dual.cpp
    src/external/stb_image.cpp
    src/external/RtMidi.cpp
    ${DUAL_RENDER_HEADERS}
    ${DUAL_ANALYSIS_HEADERS}
)

target_include_directories(incubator_dual PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${DAWN_INCLUDES}
)

target_link_libraries(incubator_dual PRIVATE ${DAWN_LIBS})

if(MSVC)
    target_compile_options(incubator_dual PRIVATE ${MSVC_COMPILE_OPTS})
    target_compile_definitions(incubator_dual PRIVATE
        ${MSVC_COMPILE_DEFS}
        INCUBATE_RENDER=${INCUBATOR_DUAL_RENDER_CARTRIDGE}
        INCUBATE_ANALYSIS=${INCUBATOR_DUAL_ANALYSIS_CARTRIDGE}
        INCUBATE_DEMO=${THE_BOARD_DEMO}
    )
endif()

add_custom_command(TARGET incubator_dual POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges
    $<TARGET_FILE_DIR:incubator_dual>/cartridges
    COMMENT "Copying cartridges for incubator_dual..."
)

add_custom_command(TARGET incubator_dual POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:incubator_dual>/assets
    COMMENT "Copying assets for incubator_dual..."
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges/${INCUBATOR_DUAL_RENDER_CARTRIDGE}"
    PREFIX "Render Cartridge"
    FILES ${DUAL_RENDER_HEADERS}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/${INCUBATOR_DUAL_ANALYSIS_CARTRIDGE}"
    PREFIX "Analysis Cartridge"
    FILES ${DUAL_ANALYSIS_HEADERS}
)


# ═══════════════════════════════════════════════════════════════════════════════
# THE_LAB — Coupling laboratory (ImGui + WebGPU, no render cartridge)
# ═══════════════════════════════════════════════════════════════════════════════
#
# Sub-second iteration sandbox for designing musical couplings.
# Pairs an analysis cartridge with an ImGui+ImPlot dashboard.
# No project WGSL pipelines — FXC compile time is trivial.
#
# Build:  cmake --build . --target the_lab
# Run:    ./the_lab.exe [optional_midi_file]
#

# Analysis cartridge headers (INCUBATOR_ANALYSIS_CARTRIDGE selection)
file(GLOB_RECURSE LAB_ANALYSIS_HEADERS CONFIGURE_DEPENDS
    "src/analysis/${INCUBATOR_ANALYSIS_CARTRIDGE}/*.hpp"
)

# ImGui sources (vendored)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
)

# ImPlot sources (vendored)
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/src/external/implot)
set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

add_executable(the_lab
    src/the_lab.cpp
    src/external/RtMidi.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    ${LAB_ANALYSIS_HEADERS}
)

target_include_directories(the_lab PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
    ${DAWN_INCLUDES}
)

target_link_libraries(the_lab PRIVATE ${DAWN_LIBS})

if(MSVC)
    target_compile_options(the_lab PRIVATE ${MSVC_COMPILE_OPTS})
    target_compile_definitions(the_lab PRIVATE
        ${MSVC_COMPILE_DEFS}
        LAB_ANALYSIS=${INCUBATOR_ANALYSIS_CARTRIDGE}
        # imgui_impl_wgpu chooses its WebGPU runtime binding via this define
        IMGUI_IMPL_WEBGPU_BACKEND_DAWN
    )
endif()

add_custom_command(TARGET the_lab POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:the_lab>/assets
    COMMENT "Copying assets for the_lab..."
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/${INCUBATOR_ANALYSIS_CARTRIDGE}"
    PREFIX "Analysis Cartridge"
    FILES ${LAB_ANALYSIS_HEADERS}
)

# ═══════════════════════════════════════════════════════════════════════════════
# ── PROBE_CANVAS — headless reading check (RtMidi only; no Dawn, no ImGui) ──
# ═══════════════════════════════════════════════════════════════════════════════
#
# The canvas-cartridge probe. The earlier one-off probes (probe, probe_events,
# probe_field, probe_interval, probe_lowest, probe_vector*) were scaffolding for
# building the analysis layer and have been archived under
# "src/dev/BACKUP 15 06/"; their targets are removed.

add_executable(probe_canvas src/analysis/canvas_1/probe_canvas.cpp src/external/RtMidi.cpp)
target_include_directories(probe_canvas PRIVATE src)
if(MSVC)
    target_compile_definitions(probe_canvas PRIVATE __WINDOWS_MM__)
    target_link_libraries(probe_canvas PRIVATE winmm)
endif()


# ═══════════════════════════════════════════════════════════════════════════════
# ── CANVAS_1 CHECKS — headless assertion tests (no Dawn, no ImGui) ──
# ═══════════════════════════════════════════════════════════════════════════════
#
# check_canvas_union   — the published surface end-to-end: two voices, the union
#                        field in the group band, opt-in and availability-bound.
#                        Drives the canvas directly, port dormant; needs RtMidi
#                        linked (the Canvas owns a MidiPort) but never opens it.
# check_field_union    — the union primitive + field election in isolation, over
#                        field.hpp alone; no canvas, no port, no RtMidi.

add_executable(check_canvas_union src/analysis/canvas_1/check_canvas_union.cpp src/external/RtMidi.cpp)
target_include_directories(check_canvas_union PRIVATE src)
if(MSVC)
    target_compile_definitions(check_canvas_union PRIVATE __WINDOWS_MM__)
    target_link_libraries(check_canvas_union PRIVATE winmm)
endif()

add_executable(check_field_union src/analysis/canvas_1/check_field_union.cpp)
target_include_directories(check_field_union PRIVATE src)

# check_pc_dft — the pc-DFT capability's pure-fn gate: known vectors →
# known interval families, over pc_dft.hpp alone; no canvas, no RtMidi.
add_executable(check_pc_dft src/analysis/canvas_1/check_pc_dft.cpp)
target_include_directories(check_pc_dft PRIVATE src)

add_executable(check_canvas_compound src/analysis/canvas_1/check_canvas_compound.cpp src/external/RtMidi.cpp)
target_include_directories(check_canvas_compound PRIVATE src)
if(MSVC)
    target_compile_definitions(check_canvas_compound PRIVATE __WINDOWS_MM__)
    target_link_libraries(check_canvas_compound PRIVATE winmm)
endif()


# ═══════════════════════════════════════════════════════════════════════════════
# MAIN TARGET — Full program with all cartridges
# ═══════════════════════════════════════════════════════════════════════════════
#
# Disabled by default during development (INCUBATOR_ONLY=ON).
# This target discovers ALL cartridge headers, which floods IntelliSense
# with duplicate definitions. Enable for production builds only.
#
# To enable:  cmake -DINCUBATOR_ONLY=OFF ..
# To disable: cmake -DINCUBATOR_ONLY=ON ..  (default)

if(NOT INCUBATOR_ONLY)

# Auto-discover ALL cartridge headers
file(GLOB_RECURSE CARTRIDGE_HEADERS "src/cartridges/*.hpp")

message(STATUS "")
message(STATUS "Discovered cartridges (main target):")
foreach(HEADER ${CARTRIDGE_HEADERS})
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${HEADER})
    message(STATUS "  ${REL_PATH}")
endforeach()
message(STATUS "")

# Header-only modules (for IDE visibility)
set(HEADERS
    # Console (runtime infrastructure)
    src/console/console.hpp

    # Core
    src/core/types.hpp
    src/core/clock.hpp
    src/core/input_event.hpp

    # Analysis (cartridge interface + signal contract)
    src/analysis/analysis_signal.hpp
    src/analysis/analysis_cartridge.hpp

    # Render (cartridge interface)
    src/render/render_cartridge.hpp

    # Sources (MIDI input providers)
    src/sources/midi_event.hpp
    src/sources/midi_file.hpp
    src/sources/keyboard_midi.hpp
    src/sources/midi_port.hpp

    # Musical
    src/musical/stream_data.hpp
    src/musical/midi_stream.hpp
    src/musical/musical_ops.hpp
    src/musical/playhead.hpp
    src/musical/wagon.hpp

    # Cartridges (auto-discovered)
    ${CARTRIDGE_HEADERS}
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${HEADERS}
    src/external/stb_image.cpp
    src/external/RtMidi.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${DAWN_INCLUDES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${DAWN_LIBS})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE ${MSVC_COMPILE_OPTS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${MSVC_COMPILE_DEFS})
endif()

# Post-build: copy assets and cartridges
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/cartridges
    COMMENT "Copying cartridges..."
)

# IDE organization
source_group("Source" FILES src/main.cpp src/incubator.cpp)
source_group("Console" FILES src/console/console.hpp)
source_group("Core" FILES src/core/types.hpp src/core/clock.hpp src/core/input_event.hpp)
source_group("Analysis" FILES src/analysis/analysis_signal.hpp src/analysis/analysis_cartridge.hpp)
source_group("Render" FILES src/render/render_cartridge.hpp)
source_group("Sources" FILES src/sources/midi_event.hpp src/sources/midi_file.hpp src/sources/keyboard_midi.hpp src/sources/midi_port.hpp)
source_group("Musical" FILES
    src/musical/stream_data.hpp src/musical/midi_stream.hpp src/musical/musical_ops.hpp
    src/musical/playhead.hpp src/musical/wagon.hpp
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/cartridges"
    PREFIX "Cartridges"
    FILES ${CARTRIDGE_HEADERS}
)

endif() # NOT INCUBATOR_ONLY
