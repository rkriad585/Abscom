#!/bin/sh
# installer.sh - one-line installer for Abscom, a C11 library of data
# structures and a Python-inspired dynamic runtime.
#
# Downloads a prebuilt release asset for the current platform when the
# matching GitHub Release exists, and otherwise downloads the Abscom source
# from GitHub (or reuses the local checkout when run from inside a repository
# clone), builds it, and installs the headers, libraries, and pkg-config file
# to a prefix.
#
# Uses Meson/Ninja when available and falls back to compiling the static
# library directly with cc/gcc. Set ABSCOM_ASSET_URL to an absolute download
# URL to override the prebuilt asset location. Pass --selfuninstall to remove
# everything.
#
#   Install:    curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh
#   Uninstall:  curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh -s -- --selfuninstall
#   Custom:     curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh -s -- --prefix "$HOME/.local"
set -eu

OWNER="rkriad585"
REPO="Abscom"
BRANCH="main"
VERSION="0.2.6"

PREFIX=""
BUILDTYPE="release"
SKIP_TESTS=0
SELFUNINSTALL=0
FORCE_DIRECT=0
CACHE_DIR=""

usage() {
    cat <<EOF
Usage: installer.sh [options]

Options:
  --prefix DIR        Install prefix (default: /usr/local if writable, else \\\$HOME/.local)
  --buildtype TYPE    debug, debugoptimized, release, or plain (default: release)
  --skip-tests        Do not run the test suite
  --selfuninstall     Remove installed Abscom files and caches
  --force-direct      Compile the static library directly with cc/gcc instead of Meson
  --cache-dir DIR     Where sources, the build, and the uninstall manifest live
                      (default: \\\$XDG_CACHE_HOME/abscom or \\\$HOME/.cache/abscom)
  -h, --help          Show this help
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix) PREFIX="$2"; shift 2 ;;
        --buildtype) BUILDTYPE="$2"; shift 2 ;;
        --skip-tests) SKIP_TESTS=1; shift ;;
        --selfuninstall) SELFUNINSTALL=1; shift ;;
        --force-direct) FORCE_DIRECT=1; shift ;;
        --cache-dir) CACHE_DIR="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *)
            echo "installer.sh: unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

if [ -z "$CACHE_DIR" ]; then
    CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/abscom"
fi
BUILD_DIR="$CACHE_DIR/build"
MANIFEST="$CACHE_DIR/manifest.txt"

# Uninstall: remove everything recorded in the manifest, prune empty dirs,
# and delete the cache.
uninstall() {
    if [ -f "$MANIFEST" ]; then
        echo "[abscom] Removing installed Abscom files ..."
        while IFS= read -r f; do
            [ -n "$f" ] && rm -f -- "$f" 2>/dev/null || true
        done < "$MANIFEST"
        while IFS= read -r f; do
            [ -z "$f" ] && continue
            dir=$(dirname -- "$f")
            while [ -n "$dir" ] && [ "$dir" != "$PREFIX" ] && [ "$dir" != "/" ]; do
                rmdir -- "$dir" 2>/dev/null || true
                dir=$(dirname -- "$dir")
            done
        done < "$MANIFEST"
        case "$PREFIX" in
            /|/usr|/usr/local|/opt|/home|/root) ;;
            *) rmdir -- "$PREFIX" 2>/dev/null || true ;;
        esac
    fi
    rm -rf "$CACHE_DIR"
    echo "[abscom] Abscom uninstalled."
    exit 0
}

if [ "$SELFUNINSTALL" = "1" ]; then
    if [ -z "$PREFIX" ] && [ -f "$CACHE_DIR/prefix.txt" ]; then
        PREFIX=$(head -n 1 "$CACHE_DIR/prefix.txt")
    fi
    uninstall
fi

# Detect a local checkout: only trust $0 when it is a real, readable file
# (piped `curl | sh` runs with $0 = "sh").
LOCAL_SRC=""
if [ -n "$0" ] && [ -f "$0" ] && [ -r "$0" ]; then
    DIR=$(CDPATH= cd -- "$(dirname -- "$0")" 2>/dev/null && pwd)
    if [ -f "$DIR/meson.build" ]; then
        LOCAL_SRC="$DIR"
    fi
fi

if [ -z "$PREFIX" ]; then
    if [ "$(id -u)" = "0" ]; then
        PREFIX="/usr/local"
    elif [ -w /usr/local ]; then
        PREFIX="/usr/local"
    else
        PREFIX="$HOME/.local"
    fi
fi

INC_DIR="$PREFIX/include/abscom"
LIB_DIR="$PREFIX/lib"
PKG_DIR="$LIB_DIR/pkgconfig"

# ---- prebuilt release asset (fast path) ----
dl() {
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$1" -o "$2"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "$1" -O "$2"
    else
        echo "installer.sh: need curl or wget to download the prebuilt asset" >&2
        return 1
    fi
}

print_summary() {
    echo ""
    echo "[abscom] Abscom installed."
    echo "[abscom]   prefix    : $PREFIX"
    echo "[abscom]   headers   : $INC_DIR"
    echo "[abscom]   library   : $LIB_DIR"
    echo "[abscom]   pkg-config: PKG_CONFIG_PATH=$PKG_DIR pkg-config --cflags --libs abscom"
    echo ""
    echo "[abscom] Compile a quick test:"
    echo "[abscom]   cc -std=c11 hello.c -I$PREFIX/include -L$PREFIX/lib -labscom -o hello"
    echo "[abscom] Uninstall:"
    echo "[abscom]   curl -fsSL https://raw.githubusercontent.com/$OWNER/$REPO/main/installer.sh | sh -s -- --selfuninstall"
}

OS_NAME="$(uname -s 2>/dev/null || echo Unknown)"
OS_ARCH="$(uname -m 2>/dev/null || echo unknown)"
OS_ID=""
case "$OS_NAME" in
    Linux*) OS_ID="linux" ;;
    Darwin*) OS_ID="macos" ;;
    MINGW*|MSYS*|CYGWIN*) OS_ID="windows" ;;
esac
ARCH_ID=""
case "$OS_ARCH" in
    x86_64|amd64) ARCH_ID="x86_64" ;;
    aarch64|arm64) ARCH_ID="arm64" ;;
esac

INSTALLED_FROM_ASSET=""
if [ -n "$OS_ID" ] && [ -n "$ARCH_ID" ]; then
    ASSET="abscom-$OS_ID-$ARCH_ID.tar.gz"
    ASSET_TMP="$CACHE_DIR/$ASSET"
    mkdir -p "$CACHE_DIR"
    DOWNLOADED=0
    if [ -n "${ABSCOM_ASSET_URL:-}" ]; then
        echo "[abscom] Downloading prebuilt asset ($ABSCOM_ASSET_URL) ..."
        if dl "$ABSCOM_ASSET_URL" "$ASSET_TMP"; then DOWNLOADED=1; fi
    else
        for BASE in \
            "https://github.com/$OWNER/$REPO/releases/download/v$VERSION/$ASSET" \
            "https://github.com/$OWNER/$REPO/releases/latest/download/$ASSET"; do
            echo "[abscom] Downloading prebuilt asset ($BASE) ..."
            if dl "$BASE" "$ASSET_TMP"; then DOWNLOADED=1; break; fi
        done
    fi
    if [ "$DOWNLOADED" = "1" ]; then
        echo "[abscom] Installing from prebuilt release asset ..."
        PREBUILT_DIR="$CACHE_DIR/prebuilt"
        rm -rf "$PREBUILT_DIR"
        mkdir -p "$PREBUILT_DIR"
        tar -xzf "$ASSET_TMP" -C "$PREBUILT_DIR"

        # Manifest = every file inside the asset, resolved under $PREFIX.
        # `tr -d '\r'` guards against CRLF output from tar on Windows/MSYS.
        tar -tzf "$ASSET_TMP" | tr -d '\r' \
            | while IFS= read -r entry; do
                case "$entry" in
                    */) continue ;;
                esac
                printf '%s\n' "$PREFIX/$entry"
            done > "$MANIFEST"
        rm -f "$ASSET_TMP"
        if [ ! -s "$MANIFEST" ]; then
            echo "installer.sh: could not determine installed files for the uninstall manifest" >&2
            exit 1
        fi

        mkdir -p "$INC_DIR" "$LIB_DIR" "$PKG_DIR"
        cp -r "$PREBUILT_DIR"/include/abscom/. "$INC_DIR/"
        for f in "$PREBUILT_DIR"/lib/*; do
            [ -e "$f" ] || continue
            cp -rP "$f" "$LIB_DIR/"
        done
        if [ -d "$PREBUILT_DIR/bin" ]; then
            mkdir -p "$PREFIX/bin"
            for f in "$PREBUILT_DIR"/bin/*; do
                [ -e "$f" ] || continue
                cp -rP "$f" "$PREFIX/bin/"
            done
        fi

        sed "s|^prefix=.*|prefix=$PREFIX|" "$PKG_DIR/abscom.pc" > "$PKG_DIR/abscom.pc.tmp"
        mv "$PKG_DIR/abscom.pc.tmp" "$PKG_DIR/abscom.pc"

        printf '%s\n' "$PREFIX" > "$CACHE_DIR/prefix.txt"

        INSTALLED_FROM_ASSET=1
    else
        echo "[abscom] Prebuilt asset not available; building from source."
        rm -f "$ASSET_TMP"
    fi
fi

if [ -n "$INSTALLED_FROM_ASSET" ]; then
    print_summary
    exit 0
fi

SRC_DIR="$CACHE_DIR/src/Abscom-main"
if [ -n "$LOCAL_SRC" ]; then
    SRC_DIR="$LOCAL_SRC"
else
    echo "[abscom] Downloading $OWNER/$REPO@$BRANCH ..."
    mkdir -p "$CACHE_DIR/src"
    TARBALL="$CACHE_DIR/abscom-$BRANCH.tar.gz"
    URL="https://github.com/$OWNER/$REPO/archive/refs/heads/$BRANCH.tar.gz"
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$URL" -o "$TARBALL"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "$URL" -O "$TARBALL"
    else
        echo "installer.sh: need curl or wget to download the source" >&2
        exit 1
    fi
    rm -rf "$SRC_DIR"
    mkdir -p "$SRC_DIR"
    tar -xzf "$TARBALL" -C "$CACHE_DIR/src"
    rm -f "$TARBALL"
fi

if [ ! -f "$SRC_DIR/meson.build" ]; then
    echo "installer.sh: source directory is missing meson.build: $SRC_DIR" >&2
    exit 1
fi

# ---- build & install ----
meson_cmd=""
if command -v meson >/dev/null 2>&1; then
    meson_cmd="meson"
elif command -v python3 >/dev/null 2>&1 && python3 -c 'import mesonbuild' >/dev/null 2>&1; then
    meson_cmd="python3 -m mesonbuild"
fi

if [ "$FORCE_DIRECT" != "1" ] && [ -n "$meson_cmd" ] && command -v ninja >/dev/null 2>&1; then
    echo "[abscom] Building with Meson ($BUILDTYPE) ..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$(dirname "$BUILD_DIR")"
    $meson_cmd setup "$BUILD_DIR" "$SRC_DIR" --buildtype "$BUILDTYPE" --prefix "$PREFIX" --libdir=lib
    $meson_cmd compile -C "$BUILD_DIR"
    if [ "$SKIP_TESTS" != "1" ]; then
        echo "[abscom] Running tests ..."
        $meson_cmd test -C "$BUILD_DIR"
    fi
    $meson_cmd install -C "$BUILD_DIR"

    echo "[abscom] Recording install manifest ..."
    $meson_cmd introspect --installed "$BUILD_DIR" \
        | grep -o '"[^"]*"' | tr -d '"' | sed 's/\\\\/\\/g' \
        | while IFS= read -r p; do
            case "$p" in
                "$PREFIX"*) printf '%s\n' "$p" ;;
            esac
        done > "$MANIFEST"
    if [ ! -s "$MANIFEST" ]; then
        echo "installer.sh: could not determine installed files for the uninstall manifest" >&2
        exit 1
    fi
    printf '%s\n' "$PREFIX" > "$CACHE_DIR/prefix.txt"
else
    if [ "$FORCE_DIRECT" = "1" ]; then
        echo "[abscom] --force-direct given; compiling the static library directly ..."
    else
        echo "[abscom] Meson/Ninja not found; compiling the static library directly ..."
    fi
    if command -v gcc >/dev/null 2>&1; then
        CC="gcc"; AR="ar"
    elif command -v cc >/dev/null 2>&1; then
        CC="cc"; AR="ar"
    else
        echo "installer.sh: no C compiler found (install gcc or clang) and rerun" >&2
        exit 1
    fi

    OBJ_DIR="$CACHE_DIR/direct/obj"
    rm -rf "$OBJ_DIR"
    mkdir -p "$OBJ_DIR"
    mkdir -p "$INC_DIR" "$LIB_DIR" "$PKG_DIR"
    SRC_INC="$SRC_DIR/include"

    for src in "$SRC_DIR"/src/*.c; do
        obj="$OBJ_DIR/$(basename "${src%.c}").o"
        "$CC" -std=c11 -O2 -DABS_BUILDING_LIBRARY -I"$SRC_INC" -c "$src" -o "$obj"
    done
    "$AR" rcs "$LIB_DIR/libabscom.a" "$OBJ_DIR"/*.o
    cp "$SRC_INC"/abscom/*.h "$INC_DIR/"

    OS="$(uname -s 2>/dev/null || echo Unknown)"
    PRIVATE=""
    case "$OS" in
        MINGW*|MSYS*|CYGWIN*) PRIVATE="Libs.private: -lws2_32" ;;
    esac
    cat > "$PKG_DIR/abscom.pc" <<EOF
prefix=$PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: abscom
Description: A C11 library of reusable data structures and a Python-inspired dynamic runtime
Version: $VERSION
Libs: -L\${libdir} -labscom
$PRIVATE
Cflags: -I\${includedir}
EOF

    {
        printf '%s\n' "$LIB_DIR/libabscom.a"
        printf '%s\n' "$PKG_DIR/abscom.pc"
        for h in "$INC_DIR"/*.h; do printf '%s\n' "$h"; done
    } > "$MANIFEST"
    printf '%s\n' "$PREFIX" > "$CACHE_DIR/prefix.txt"
fi

print_summary
