# RISC-V IOMMU Demo

## License

This work is licensed under the Apache-2.0 License. See the [LICENSE](./LICENSE) file for more details.

## Table of Contents

- [About the Demo](#about-the-demo)
- [Tools and Versions](#tools-and-versions)
- [Synthesizing the RTL Design](#synthesizing-the-rtl-design)
- [Building the Demos](#building-the-demos)
    - [Demo #1: Linux w/ IOMMU Driver](#demo-1-linux-w-iommu-driver)
    - [Demo #2: Bao Hypervisor and Guest Attacker](#demo-2-bao-hypervisor-and-guest-attacker)
- [Copying the image to an SD card](#copying-the-image-to-an-sd-card)
- [Running the Demo](#running-the-demo)

***

## About the Demo

This repository contains source code and instructions to build and run two functional demos of the [RISC-V IOMMU IP](https://github.com/zero-day-labs/riscv-iommu) developed by the Zero-Day Labs team. Both demos are carried out in a Genesys2 FPGA board, and run atop a CVA6-based SoC with support for the RISC-V Hypervisor extension, integrating one [DMA device](https://github.com/pulp-platform/iDMA) and the RISC-V IOMMU IP. 

- **Demo #1** consists of a tutorial to run Linux atop the referred platform, using the RISC-V IOMMU Linux driver developed by the RISC-V IOMMU task group. We provide a simple application to issue DMA transfers using both mapped and unmapped addresses.

- In **Demo #2**, we show the operation of the IOMMU within a virtualized environment in a more interactive way. In this demo, the DMA device is programmed to tamper critical memory regions of one of two targets: [Bao hypervisor](https://github.com/bao-project/bao-hypervisor) or [OpenSBI](https://github.com/riscv-software-src/opensbi) (firmware). The IOMMU IP is enabled by default, but it can be disabled to let the user see the consequences. The user navigates through the demo using the push buttons in the board, and the output is printed to the console.

## Tools and Versions

To build and run the demo we used the following tools:

| Requirement | Version | Used to |
| - | - | - |
| Vivado | 2022.2 | Synthesize RTL design and generate bitstream |
| [riscv-tools](https://github.com/riscv-software-src/riscv-tools) | - | Synthesize RTL design |
| [riscv64-linux-gnu-gcc](https://packages.ubuntu.com/search?keywords=gcc-riscv64-linux-gnu) | 9.4.0 (RISC-V GNU GCC - Ubuntu pkg) | Build Linux |
| [riscv64-unknown-elf-gcc](https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.08/riscv64-unknown-elf-gcc-10.1.0-2020.08.2-x86_64-linux-ubuntu14.tar.gz) | 10.1.0 (SiFive GCC) | Build Bao hypervisor and baremetal guest |
| [riscv64-unknown-linux-gnu-gcc](https://github.com/riscv-collab/riscv-gnu-toolchain/releases/tag/2021.08.11) | 12.2.0 (RISC-V GNU GCC) | Build OpenSBI |

## Synthesizing the RTL Design

First of all, clone this repository and load all git submodules:

```bash
git clone git@github.com:zero-day-labs/riscv-iommu-demo.git
cd riscv-iommu-demo
git submodule update --init --recursive
```

As both demos use the same hardware design, the first step is to synthesize the RTL code using Vivado. For this purpose, you need to point the `RISCV` environment variable to the path where your [riscv-tools](https://github.com/riscv-software-src/riscv-tools) instalation is located.

```bash
export RISCV=/path/to/riscv_tools
```

Then, to synthesize the RTL design of the CVA6-based SoC with the IOMMU IP and generate the bitstream, run from the top directory of the repo:
```bash
make -C cva6 fpga
```

After completing, you can find the bitstream in **cva6/corev_apu/fpga/work-fpga/ariane_xilinx.bit**

## Building the demos

### Demo #1: Linux w/ RISC-V IOMMU

1. Build the Linux kernel with the RISC-V IOMMU driver.

```bash
make -C linux ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- KBUILD_DEFCONFIG=defconfig O=build -j$(nproc) defconfig Image
```
:information_source: We provide a pre-built filesystem generated with Buildroot in [linux/fs/cva6/](./linux/fs/cva6/). This FS is passed to the Kernel in the build configuration files, and contains a user-space application to perform DMA transfers within Linux

2. Compile the device tree

```bash
cd linux/arch/riscv/boot/dts/cva6 &&
dtc cva6-ariane-minimal.dts > cva6-ariane-minimal.dtb
```

3. Concatenate the DTB and the Linux image

```bash
make -C linux/tools/lloader CROSS_COMPILE=riscv64-unknown-elf- ARCH=rv64 IMAGE=../../build/arch/riscv/boot/Image DTB=../../arch/riscv/boot/dts/cva6/cva6-ariane-minimal.dtb TARGET=linux-rv64-cva6
```

4. Build OpenSBI and generate firmware payload

```bash
make -C opensbi CROSS_COMPILE=riscv64-unknown-linux-gnu- PLATFORM=fpga/ariane FW_PAYLOAD=y FW_PAYLOAD_PATH=../linux/tools/lloader/linux-rv64-cva6.bin
```

### Demo #2: Bao Hypervisor and Guest Attacker

1. Build the baremetal guest

```bash
make -C bao-baremetal-guest CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=cva6
```

2. In the VM configuration ([config.c](./vm-configs/cva6-baremetal/config.c) file), line 3, set the **absolute** path to the VM image generated in **bao-baremetal-guest/build/cva6/baremetal.bin**, i.e.:

```
VM_IMAGE(baremetal_image, XSTR(/absolute/path/to/baremetal.bin));
```

3. Copy the VM configuration and the platform configuration to the Bao hypervisor directory

```bash
cp -R vm-configs/* bao-hypervisor/configs
cp -R plat-configs/* bao-hypervisor/src/platform/
```

4. Build Bao

```bash
make -C bao-hypervisor CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=cva6 CONFIG=cva6-baremetal CONFIG_BUILTIN=y
```

5. Build OpenSBI and generate firmware payload

```bash
make -C opensbi CROSS_COMPILE=riscv64-unknown-linux-gnu- PLATFORM=fpga/ariane FW_PAYLOAD=y FW_PAYLOAD_PATH=../bao-hypervisor/bin/cva6/cva6-baremetal/bao.bin
```

## Copying the image to an SD card

After building the corresponding demo, you have to copy the output binary file to an SD card in order to run the demo in the *Genesys2* board

1. Insert an SD card into your computer and ***carefully*** identify the dev file associated to your SD Card (e.g., /dev/sda, /dev/mmcblk).
```bash
sudo fdisk -l
```

2. Format the SD card. Replace *\<dev_file\>* with the dev file you identfied in step 1. 
```bash
sudo sgdisk --clear --new=1:2048:+32M --new=2 --typecode=1:3000 --typecode=2:8300 <dev_file> -g
```
:information_source: If you are willing to run **Demo #2**, you can speed up the loading of the image by assigning a lower size to the  the first (BOOT) partition of the SD card (e.g., 4 MiB). To do this, replace **32M** with the size you pretend to use.

3. Copy the binary file to the SD card. Again, replace *\<dev_file\>* with the dev file you identfied in step 1. 
```bash
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