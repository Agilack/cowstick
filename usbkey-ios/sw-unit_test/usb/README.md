# USB unit-test

The goal of this program is to develop a USB stack for cowstick. This is put
into a unit-test to offer a simple way to test it :)

## Why ??

If your read this the first question that comes to mind is probably "Why a
custom USB stack ?" There are two main reasons :

 * The "official" atmel/microchip stack use static datas (like usb_d_inst or
   dev_inst) For cowstick, i want to share code of USB stack between bootloader
   and firmware and that's a bit more difficult with static datas.

 * All the stacks i've seen are written to be portable and/or generic. This
   is great but use lot of code. This one is made to have a small memory
   footprint. I want (i hope) a bootloader that fits into less than 16kB.

## HID Mouse

The unit-test contains an HID mouse. This is one of the simplest device (only
one endpoint) that is easy to test (detected by all OS). When loaded this
virtual mouse will move automatically into a square.
