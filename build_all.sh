#!/usr/bin/env bash
# vim:ts=4:sts=4:sw=4:et
# shellcheck disable=SC2317,SC2329

# SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
# SPDX-License-Identifier: Apache-2.0
#
# Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

set -eEuo pipefail

declare -r PROGNAME="${0##*/}"

declare -r COLOR_RESET_DEFAULT="\033[0m"
declare -r COLOR_RED_DEFAULT="\033[31;1m"
declare -r COLOR_GREEN_DEFAULT="\033[32;1m"
declare -r COLOR_YELLOW_DEFAULT="\033[33;1m"
declare -r COLOR_BLUE_DEFAULT="\033[34;1m"
declare -r COLOR_CYAN_DEFAULT="\033[36;1m"

COLOR_RESET="${COLOR_RESET_DEFAULT}"
COLOR_RED="${COLOR_RED_DEFAULT}"
COLOR_GREEN="${COLOR_GREEN_DEFAULT}"
COLOR_YELLOW="${COLOR_YELLOW_DEFAULT}"
COLOR_BLUE="${COLOR_BLUE_DEFAULT}"
COLOR_CYAN="${COLOR_CYAN_DEFAULT}"

CONSOLE_MODE="auto"

function log_timestamp()
{
    date "+%Y-%m-%d %H:%M:%S"
}

function log_info()
{
    printf '%b\n' "${COLOR_CYAN}[$(log_timestamp)] [INFO]${COLOR_RESET} $*"
}

function log_success()
{
    printf '%b\n' "${COLOR_GREEN}[$(log_timestamp)] [SUCCESS]${COLOR_RESET} $*"
}

function log_warning()
{
    printf '%b\n' "${COLOR_YELLOW}[$(log_timestamp)] [WARNING]${COLOR_RESET} $*"
}

function log_error()
{
    printf '%b\n' "${COLOR_RED}[$(log_timestamp)] [ERROR]${COLOR_RESET} $*" >&2
}

function log_step()
{
    printf '\n%b\n\n' "${COLOR_BLUE}[$(log_timestamp)] [STEP]${COLOR_RESET} $*"
}

function apply_console_mode()
{
    function _disable_colors()
    {
        COLOR_RESET=""
        COLOR_RED=""
        COLOR_GREEN=""
        COLOR_YELLOW=""
        COLOR_BLUE=""
        COLOR_CYAN=""
    }

    case "${CONSOLE_MODE}" in
        auto)
            if [[ ! -t 1 ]]; then
                _disable_colors
            fi
            ;;
        plain)
            _disable_colors
            ;;
        colored)
            :
            ;;
        verbose)
            set -x
            ;;
        *)
            log_error "Invalid --console value: '${CONSOLE_MODE}'. Allowed: auto, plain, colored, verbose"
            usage
            exit 1
            ;;
    esac
}

function check_cmd()
{
    command -v "${1}" >/dev/null 2>&1
}

function is_linux()
{
    local OS_NAME
    OS_NAME="$(uname | tr '[:upper:]' '[:lower:]')"
    [[ "${OS_NAME}" == "linux" ]]
}

function is_linux64()
{
    is_linux || return 1
    local OS_ARCH
    OS_ARCH="$(uname -m)"
    [[ "${OS_ARCH}" == "x86_64" ]]
}

function is_ci()
{
    if [[ -n "${CI:-}" ]] && [[ "${CI}" != "false" ]] && [[ "${CI}" != "0" ]]; then
        return 0
    fi

    local -a CI_VARS=(
        GITHUB_ACTIONS
        GITLAB_CI
        JENKINS_URL
        TEAMCITY_VERSION
        BUILDKITE
        CIRCLECI
        TRAVIS
        APPVEYOR
        TF_BUILD
        BITBUCKET_BUILD_NUMBER
        DRONE
        SEMAPHORE
        CODEBUILD_BUILD_ID
    )

    local VAR
    for VAR in "${CI_VARS[@]}"; do
        [[ -n "${!VAR:-}" ]] && return 0
    done

    return 1
}

if ! is_linux64; then
    log_error "This script only supports 64-bit Linux (x86_64)."
    log_error "Detected OS: $(uname -s), Architecture: $(uname -m)"
    exit 1
fi

SCRIPT_DIR=""
SHOW_EXIT_MESSAGE=true

function dist_clean()
{
    [ "${CLEAN_ON_EXIT}" = true ] || return 0

    rm -rf "${SCRIPT_DIR}/BUILD" 2>/dev/null || true
}

function ctrl_c()
{
    exit 1
}
trap ctrl_c INT TERM

function on_error()
{
    local EXIT_CODE="${1:-1}"
    local LINE="${2:-unknown}"
    local COMMAND="${3:-unknown}"

    log_error "Command failed at line ${LINE}: ${COMMAND}"
    log_error "Exit code: ${EXIT_CODE}"

    if [[ ${#FUNCNAME[@]} -gt 2 ]]; then
        log_error "Call stack:"
        for ((I = 1; I < ${#FUNCNAME[@]} - 1; I++)); do
            log_error "  ${FUNCNAME[I]}() at ${BASH_SOURCE[I]}:${BASH_LINENO[I - 1]}"
        done
    fi
}
trap 'on_error "$?" "${LINENO}" "${BASH_COMMAND}"' ERR

function on_exit()
{
    local EXIT_CODE=$?
    set +e

    dist_clean

    if "${SHOW_EXIT_MESSAGE}"; then
        printf '\n'
        log_info "Bye-Bye"
    fi

    exit "${EXIT_CODE}"
}
trap on_exit EXIT

function usage()
{
    SHOW_EXIT_MESSAGE=false

    printf '%b\n' "${COLOR_GREEN}
Usage: ${PROGNAME} [options]

Options:
    -h, --help                   Show this help message

    --sdk-archive=<path>         Set path to the Maps SDK for C++ archive
                                 (REQUIRED)

    --build-type=<type>          Set build type: Debug, Release, or RelWithDebInfo
                                 (default: Release)

    --api-token=<token>          Specify API token to be hardcoded into examples
                                 Can also be set via API_TOKEN environment variable
                                 (command line takes precedence)

    --with-sdl                   Prefer using SDL for OpenGL context creation
                                 instead of GLFW

    --with-sanitizer=<type>      Configure for Linux 64-bit RelWithDebInfo build
                                 (+GLFW) with specified sanitizer enabled
                                 Allowed: address, undefined, thread

                                 NOTE: Sanitizer builds use GLFW backend for now,
                                 even if --with-sdl is specified.

    --analyze                    Configure for Linux 64-bit Debug build (+GLFW)
                                 and analyze C++ code for all examples with
                                 Cppcheck, Clang-Tidy and Cpplint

    --clean                      Remove build artifacts on exit
                                 (default: on in CI, off locally)

    --console=(auto|plain|colored|verbose)
                                 Specifies which type of console output to generate

                                 auto:    colored when attached to a terminal,
                                          plain otherwise (default)
                                 plain:   plain text only; disables all color
                                 colored: colored output
                                 verbose: colored output and verbose logging
${COLOR_RESET}"
}

function check_pkg()
{
    dpkg-query -W --showformat='${Status}\n' "${1}" 2>/dev/null | grep -q "install ok installed"
}

function check_linux_prerequisites()
{
    # Only apt/dpkg-based distributions are supported
    if ! check_cmd apt-get || ! check_cmd dpkg-query; then
        log_error "This script requires apt package manager (Debian/Ubuntu-based distributions)."
        log_error "Other distributions (Fedora, Arch, openSUSE, etc.) are not currently supported."
        log_error "Please manually install the required development packages:"
        log_error "  - X11/Xorg development headers"
        log_error "  - OpenGL/GLU development headers"
        log_error "  - EGL development headers"
        if "${WITH_SDL}"; then
            log_error "  - Python 3 Jinja2 module"
        fi
        exit 1
    fi

    declare -a REQUIRED_PKGS=('xorg-dev' 'libglu1-mesa-dev' 'libegl1-mesa-dev')

    if "${WITH_SDL}"; then
        REQUIRED_PKGS+=('python3-jinja2')
    fi

    local -a MISSING_PKGS=()
    local PKG
    for PKG in "${REQUIRED_PKGS[@]}"; do
        if ! check_pkg "${PKG}"; then
            MISSING_PKGS+=("${PKG}")
        fi
    done

    if [[ ${#MISSING_PKGS[@]} -gt 0 ]]; then
        log_warning "Missing required packages: ${MISSING_PKGS[*]}"

        # Check if running as root
        if [[ "$(id -u)" -ne 0 ]]; then
            log_warning "Not running as root. Package installation may fail."
            log_info "You may need to run the following command manually with sudo:"
            log_info "    sudo apt-get install -y ${MISSING_PKGS[*]}"
        fi

        log_info "Attempting to install missing packages..."
        if ! apt-get install -y "${MISSING_PKGS[@]}"; then
            log_error "Failed to install required packages"
            if [[ "$(id -u)" -ne 0 ]]; then
                log_error "Try running: sudo apt-get install -y ${MISSING_PKGS[*]}"
            fi
            exit 1
        fi
        log_success "Required packages installed"
    fi
}

function check_analyze_prerequisites()
{
    "${ANALYZE}" || return 0

    log_info "Checking static analysis tools..."

    if ! check_cmd clang-tidy; then
        log_error "You must install Clang-Tidy"
        log_error "Check how to install the latest stable version of LLVM at https://apt.llvm.org"
        exit 1
    fi
    log_info "Found clang-tidy: $(which clang-tidy)"

    if ! check_cmd cppcheck; then
        log_error "You must install Cppcheck"
        log_error "Check how to install the latest stable version at https://cppcheck.sourceforge.io"
        exit 1
    fi
    log_info "Found cppcheck: $(which cppcheck)"

    if ! check_cmd cpplint; then
        log_error "You must install Cpplint"
        log_error "Check how to install the latest stable version at https://github.com/cpplint/cpplint"
        exit 1
    fi
    log_info "Found cpplint: $(which cpplint)"

    log_success "Static analysis tools verification passed"
}

function extract_sdk_archive()
{
    log_step "Extracting SDK archive..."

    if [[ -d "${SCRIPT_DIR}/SDK/lib" ]]; then
        log_info "Cleaning existing SDK directory..."
        rm -rf "${SCRIPT_DIR:?}/SDK/"*
    fi

    tar -xvf "${SDK_ARCHIVE_PATH}" --strip-components=1 -C "${SCRIPT_DIR}/SDK" || { log_error "SDK extraction failed"; exit 1; }
    log_success "SDK archive extracted successfully"
}

function prepare_cmake_lists()
{
    if [[ -f "${SCRIPT_DIR}/CMakeLists.txt.SDK" ]]; then
        log_info "Preparing CMakeLists.txt files from SDK templates..."
        find "${SCRIPT_DIR}" -name "CMakeLists.txt.SDK" -exec bash -c \
            'cp "${0}" "$(echo "${0}" | sed -r "s/.txt.SDK/.txt/g")"' '{}' \;
    fi
}

function build_all_examples()
{
    log_step "Building all examples..."

    pushd "${SCRIPT_DIR}" > /dev/null || exit 1

    prepare_cmake_lists

    local OPENGL_BACKEND="glfw"
    if "${WITH_SDL}"; then
        if [[ -n "${WITH_SANITIZER}" ]]; then
            log_warning "Sanitizer builds use GLFW backend. Ignoring --with-sdl flag."
        else
            OPENGL_BACKEND="sdl"
        fi
    fi

    # Always clean build
    if [[ -d "BUILD" ]]; then
        log_info "Removing existing BUILD directory..."
        rm -rf "BUILD"
    fi

    local CONFIG_PRESET_NAME="linux-${OPENGL_BACKEND}-ninja"
    local BUILD_PRESET_NAME="linux-${OPENGL_BACKEND}-ninja"

    if [[ -n "${WITH_SANITIZER}" ]]; then
        CONFIG_PRESET_NAME+="-relwithdebinfo-${WITH_SANITIZER,,}-sanitizer"
        BUILD_PRESET_NAME+="-relwithdebinfo-${WITH_SANITIZER,,}-sanitizer-all"
    elif "${ANALYZE}"; then
        CONFIG_PRESET_NAME+="-debug-analyze"
        BUILD_PRESET_NAME+="-debug-analyze-all"
    else
        CONFIG_PRESET_NAME+="-${BUILD_TYPE,,}"
        BUILD_PRESET_NAME+="-${BUILD_TYPE,,}-all"
    fi

    log_info "Configure preset: ${CONFIG_PRESET_NAME}"
    log_info "Build preset: ${BUILD_PRESET_NAME}"

    local -a CMAKE_ARGS=()
    CMAKE_ARGS+=("--preset" "${CONFIG_PRESET_NAME}")

    if [[ -n "${API_TOKEN}" ]]; then
        CMAKE_ARGS+=("-DAPI_TOKEN=${API_TOKEN}")
    else
        log_warning "No token set. You can still test your apps, but a watermark will be displayed, and all the online services including mapping, searching, routing, etc. will slow down after a few minutes."
    fi

    log_step "Running CMake configure..."
    cmake "${CMAKE_ARGS[@]}"

    log_step "Running CMake build..."
    cmake --build --preset "${BUILD_PRESET_NAME}" --verbose

    popd > /dev/null || true

    log_success "Build completed successfully"

    # Print sanitizer runtime instructions if applicable
    if [[ -n "${WITH_SANITIZER}" ]]; then
        print_sanitizer_instructions
    fi
}

function print_sanitizer_instructions()
{
    local SUPPRESSIONS_FILE="${SCRIPT_DIR}/lsan_suppressions.txt"

    log_step "Sanitizer Runtime Instructions"

    log_info "Your build was compiled with ${WITH_SANITIZER} Sanitizer enabled."
    log_info ""

    if [[ "${WITH_SANITIZER,,}" == "address" ]]; then
        log_info "AddressSanitizer (ASan) detects memory errors such as:"
        log_info "  - Buffer overflows (stack, heap, global)"
        log_info "  - Use-after-free, use-after-return"
        log_info "  - Memory leaks (via LeakSanitizer)"
        log_info ""

        if [[ -f "${SUPPRESSIONS_FILE}" ]]; then
            log_info "A LeakSanitizer suppressions file is available at:"
            log_info "  ${SUPPRESSIONS_FILE}"
            log_info ""
            log_info "To use it, run your application with:"
            log_info ""
            log_info "  export LSAN_OPTIONS=suppressions=${SUPPRESSIONS_FILE}"
            log_info "  ./BUILD/${CONFIG_PRESET_NAME}/RelWithDebInfo/bin/<YourApp>"
            log_info ""
            log_info "Or in a single command:"
            log_info ""
            log_info "  LSAN_OPTIONS=suppressions=${SUPPRESSIONS_FILE} ./BUILD/${CONFIG_PRESET_NAME}/RelWithDebInfo/bin/<YourApp>"
        fi
    elif [[ "${WITH_SANITIZER,,}" == "thread" ]]; then
        log_info "ThreadSanitizer (TSan) detects data races and threading issues."
        log_info ""
        log_info "Run your application normally. TSan will report any detected races."
        log_info ""
        log_info "To suppress false positives, create a suppressions file and use:"
        log_info "  export TSAN_OPTIONS=suppressions=/path/to/tsan_suppressions.txt"
    elif [[ "${WITH_SANITIZER,,}" == "undefined" ]]; then
        log_info "UndefinedBehaviorSanitizer (UBSan) detects undefined behavior such as:"
        log_info "  - Integer overflow"
        log_info "  - Null pointer dereference"
        log_info "  - Misaligned memory access"
        log_info ""
        log_info "Run your application normally. UBSan will report any detected issues."
        log_info ""
        log_info "To suppress false positives, create a suppressions file and use:"
        log_info "  export UBSAN_OPTIONS=suppressions=/path/to/ubsan_suppressions.txt"
    fi

    log_info ""
    log_info "For more information on sanitizer options, see:"
    log_info "  https://github.com/google/sanitizers/wiki"
}

# =============================================================================
# Main
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

SDK_ARCHIVE_PATH=""
API_TOKEN="${API_TOKEN:-}"
BUILD_TYPE="Release"
WITH_SDL=false
WITH_SANITIZER=""
ANALYZE=false
CLEAN_ON_EXIT=false

if is_ci; then
    CLEAN_ON_EXIT=true
fi

SHORTOPTS="h"
LONGOPTS_LIST=(
    "help"
    "sdk-archive:"
    "build-type:"
    "api-token:"
    "with-sdl"
    "with-sanitizer:"
    "analyze"
    "clean"
    "console:"
)

if ! PARSED_OPTIONS="$(getopt \
    -s bash \
    --options "${SHORTOPTS}" \
    --longoptions "$(printf "%s," "${LONGOPTS_LIST[@]}")" \
    --name "${PROGNAME}" \
    -- "$@")"; then
    usage
    exit 1
fi

eval set -- "${PARSED_OPTIONS}"
unset PARSED_OPTIONS

while true; do
    case "${1}" in
        -h|--help)
            usage
            exit 0
            ;;
        --sdk-archive)
            shift
            SDK_ARCHIVE_PATH="${1}"
            ;;
        --build-type)
            shift
            BUILD_TYPE="${1}"
            ;;
        --api-token)
            shift
            API_TOKEN="${1}"
            ;;
        --with-sdl)
            WITH_SDL=true
            ;;
        --with-sanitizer)
            shift
            WITH_SANITIZER="${1}"
            ;;
        --analyze)
            ANALYZE=true
            ;;
        --clean)
            CLEAN_ON_EXIT=true
            ;;
        --console)
            shift
            CONSOLE_MODE="${1}"
            ;;
        --)
            shift
            break
            ;;
        *)
            log_error "Internal error"
            exit 1
            ;;
    esac
    shift
done

apply_console_mode

log_step "Checking prerequisites..."

if [[ ! -f "${SDK_ARCHIVE_PATH}" ]]; then
    log_error "You must provide local path to SDK archive using --sdk-archive=<path>"
    usage
    exit 1
fi

if ! [[ "${BUILD_TYPE}" =~ ^(Debug|Release|RelWithDebInfo)$ ]]; then
    log_error "Invalid build type '${BUILD_TYPE}'. Must be Debug, Release, or RelWithDebInfo"
    usage
    exit 1
fi

if [[ -z "${VCPKG_ROOT:-}" ]]; then
    log_error "vcpkg package manager not found"
    log_error "Please export VCPKG_ROOT environment variable"
    log_error "See: https://vcpkg.io/en/getting-started.html"
    exit 1
fi
log_info "VCPKG_ROOT: ${VCPKG_ROOT}"

if ! check_cmd cmake; then
    log_error "cmake command not found"
    log_error "Please install CMake: https://cmake.org/download/"
    exit 1
fi
log_info "Found cmake: $(cmake --version | head -n1)"

if ! check_cmd ninja; then
    log_error "ninja command not found"
    log_error "Please install Ninja: https://ninja-build.org/"
    exit 1
fi
log_info "Found ninja: $(ninja --version)"

if [[ -n "${WITH_SANITIZER}" ]]; then
    if ! [[ "${WITH_SANITIZER,,}" =~ ^(address|undefined|thread)$ ]]; then
        log_error "Invalid sanitizer value '${WITH_SANITIZER}'"
        log_error "Allowed: address, undefined, thread"
        usage
        exit 1
    fi
    log_info "Sanitizer enabled: ${WITH_SANITIZER}"
fi

check_linux_prerequisites
check_analyze_prerequisites

log_success "Prerequisites check passed"

extract_sdk_archive

build_all_examples

exit 0
