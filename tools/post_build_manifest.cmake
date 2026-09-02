# EMBER_0 — THE POST-BUILD MANIFEST
#
# Run by CMakeLists' last POST_BUILD step, via `cmake -P`. It prints what
# is actually beside the freshly-linked exe and NEVER fails: no
# FATAL_ERROR, no non-zero exit, no message(SEND_ERROR). A witness that
# can break a build is a witness nobody leaves armed, and this one exists
# to be left armed.
#
# WHY IT EXISTS. A POST_BUILD step's COMMENT prints when the step is
# REACHED, not when it succeeds, so a step that silently fails to run
# reads in the log exactly like one that ran. That is not a worry, it is
# an observation: a certified MSBuild build printed both copy comments
# and left no DLL in the output tree at all. This turns the question
# "did it land?" from an archaeology exercise into a line of build
# output.
#
# Inputs, all passed as -D by the caller:
#   T7_EXE_DIR   the target file's directory
#   T7_EXE_NAME  the target file's name (the_board.exe on Windows)
#   T7_WANT_DXC  1 when configure found a DXC pair to copy, else 0

if(NOT DEFINED T7_EXE_DIR OR T7_EXE_DIR STREQUAL "")
    message(STATUS "[manifest] T7_EXE_DIR not supplied — nothing to report")
    return()
endif()

if(NOT DEFINED T7_EXE_NAME OR T7_EXE_NAME STREQUAL "")
    set(T7_EXE_NAME "the_board")
endif()

# The expected set. `assets` is a directory; the rest are files. The DXC
# pair is only expected when configure actually found one — asking for it
# otherwise would print two ABSENT lines that mean nothing.
set(T7_EXPECTED "${T7_EXE_NAME}" "assets")
if(T7_WANT_DXC)
    list(APPEND T7_EXPECTED "dxcompiler.dll" "dxil.dll")
endif()

set(T7_MISSING "")
message(STATUS "[manifest] beside the exe — ${T7_EXE_DIR}")
foreach(T7_ITEM IN LISTS T7_EXPECTED)
    if(EXISTS "${T7_EXE_DIR}/${T7_ITEM}")
        message(STATUS "[manifest]   PRESENT  ${T7_ITEM}")
    else()
        message(STATUS "[manifest]   ABSENT   ${T7_ITEM}")
        list(APPEND T7_MISSING "${T7_ITEM}")
    endif()
endforeach()

if(T7_MISSING)
    # STATUS, not WARNING: on the Vulkan plan a missing DXC pair is the
    # normal state, and a warning that fires on every normal build is
    # noise that teaches the reader to skip this block. The ABSENT lines
    # above are the signal; this line only names what to do about them.
    message(STATUS
        "[manifest] MISSING: ${T7_MISSING} — a post-build step did not "
        "land. Look at the build log for the step's own error, not at "
        "CMakeLists' command list (docs/OPEN.md: THE pwsh POST-BUILD "
        "FAILURE).")
else()
    message(STATUS "[manifest] complete — every expected item is present")
endif()
