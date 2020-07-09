# randmac

Some Virtual Machine Monitors need to specify a MAC address as a command line parameter when starting a VM from shell or as a statement of a script.

It is the case of qemu/kvm: the default configuration assigns 52:54:00:12:34:56 to all the VM so two qemu machines
on the same virtual ethernet cannot communicate if the default value has not been changed.

This tool solves the problem to generate random MAC addresses.


## Install

get the source code, from the root of the source tree run:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

## Syntax

`randmac` [*options*]

### options

  * `-l`, `--local`:
    Generate a local administered MAC.

  * `-g`, `--global`:
    Generate a global unique MAC.

  * `-u`, `--unicast`:
    Generate a MAC for unicast.

  * `-m`, `--multicast`:
    Generate a MAC for multicast.

  * `-U`, `--uppercase`:
    Print uppercase hex digits.

  * `-e`, `--eui64`:
    Generate an EUI64 address.

  * `-o` *oui_addr*, `--oui` *oui_addr*:
    Set the Organizationally Unique Identifier (OUI).

  * `-v` *vendor*, `--vendor` *vendor*:
    Set the OUI of a specific vendor.

  * `-q`, `--qemu`:
    Set the standard OUI of qemu/kvm.

  * `-x`, `--xen`:
    Set the standard OUI of xen.

  * `-h`, `--help`:
    Display a short help message and exit.

### Examples:

The simplest command, without any option:
```bash
$ randmac
16:38:5a:f6:81:33
```

A common case: provide kvm virtual machines wih random MAC addresses:
```bash
$ kvm -cdrom live.iso -device e1000,netdev=vde0,mac=`randmac -q` \
     -netdev vde,id=vde0,sock=vde://
```

Set the qemu's OUI and use uppercase letters in hex:
```bash
$ randmac -qU
52:54:00:1F:C5:C8
```

Print a fake random EUI64 apparently coming from an Intel controller:
```bash
$ randmac -e -v Intel
02:03:47:ff:fe:2d:be:88
```

## Author
Renzo Davoli, VirtualSquare Team. 2020