![Weird A' Header](Media/WesterOS_Header.png "WesterOS Header")

> **Warning!** Almost _nothing_ is complete yet. Everything here is experimental AND
> in VERY ACTIVE development. Proceed with caution.

## **Motive**

Computers are _weird_. Weird in the sense that they feel magical if you don't 
know how they work. If, and when, you start learning how it works, that magical
feeling fades away. You will either end up with "Wow! This is brillaint" or 
"Wow... this is shit."

So, you have basically _two_ options:

* **Take the blue pill:** Continue using the computer like normal. Magical feeling!
* **Take the red pill:** Go on a learning frenzy. _Risk of ending up as mad men +45%_

In case it wasn't obvious yet, I _took_ the latter.

Basically, I wanted to learn more about "computers". Working on the relatively 
high-level software is pretty fun, but I have a bad habit of ending up looking
at low-level internels and workings of stuff.

> "Oh, so my calls to `malloc()` or `new` ends up calling the syscall `mmap()`?
> Let's check it's implementation.. Oh, what's `VMA`? What's a `MMU`? What's a
> `TLB`?? What's a `TCR` and a `MAIR`??? Why am I here.. Just to suffer..?"

There was these layers of **abstraction** that I had no idea of! I should've
stopped there, but no. Instead, I said: "Okay. I really like the _call-of-the-void_
feeling of low-level stuff. Let's go wild and try to learn all the abstractions
by writing a 'simple' operating system. What could go wrong?" And now, here I am.

## **WesterOS**

Work-in-progress hobbyist operating system based on MIT's xv6[^1]. The name is 
from the most political & messed-up continent in the world of Game of Thrones.[^2]
I really like GoT (yeah, judge me all you want) and the political messiness of it
felt similar to my own goals with this project. _Everything is all over the place
and nothing makes sense._

This project exists only to help _me_ learn about operating systems and to have
some fun along the way. I don't claim to know much about _OS_ development. So,
the practices, design chocies and some implementations you see here WILL scare
you.

> I suggest to kepp your expectations LOW >.<

> **Important Note:** WesterOS is in _very early_ development. Stuff WILL change,
> brake or straight-up be stupid. I am still yet to define an end goal. Until
> then expect [Everything, Everywhere, All at Once](https://a24films.com/films/everything-everywhere-all-at-once).

## **Features**

* ARM64 kernel /w pre-emptive multithreading (Based on xv6)
* Unix-like syscalls (Based on xv6)
* `/dev` and `/proc` filesystems
* Virtual Memory Management (Based on xv6)
* 2-Level page tables /w 4K size
* CLI based interface (Based on xv6)
* Simple userland programs (Based on xv6)
* Terminal games (?)

## **Overview**

Let's dive a bit more into inner working of things. The target machine is 
_very specific & static_. I wanted to keep it as simple as possible by supporting
only the ARM64 architecture (ARMv8) and QEMU's ARM Virt machine.

Here's the target machine.

!['Hardware' Specs and QEMU Config](Media/Hardware_w_QEMU.png "Hardware & QEMU")

Only the must _basic_ peripherals are defined. Sadly, this means no USB, NVME and
NIC. :( Maybe in the future?

Here's the peripherals that I _plan_ to support.

* [ARM PrimeCell RTC PL031](https://developer.arm.com/documentation/ddi0224/)
* [ARM PrimeCell UART PL011](https://developer.arm.com/documentation/ddi0183/)
* [ARM PrimeCell GPIO PL061](https://developer.arm.com/documentation/ddi0190/)
* [QEMU RAM FrameBuffer (?)](https://github.com/qemu/qemu/blob/master/hw/display/ramfb.c)

[TODO: Kernel Specs (Similar to the image above)]()

TODO: Overall OS specifications. What is what and the future goals.

### Directory Structure
```
|-- Build              <- Compiled objects, binaries & debug files
|-- Documents          <- Reference documents
|-- Emulation          <- QEMU scripts & Device Tree Structure
|-- Kernel             <- The source code. Headers, C and C++ files
|   `-- Arch           <- Architecture related code
|   `-- Drivers        <- Driver source and header files
|   `-- Library        <- Library source and header files
|   `-- Include        <- Kernel header files
|-- Media              <- Images and other media
|-- Toolchain          <- Cross-compiling environment
|-- Userland           <- User level source code
|   `-- Dir.           <- TBD.
|-- .gitignore         <- Good ol' .gitignore
|-- Makefile           <- Makefile
`-- README.md          <- Main README
```

### Bootstrapping

TODO: How does everything starts? Why the things I do are the way they are?

### Kernel

TODO: Tell everyone that this part is MAINLY referenced from xv6 and why so.

### Userland

TODO: Some fun stuff. What awaits someone one they launch the OS?

## Build

TODO: How to build everything. ARM toolchain, make system and host requirements.

## Run/Emulate

TODO: QEMU emulation. Run the OS and see what's in there.

## Explore

TODO: Navigate the user to another README which is basically the documentation.
