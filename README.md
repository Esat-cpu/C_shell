# C_shell

A simple Linux shell that is written in C.

---

## Installation

```bash
git clone https://github.com/Esat-cpu/C_shell
cd C_shell
```

if you don't have the readline lib:
```bash
# for debian/ubuntu
sudo apt install libreadline-dev

# for RHEL/Fedora/CentOS
sudo dnf install readline-devel
# or
sudo yum install readline-devel

# for arch/manjaro
pacman -S readline
```

## Compilation

To build the shell, run:
```bash
make
```

Execute:
```bash
./shell
```

---

### Testing
```bash
make test
```

### Cleaning
```bash
make clean
```

