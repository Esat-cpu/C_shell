# she-shell

[![CI](https://github.com/Esat-cpu/she-shell/actions/workflows/ci.yml/badge.svg)](https://github.com/Esat-cpu/she-shell/actions/workflows/ci.yml)
![Version](https://img.shields.io/github/v/tag/Esat-cpu/she-shell)
![License](https://img.shields.io/github/license/Esat-cpu/she-shell)

A POSIX-inspired shell written in C, featuring an AST-based parser, and support
for common shell operators, running either as an interactive shell or executing
script files.

---

## Usage

```bash
she                   # for interactive mode
she <script file>     # execute the script file
she -c <COMMAND>      # run the command
she --help            # show usage message
```

> `she` reads ~/.sherc file if it exists (unless --no-profile option is used).

---

## Installation

```bash
git clone https://github.com/Esat-cpu/she-shell
cd she-shell
```

`she` links against `readline` for interactive input.
Installing the `-dev` package below also installs the
runtime library as a dependency on most distros.

if you don't have the readline lib:
```bash
# for debian/ubuntu
sudo apt install libreadline-dev

# for RHEL/Fedora/CentOS
sudo dnf install readline-devel
# or
sudo yum install readline-devel

# for arch/manjaro
sudo pacman -S readline

# for alpine
sudo apk add readline-dev
```

---

## Compilation

There are three build configurations. ASAN mode for build with
AddressSanitizer, DEBUG mode for debug build and release mode. You can combine
ASAN mode with the other two.


To build the shell in different modes, run:
```bash
make                # release
make asan           # release-asan
DEBUG=1 make        # debug
DEBUG=1 make asan   # debug-asan
```

Executing:
```bash
./build/release/she       # release
./build/debug/she         # debug
./build/release-asan/she  # release + ASan
./build/debug-asan/she    # debug + ASan
```

---

## Testing

Unit tests (parser, tokenizer, expansion, ...) and integration tests (pytest,
black-box, execution) are separated and still can be used together.

```bash
make test-unit  # C unit tests
make test-py    # integration tests with pytest
make test       # both
make test-asan  # both with address sanitizer
```

CI runs on Ubuntu (glibc), Alpine (musl) and Ubuntu with AddressSanitizer on
every push.

## Cleaning
```bash
make clean
```

