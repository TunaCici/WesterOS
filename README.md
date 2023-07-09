![Weird A' Header](Media/WesterOS_Header.png "WesterOS Header")

> **Warning!** Almost _nothing_ is complete yet. Everything here is experimental AND
> in VERY ACTIVE development. Proceed with caution.

## **Motivation**

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

> "Oh, so my calls to `malloc()` and `new` ends up calling the syscall `mmap()`?
> Let's check it's implementation.. Oh, what's `VMA`? What's a `MMU`? What's a
> `TLB`?? What's the `TCR` and the `MAIR`??? How did I get here? Why am I here..
> Just to suffer..? :("

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

## Installation

To build & run the operating system you need three main things: `ARM GNU Toolchain`, `QEMU` and a little bit of Patience™.

It is possible to build everything on your favorite operating system. `ARM GNU Toolchain` is available on both **Windows**, **macOS** and **GNU/Linux**. However, I have NOT tested **Windows** yet. So, you are alone in that space. Sorry :(

The below steps are for **GNU/Linux (Aarch64)** hosts.

**0. Make sure you have `git` and `make`**
```bash
$ apt install git make # if using `apt`
$ pacman -S git make # if using `pacman`
```

**1. Clone this repository**
```bash
$ git clone https://github.com/TunaCici/WesterOS.git
```

**2. Download the latest `ARM GNU Toolchain`**

Navigate to [ARM GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) web page.

Choose the appropriate **Aarch64 bare-matel target** to download. The version _should not_ matter, so choose the latest one. However, which hosted toolchain you download DOES matter. Choose the one that is created for your OWN operating system.

For example, if your OS is **GNU/Linux (x86_64)** then you download:
```text
https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-elf.tar.xz
```

```bash
$ cd WesterOS/Toolchain
$ wget https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/arm-gnu-toolchain-12.2.rel1-aarch64-aarch64-none-elf.tar.xz
```

> It is your responsibility to verify the "integrity" and "signature" of the downloaded file. Use the **SHA256** keys provided in the downloads page.

**3. Extract the downloaded `ARM GNU Toolchain`**

Make sure to extract it while in `Westeros/Toolchain`.

```bash
$ tar -xvf arm-gnu-toolchain-12.2.rel1-aarch64-aarch64-none-elf.tar.xz
```

**4. Modify the `TOOLCHAIN_PATH` in the `Makefile`**

The `TOOLCHAIN_PATH` should point to your newly downloaded and extracted `ARM GNU Toolchain`. Since your host OS and toolchain version might be different than mine, you MUST edit the PATH variable.

If not set correctly, the `make` process WILL fail with an error message like:
```text
make[1]: Toolchain/arm-gnu-toolchain-12.2.rel1-darwin-arm64-aarch64-none-elf/bin/aarch64-none-elf-as: No such file or directory
```

So, make sure to edit the `TOOLCHAIN_PATH`.
```bash
# Open the main Makefile /w your favorite text editor
$ vim Makefile

# And change the `TOOLCHAIN_PATH` accordingly. For example..
> TOOLCHAIN_PATH=Toolchain/arm-gnu-toolchain-12.2.rel1-darwin-arm64-aarch64-none-elf

# Save & exit
```

**5. Build using `make`**

```bash
$ make all
```

The build ends with a message **Build complete. Enjoy life <3**. If you don't see it, contact me. I'll try the fix the issue >.<

## Run/Emulate

WesterOS can ONLY be run using QEMU. I have no plans to make a fully-bootable image for real-world hardware. Originally my idea was to emulate a _Raspberrry Pi 4b_, but I realized it might not that be "beginner friendly" when testing and deubgging. So, QEMU it is!

> Shameless plug time! If you want more about QEMU, visit my [QEMU_Starter](https://github.com/TunaCici/QEMU_Starter) GitHub thingy.

**0. Make sure you have `qemu-system-aarch64` installed**
```bash
$ apt install qemu-system qemu-utils # if using `apt`
$ pacman -S qemu-full # if using `pacman`
```

**1. Launch WesterOS**
```bash
$ make run
```

> As of 9 July 2023, the WesterOS is straight-up empty! So, you can't do anything except to see some very basic kernel messages on the terminal :/
> 
> Then again, WesterOS is a _hobbyist operating system_ and a _learning process_. You should really try to explore it's source code. I'm sure you will have more fun there.

## Explore

TODO: Navigate the user to another README which is basically the documentation.

[1]: https://pdos.csail.mit.edu/6.828/2012/xv6.html
[2]: https://gameofthrones.fandom.com/wiki/Westeros
