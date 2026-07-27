
# Introduction

This is a collection of simple RP2350 application examples that run on a [Raspberry Pi Pico 2](https://www.raspberrypi.com/products/raspberry-pi-pico-2/) microcontroller board.


# Repository Structure

```
.
├── Readme.md  
├── .gitignore            
├── scripts/             
│   ├── uf2conv.py         # elf to uf2 converting script
│   └── uf2famillies.json
|
├── include/               # rp2350 simple functionalities libraries
│   ├── debug.h
|   ├── gpio.h
|   ├── ... 
|   └── xosc.h
└── src/
    |
    ├── common/             # implementation of included libraries
    │   ├── debug.c         # implementation of semihosting and blink feature for debugging 
    │   ├── uart.c          # implementation of serial connection initialization
    │   ├── ...             
    │   └── flash.c         # implementation of flash programming and erasing 
    │
    └── examples/           # simple usage of included libraries
        ├── blink_led/
        |   ├── main.c                               # main application testing the example (blinking led)
        |   ├── Makefile                             # build, flash, and debug targets
        |   ├── linker.ld                            # linker description script
        |   └── minimum_arm_image_def_block.s        # minimal IMAGE_DEF block definition and booting code
        ├── uart/
        ├── semihosting/
        ├── flash/
        ├── mpu/
        ...
        └── feature_n/

```

# Example Execution

1. Enter to one of the example directories
```bash
cd src/examples/[example]
```

2. Clean previous executions
```bash
make clean
```

3. Build then flash using OpenOCD
```bash
make
make flash
```


# Build

Two toggles at the top of the `Makefile` control how `program.elf` is built. Each can be set on the command line (e.g. `make debug=0`) or by editing the default in the `Makefile`:

- `debug` — `1` (default) compiles with `-g3 -O0`; `0` compiles with `-O2`.
- `fpu` — `soft` (default), `softfp`, or `hard` selects the float ABI / FPU configuration. The same setting is applied to C, assembler, and link flags.

## Makefile targets

- all (default target): builds program.elf (rebuilds only if sources changed)
- reset: resets RP2350 (a software substitute for pressing the BOOTSEL button)
- flash: flashes program.elf (rebuilds only if sources changed)
- openocd-server: runs openocd gdb server
- debug: runs gdb (connects to openocd gdb server, resets, puts a breakpoint to Reset_Handler)
- size: prints the section sizes of program.elf

# Using the debug probe from WSL

If you are developing inside WSL on Windows, the USB debug probe is not visible to Linux by default. The steps below pass it through to WSL and grant openocd permission to use it.

1. **Install [usbipd-win](https://github.com/dorssel/usbipd-win)** on Windows (one-time): `winget install usbipd`. Plug in the debug probe and run `usbipd list` in an elevated PowerShell to confirm the device's VID:PID is `2e8a:000c` (if not, substitute your value in the steps below). Then bind it once with `usbipd bind -i 2e8a:000c`.

2. **Attach the probe to WSL** every time you plug it in. From an elevated PowerShell:
   ```
   usbipd attach --wsl -i 2e8a:000c -a
   ```
   The `-a` flag auto-reattaches on replug. For convenience, this command can be put in a `.bat` file on the desktop that self-elevates via `powershell Start-Process -Verb RunAs`.

3. **Add a udev rule inside WSL** so openocd (running as your normal user) can open the probe. Your user must be in the `plugdev` group (`sudo usermod -aG plugdev $USER`, then log out/in):
   ```
   echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", ATTRS{idProduct}=="000c", MODE="0660", GROUP="plugdev"' | sudo tee /etc/udev/rules.d/60-picoprobe.rules
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```
   Note: udev requires systemd in WSL. Set `systemd=true` under `[boot]` in `/etc/wsl.conf` and run `wsl --shutdown` from Windows if udev rules don't take effect.

4. **openocd adapter speed.** USB-IP forwarding adds latency; running openocd faster than ~2000 kHz over WSL may produce `USB write: late transfer competed` errors depending on the host environment. The Makefile is configured for 5000 kHz on the `flash`, `openocd-server`, and `reset` targets, which works on both native Linux and WSL — bump it back up if you are running natively and want faster flashing.

After these steps, `make flash` and `make debug` work the same as on a native Linux host.

# License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
