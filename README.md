# DumbCycle

A commit-by-commit[^1] (hopefully) instructional example of how to make a game
using the dumb frame buffer interface of the Linux Direct Rendering Manager.
The entire game is ~1000 lines of
C99 code with a small `x86_64` assembly runtime. No libraries or preprocessor
directives were allowed, even libc.

The intent is for a reader to step through each commit, starting from
a basic executable and building up to a double buffered snake/LightCycle game.
I also [wrote an article][13] going through the development step-by-step.

## Requirements

In order to build the game on an `x86_64` Linux machine you will need a C
compiler that supports the C99 standard along with [make][7] and [binutils][8]
for the `as` assembler and `ld` linker.

To run the game you will need a working
`/dev/dri/card0` device and a working keyboard device in `/dev/input/`,
and the game must have permission to access these files.
The game will take over an entire display as well as the system
keyboard, so it will be necessary for you to turn off your window
manager/compositor if you usually run one.

```
make
sudo ./dumb_cycle
```

![DumbCycle](https://musing.permutationlock.com/dumb_cycle/dumb_cycle.gif)

You can also run the game in a virtual machine if you install [QEMU][9] and
[tiger vnc][10].

```
make test
```

The command will download a [`vm.tar.gz`][11] zip file
containing a Linux kernel and a basic initial ram filesystem using
[toybox][6] for a simple shell environment. If you want to build the kernel
and toybox yourself a corresponding [`vm_src.tar.gz`][12] is provided.

## References

 - [drm-howto][4]
 - [barebones linux][5]
 - [toybox][6]
 - [musl libc][1]
 - [libdrm][2]
 - [linux source][3]

[^1]: I have since added a few commits and modified the article slightly, but in
      general you should still be able to follow the article sections while stepping
      through the commit history.

[1]: https://musl.libc.org/
[2]: https://gitlab.freedesktop.org/mesa/drm
[3]: https://github.com/torvalds/linux
[4]: https://github.com/dvdhrm/docs/tree/master/drm-howto
[5]: https://suchprogramming.com/barebones-linux-system/
[6]: https://github.com/landley/toybox
[7]: https://www.gnu.org/software/make/
[8]: https://www.gnu.org/software/binutils/
[9]: https://www.qemu.org/
[10]: https://tigervnc.org/
[11]: https://musing.permutationlock.com/dumb_cycle/vm.tar.gz
[12]: https://musing.permutationlock.com/dumb_cycle/vm_src.tar.gz
[13]: https://musing.permutationlock.com/dumb_cycle/
