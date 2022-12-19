<!--
.\" Copyright (C) 2019 VirtualSquare. Project Leader: Renzo Davoli
.\"
.\" This is free documentation; you can redistribute it and/or
.\" modify it under the terms of the GNU General Public License,
.\" as published by the Free Software Foundation, either version 2
.\" of the License, or (at your option) any later version.
.\"
.\" The GNU General Public License's references to "object code"
.\" and "executables" are to be interpreted as the output of any
.\" document formatting or typesetting system, including
.\" intermediate and printed output.
.\"
.\" This manual is distributed in the hope that it will be useful,
.\" but WITHOUT ANY WARRANTY; without even the implied warranty of
.\" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\" GNU General Public License for more details.
.\"
.\" You should have received a copy of the GNU General Public
.\" License along with this manual; if not, write to the Free
.\" Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
.\" MA 02110-1301 USA.
.\"
-->
# NAME

randmac -- generate random MAC addresses

# SYNOPSIS

`randmac` [*options*]

# DESCRIPTION

`randmac` prints a random MAC address. This utility has been designed to
give random MAC addresses to interfaces of virtual machines.

# OPTIONS
  `-l`, `--local`
: Generate a local administered MAC.

  `-g`, `--global`
: Generate a global unique MAC.

  `-u`, `--unicast`
: Generate a MAC for unicast.

  `-m`, `--multicast`
: Generate a MAC for multicast.

  `-U`, `--uppercase`
: Print uppercase hex digits.

  `-e`, `--eui64`
: Generate an EUI64 address.

  `-o` *oui_addr*, `--oui` *oui_addr*
: Set the Organizationally Unique Identifier (OUI).

  `-v` *vendor*, `--vendor` *vendor*
: Set the OUI of a specific vendor.

  `-q`, `--qemu`
: Set the standard OUI of qemu/kvm.

  `-x`, `--xen`
: Set the standard OUI of xen.

  `-h`, `--help`
: Display a short help message and exit.

# EXAMPLE

Start a kvm VM using a random generated MAC address (while preserving
the standard qemu's OUI)

```sh
$ qemu-system-x86_64 -accel kvm ... \
  -device e1000,netdev=vde0,mac=`randmac -q` \
  -netdev vde,id=vde0,sock=vde://
```

# SEE ALSO
qemu(1)

# AUTHOR
VirtualSquare. Project leader: Renzo Davoli.

