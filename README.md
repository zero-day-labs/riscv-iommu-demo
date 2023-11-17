# RISC-V IOMMU Demo

## License

This work is licensed under the Apache-2.0 License. See the [LICENSE](./LICENSE) file for more details.

## Table of Contents

- [About the Demo](#about-the-demo)
- [Dependencies and Requirements](#dependencies-and-requirements)
- [Setting up the Demo](#setting-up-the-demo)
    - [Synthesizing the RTL Design](#synthesizing-the-rtl-design)
    - [Building the Demo](#building-the-demo)
    - [Copying the image to an SD card](#copying-the-image-to-an-sd-card)
- [Running the Demo](#running-the-demo)

***

## About the Demo

This repository contains source code and instructions to run a functional demo of the [RISC-V IOMMU IP](https://github.com/zero-day-labs/riscv-iommu) developed by the Zero-Day Labs team. 

The demo is carried out in a Genesys2 FPGA board, and runs atop a [CVA6-based SoC](https://github.com/zero-day-labs/cva6/tree/feat/iommu-demo) with support for the RISC-V Hypervisor extension, with one DMA device and the RISC-V IOMMU IP integrated. The software stack encompasses [OpenSBI](https://github.com/riscv-software-src/opensbi) and [Bao hypervisor](https://github.com/bao-project/bao-hypervisor) with one baremetal VM running atop.

In this demo, a DMA device is programmed to tamper critical memory regions of one of two targets: **Bao** or **OpenSBI** (firmware). The IOMMU IP is enabled by default, but the user may disable it to see the differences. The user navigates through the demo using the push buttons in the board, and the output is printed to the console.

## Dependencies and Requirements

In order to build and run the demo you need to download/install the following tools:

| Requirement | Version | Used to |
| - | - | - |
| Vivado | 2022.2 (may be other) | Synthesize RTL design and generate bitstream |
| [riscv-tools](https://github.com/riscv-software-src/riscv-tools) | - | Synthesize RTL design |
| [riscv64-unknown-elf-gcc](https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.08/riscv64-unknown-elf-gcc-10.1.0-2020.08.2-x86_64-linux-ubuntu14.tar.gz) | 10.1.0 (SiFive GCC) | Build Bao hypervisor and the baremetal guest |
| [riscv64-unknown-linux-gnu-gcc](https://github.com/riscv-collab/riscv-gnu-toolchain/releases/tag/2021.08.11) | 12.2.0 (RISC-V GNU GCC) | Build OpenSBI |

## Setting up the Demo

The following steps will guide you through the steps to configure and run the demo.

First of all, clone this repository and load all git submodules:

```bash
git clone git@github.com:zero-day-labs/riscv-iommu-demo.git

git submodule update --init --recursive
```

### Synthesizing the RTL Design

First, you need to point the `RISCV` environment variable to the path where your [riscv-tools](https://github.com/riscv-software-src/riscv-tools) instalation is located.

```bash
export RISCV=/path/to/riscv_tools
```

Then, to synthesize the RTL design of the CVA6-based SoC with the IOMMU IP and generate the bitstream using Vivado, run from the top directory of the repo:
```bash
make -C cva6 fpga
```

After completing, you can find the bitstream in **cva6/corev_apu/fpga/work-fpga/ariane_xilinx.bit**

### Building the Demo

1. Build the baremetal guest

```bash
make -C bao-baremetal-guest CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=cva6
```

2. In the [config.c](./vm-configs/cva6-baremetal/config.c) file, line 3, setup the **absolute** path to the VM image generated in **bao-baremetal-guest/build/cva6/baremetal.bin**, i.e.:

```
VM_IMAGE(baremetal_image, XSTR(/absolute/path/to/baremetal.bin));
```

3. Copy the VM configuration and the platform configuration to Bao's workspace

```bash
cp -R vm-configs/* bao-hypervisor/configs
cp -R plat-configs/* bao-hypervisor/src/platform/
```

4. Build Bao

```bash
make -C bao-hypervisor CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=cva6 CONFIG=cva6-baremetal CONFIG_BUILTIN=y
```

5. Build OpenSBI and generate firmware payload

```
make -C opensbi CROSS_COMPILE=riscv64-unknown-linux-gnu- PLATFORM=fpga/ariane FW_PAYLOAD=y FW_PAYLOAD_PATH=../bao-hypervisor/bin/cva6/cva6-baremetal/bao.bin
```

### Copying the image to an SD card

1. Insert an SD card into your computer and ***carefully*** identify the dev file associated to your SD Card (e.g., /dev/sda, /dev/mmcblk).
```
sudo fdisk -l
```

2. Format the SD card. Replace *\<dev_file\>* with the dev file you identfied in step 1. 
```
sudo sgdisk --clear --new=1:2048:+4M --new=2 --typecode=1:3000 --typecode=2:8300 <dev_file> -g
```
:information_source: To speed up the loading of the image, the BOOT (first) partition of the SD card is formatted to 4 MiB.

3. Copy the binary file to the SD card. Again, replace *\<dev_file\>* with the dev file you identfied in step 1. 
```
sudo dd if=opensbi/build/platform/fpga/ariane/firmware/fw_payload.bin of=<dev_file> oflag=sync bs=1M
```

## Running the Demo

1. Insert the SD Card with the binary file into the Genesys2 board.
2. Connect two USB cables to the board: one to the port labelled **UART** and another to the one labelled **JTAG**.
3. Power on the board so that the UART interface used (e.g. /dev/ttyUSB0) becomes available.
4. Open a console with 115200-8-N-1 and connect to the created interface. If there are multiple ttyUSB devices just open a console to each of them.
5. Link the Vivado Hardware Manager to the board.
6. Program the device with the generated bitstream (**cva6/corev_apu/fpga/work-fpga/ariane_xilinx.bit**).

Once programming is finished (around 10s), the boot image will be copied from the SD card to the board. Then, the baremetal VM with the demo will start running atop Bao.