---
title: fast-ninja
---
## What Is fast-ninja?

The [Ninja Build System](https://ninja-build.org) is not designed to be used directly. fast-ninja is a light-weight preprocessor that adds the missing features and conveniences to make it usable almost directly.

## Why Use fast-ninja?

Most other build systems that can use Ninja are centered around building executables for modern computers on many platforms and add a lot of weight to accomplish that. They are therefore a bad fit for projects that don't need any of that, like cross-building programs for retro computers.

fast-ninja is designed for projects that don't use a modern compiler toolchain but use tools that already work across multiple platforms.

## Getting Started

First, [build and install](https://github.com/T-Pau/fast-ninja/blob/main/INSTALL.md) fast-ninja.

Then, set up the build directory:

```sh
mkdir build
cd build
fast-ninja ..
```

Lastly, build your project:

```sh
ninja
```

## Staying in Touch

If you found a problem, please [create an issue on GitHub](https://github.com/T-Pau/fast-ninja/issues/new/choose) or let us know at [fast-ninja@tpau.group](mailto:fast-ninja@tpau.group).

Also let us know if the documentation is incomplete, inaccurate, or hard to understand.
