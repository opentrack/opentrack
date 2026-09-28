#!/usr/bin/env bash
# Script to cross-compile relay.c for Android using the Android NDK
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${OUT_DIR:-$SCRIPT_DIR/out}"
mkdir -p "${OUT_DIR}"

if [[ -z "${ANDROID_NDK_ROOT:-}" && -n "${ANDROID_NDK_HOME:-}" ]]; then
    ANDROID_NDK_ROOT="${ANDROID_NDK_HOME}"
fi

if [[ -z "${ANDROID_NDK_ROOT:-}" ]]; then
    echo "ANDROID_NDK_ROOT is not set. Looking for NDK in standard locations..."
    for candidate in \
        "$HOME/AppData/Local/Android/Sdk/ndk/"* \
        "/c/Program Files (x86)/Android/android-sdk/ndk/"* \
        "$HOME/Android/Sdk/ndk/"*; do
        if [[ -d "${candidate}" ]]; then
            ANDROID_NDK_ROOT="${candidate}"
            break
        fi
    done
fi

if [[ -n "${ANDROID_NDK_ROOT:-}" && -d "${ANDROID_NDK_ROOT}" ]]; then
    echo "Found Android NDK at: ${ANDROID_NDK_ROOT}"
    HOST_OS="linux-x86_64"
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        HOST_OS="windows-x86_64"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        HOST_OS="darwin-x86_64"
    fi

    TOOLCHAIN="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/${HOST_OS}/bin"
    API_LEVEL=24

    CLANG="${TOOLCHAIN}/clang"
    if [[ ! -x "${CLANG}" && -x "${CLANG}.exe" ]]; then
        CLANG="${CLANG}.exe"
    fi
    echo "Compiling st-relay-arm64..."
    "${CLANG}" --target=aarch64-linux-android${API_LEVEL} \
        -O2 -Wall -Wextra -fPIE -pie -Wl,-z,max-page-size=16384 \
        "${SCRIPT_DIR}/relay.c" -o "${OUT_DIR}/st-relay-arm64"
    chmod +x "${OUT_DIR}/st-relay-arm64"

    echo "Compiling st-relay-armv7..."
    "${CLANG}" --target=armv7a-linux-androideabi${API_LEVEL} \
        -O2 -Wall -Wextra -fPIE -pie -Wl,-z,max-page-size=16384 \
        "${SCRIPT_DIR}/relay.c" -o "${OUT_DIR}/st-relay-armv7"
    chmod +x "${OUT_DIR}/st-relay-armv7"

    echo "Relay binaries successfully built in ${OUT_DIR}"
    ls -la "${OUT_DIR}/st-relay-arm64" "${OUT_DIR}/st-relay-armv7"
else
    echo "Android NDK not found. To compile manually:"
    echo "  \$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/<host>/bin/clang --target=aarch64-linux-android24 -O2 -fPIE -pie -Wl,-z,max-page-size=16384 relay.c -o st-relay-arm64"
    exit 1
fi
