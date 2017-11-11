# Cowstick Bootloader

To offer a simple and flexible firmware upgrade, cowstick contains a bootloader.
This program also provide an API to allow main application to reuse functions
and avoid multiple copies of generic methods. Here a list of main features :

  * Network oriented interface (show as Ethernet over USB interface),
  * Contains a minimalist HTTP server (not already available)
  * Small memory footprint (less than 20kB)
  * API to allow reuse functions into main firmware

# Usage

The bootloader is placed at the begining of flash memory, where processor
start code execution. In all cases, peripherals and low-level initialization
are made on startup (configure IOs, clocks, ...). At this point, three is
three possible behaviors :

 * If the push button is pressed on startup, cowstick start in bootloader
   mode.
 * Flash is read at address 0x4000. If a valid stack address is found
   (an address that point into internal SRAM) then, the bootloader get firmware
   entry address from second vector (at 0x4004) and jump at this address.
 * If no valid stack address is found, firmware is considered invalid and
   cowstick start in bootloader mode.

## License

CowStick-bootloader is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License version 3 as published
by the Free Software Foundation. You should have received a copy of the GNU
Lesser General Public License along with this program, see LICENSE.md file for
more details. This program is distributed WITHOUT ANY WARRANTY.
