#!/bin/sh
# build.sh - Build Abscom locally with Meson and Ninja, optionally installing it.
#
#   ./build.sh                      build (release) and run the tests
#   ./build.sh --install            build, test, and install to /usr/local
#   ./build.sh --prefix "$HOME/.local" --install
#   ./build.sh --clean --skip-tests
set -eu

PREFIX=""
BUILDTYPE="release"
BUILDDIR="build"
DO_INSTALL=0
CLEAN=0
SKIP_TESTS=0

usage() {
    cat <<'EOF'
Usage: build.sh [options]

Options:
  -p, --prefix DIR     Install prefix (used with --install)
  -b, --buildtype TYPE debug, debugoptimized, release, or plain (default: release)
  -i, --install        Run 'meson install' after building
  -c, --clean          Remove the build directory first
  -s, --skip-tests     Do not run the test suite
  -h, --help           Show this help
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -p|--prefix) PREFIX="$2"; shift 2 ;;
        -b|--buildtype) BUILDTYPE="$2"; shift 2 ;;
        -i|--install) DO_INSTALL=1; shift ;;
        -c|--clean) CLEAN=1; shift ;;
        -s|--skip-tests) SKIP_TESTS=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *)
            echo "build.sh: unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

meson_cmd=""
if command -v meson >/dev/null 2>&1; then
    meson_cmd="meson"
elif command -v python3 >/dev/null 2>&1 && python3 -c 'import mesonbuild' >/dev/null 2>&1; then
    meson_cmd="python3 -m mesonbuild"
else
    echo "build.sh: meson not found. Install it with:  pip install meson ninja" >&2
    exit 1
fi

if ! command -v ninja >/dev/null 2>&1; then
    echo "build.sh: ninja not found. Install it with:  pip install meson ninja" >&2
    exit 1
fi

if [ "$CLEAN" = "1" ] && [ -d "$BUILDDIR" ]; then
    echo "Removing $BUILDDIR ..."
    rm -rf "$BUILDDIR"
fi

if [ -d "$BUILDDIR" ]; then
    echo "Reconfiguring $BUILDDIR ..."
    $meson_cmd configure "$BUILDDIR" --buildtype "$BUILDTYPE"
    if [ -n "$PREFIX" ]; then
        $meson_cmd configure "$BUILDDIR" --prefix "$PREFIX"
    fi
else
    echo "Configuring $BUILDDIR (buildtype: $BUILDTYPE) ..."
    if [ -n "$PREFIX" ]; then
        $meson_cmd setup "$BUILDDIR" --buildtype "$BUILDTYPE" --prefix "$PREFIX"
    else
        $meson_cmd setup "$BUILDDIR" --buildtype "$BUILDTYPE"
    fi
fi

echo "Compiling ..."
$meson_cmd compile -C "$BUILDDIR"

if [ "$SKIP_TESTS" != "1" ]; then
    echo "Running tests ..."
    $meson_cmd test -C "$BUILDDIR"
fi

if [ "$DO_INSTALL" = "1" ]; then
    echo "Installing ..."
    $meson_cmd install -C "$BUILDDIR"
fi

echo ""
echo "Build complete."
echo "  Libraries:  $BUILDDIR/libabscom.a  (static)"
echo "              $BUILDDIR/libabscom.so / libabscom.dylib  (shared)"
echo "  Tests:      $BUILDDIR/tests/"
echo "  Examples:   $BUILDDIR/examples/"
if [ "$DO_INSTALL" = "1" ]; then
    echo "  Installed to prefix: $PREFIX"
fi
echo ""
echo "Quick test of the installed library:"
echo "  cc -std=c11 hello.c -I<prefix>/include -L<prefix>/lib -labscom -o hello"
echo "  pkg-config --cflags --libs abscom   (set PKG_CONFIG_PATH to <prefix>/lib/pkgconfig)"
