# Abscom build image: installs dependencies, builds the library, runs the test
# suite, and installs headers + libraries to /usr/local.
FROM debian:bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        meson \
        ninja-build \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN meson setup build --buildtype=release --prefix=/usr/local \
    && meson compile -C build \
    && meson test -C build \
    && meson install -C build

# Runtime image: only the installed headers and libraries.
FROM debian:bookworm-slim AS runtime
COPY --from=build /usr/local/ /usr/local/
