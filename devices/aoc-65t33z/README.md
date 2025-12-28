# 固件

[Armbian](https://github.com/retro98boy/armbian-build)

[CoreELEC](https://github.com/retro98boy/CoreELEC)

# 硬件

AOC的电子教学白板主板，原机安卓系统显示型号为65T33Z

Amlogic A311D2主控，作者购买的板子都是T7版本，未见到T7C版本

4G RAM加32G eMMC，同样，作者买到的都是这个存储配置

![top-overview](pictures/top-overview.jpg)

## 供电

![power](pictures/power.jpg)

在板子背面可以看到供电连接器的pin定义。接上12V供电板子就可以开机，但是USB接口无供电，所以还要将AMP_PWR接到5V（推荐购买12V转5V模块）供电

## 调试串口

![debug-uart](pictures/debug-uart.jpg)

波特率为921600

## 核心板

![som](pictures/som.jpg)

![short-point](pictures/short-point.jpg)

短接USB Boot再上电会进入USB下载模式

短接eMMC Short电容两边会让eMMC无法工作。所以先短接，再上电，BootROM会跳过eMMC，直接从其他设备例如SD卡启动bootloader。但是和之前的SoC例如A311D不同，不会fallback到USB下载模式。要使用USB下载，还是得短接上图USB Boot的点。注意，如果没有特殊需求，尽量不要短接eMMC，不确定该操作是否安全。该载板无SD卡槽，所以eMMC Short点位意义不大

# U-Boot

U-Boot源码：[khadas-u-boot](https://github.com/retro98boy/khadas-u-boot/tree/khadas-vims-v2019.01)和[coreelec-u-boot](https://github.com/retro98boy/coreelec-u-boot/tree/khadas-vims-v2019.01)，正如仓库名，分别fork自Khadas和CoreELEC，再开发

khadas-u-boot被[fenix](https://github.com/retro98boy/fenix)使用

coreelec-u-boot被[Armbian](https://github.com/retro98boy/armbian-build)和[CoreELEC](https://github.com/retro98boy/CoreELEC)使用。Armbian官方在一次提交后从Khadas的U-Boot源码改成CoreELEC的，且需要打一些[补丁](https://github.com/retro98boy/armbian-build/tree/main/patch/u-boot/u-boot-meson-s4t7)

## 编译U-Boot

首先下载`riscv-none-embed-gcc-xpack`和`aarch64-linux-gnu`工具链

Khadas的fenix脚本使用v8.3.0-1.2版本的`riscv-none-embed-gcc-xpack`，[下载链接](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases/tag/v8.3.0-1.2)，作者实测xpack-riscv-none-embed-gcc-10.2.0-1.2同样可以兼容

`aarch64-linux-gnu`[下载链接](https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz)

将工具链解压到某个位置并准备U-Boot源码后，开始编译：

```
# 设置环境变量
export PATH=$PATH:/workspace/toolchain/xpack-riscv-none-embed-gcc-8.3.0-1.2/bin
export CROSS_COMPILE=/workspace/toolchain/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

# 编译khadas-u-boot
# 构建BL33和FIP
bash fip/mk_script.sh 65t33z /workspace/u-boot/khadas-u-boot

# 编译coreelec-u-boot
make 65t33z_defconfig
# 编译BL33即U-Boot
make -j$(nproc)
# 构建FIP
bash fip/mk_script.sh 65t33z /workspace/u-boot/coreelec-u-boot
```

之所以coreelec-u-boot要分步编译而khadas-u-boot只需一条命令，是因为该[提交](https://github.com/CoreELEC/u-boot/commit/7f0145c9b759e6dda33e6beb3cdb7bd353cafe14)

两种源码编译后输出都在fip/_tmp目录下，三个文件很重要`u-boot.bin.sd.bin.signed` `u-boot.bin.signed` `u-boot.bin.usb.signed`。个人猜测`u-boot.bin.usb.signed`专用于USB下载模式side load后实现线刷系统到eMMC。`u-boot.bin.sd.bin.signed`被刻录到MMC上，是实际起作用的bootloader。`u-boot.bin.usb.signed`的作用未知

如何将`u-boot.bin.sd.bin.signed`安装到eMMC/SD卡（复制自armbian-build/config/sources/families/meson-s4t7.conf）：

```
## funny enough, the uboot+fip blobs go in the same spot as normal meson64...
function write_uboot_platform() {
        dd if="$1/u-boot.bin.sd.bin.signed" of="$2" conv=fsync,notrunc bs=442 count=1       #> /dev/null 2>&1
        dd if="$1/u-boot.bin.sd.bin.signed" of="$2" conv=fsync,notrunc bs=512 skip=1 seek=1 #> /dev/null 2>&1
}
```

如何将这个三个文件替换到Amlogic Burn Tool线刷包里面完成替换bootloader？

首先解包线刷包，查看image.cfg：

```
[LIST_NORMAL]
file="DDR.USB"		main_type="USB"		sub_type="DDR"	file_type="normal"
file="DDR.USB"		main_type="USB"		sub_type="UBOOT"	file_type="normal"
file="aml_sdc_burn.UBOOT"		main_type="UBOOT"		sub_type="aml_sdc_burn"	file_type="normal"
file="aml_sdc_burn.ini"		main_type="ini"		sub_type="aml_sdc_burn"	file_type="normal"
file="_aml_dtb.PARTITION"		main_type="dtb"		sub_type="meson1"	file_type="normal"
file="platform.conf"		main_type="conf"		sub_type="platform"	file_type="normal"
file="usb_flow.aml"		main_type="aml"		sub_type="usb_flow"	file_type="normal"

[LIST_VERIFY]
file="_aml_dtb.PARTITION"		main_type="PARTITION"		sub_type="_aml_dtb"	file_type="normal"
file="bootloader.PARTITION"		main_type="PARTITION"		sub_type="bootloader"	file_type="normal"
```

再查看fenix编译脚本中的打包conf文件`build/images_upgrade-d0bcc7f98d8294fc9554d07370270610c6732168/Amlogic/package_t7.conf`：

```
#This file define how pack aml_upgrade_package image

[LIST_NORMAL]
#partition images, don't need verfiy
file="u-boot.bin.usb.signed"            main_type= "USB"            sub_type="DDR"
file="u-boot.bin.usb.signed"            main_type= "USB"            sub_type="UBOOT"
file="u-boot.bin.sd.bin.signed"         main_type="UBOOT"           sub_type="aml_sdc_burn"
file="platform.conf"                    main_type= "conf"           sub_type="platform"
file="aml_sdc_burn.ini"                 main_type="ini"             sub_type="aml_sdc_burn"
file="kvim.dtb"                         main_type="dtb"             sub_type="meson1"
file="usb_flow.aml"                     main_type="aml"             sub_type="usb_flow"

[LIST_VERIFY]
#partition images needed verify

#if you want download userdata image, uncomment below line
#file="userdata.img"     main_type="PARTITION"      sub_type="data"
#file="logo.img"                 main_type="PARTITION"    sub_type="logo"
file="rootfs.img"               main_type="PARTITION"    sub_type="rootfs"
file="u-boot.bin.signed"        main_type="PARTITION"    sub_type="bootloader"
file="kvim.dtb"                 main_type="PARTITION"    sub_type="_aml_dtb"
```

对比得出，只要将

u-boot.bin.usb.signed替换DDR.USB

u-boot.bin.signed替换bootloader.PARTITION

u-boot.bin.sd.bin.signed替换aml_sdc_burn.UBOOT

最后重新打包即可

# 内核

暂时只能使用供应商内核，最新版本是5.15。dts在[此处](https://github.com/retro98boy/khadas-common-drivers/blob/khadas-vims-5.15.y/arch/arm64/boot/dts/amlogic/t7-a311d2-65t33z-4g.dts)

## 音频

使用参考此处[此处](../corelab-tvpro/README.md#音频)

该板子和CoreLab TVPro音频方面的区别是少了一路LINE IN，多了一路I2S CODEC输出（其他未贴的电路不考虑）。暂时不驱动I2S CODEC的情况下，完全套用CoreLab TVPro的音频配置，HDMI和LINE OUT输出正常

## GPU

参考[此处](../corelab-tvpro/README.md#gpu)

# 安装系统

该设备无SD卡槽，所以只能将U-Boot安装到eMMC

该设备的USB5744 Hub在U-Boot下工作不正常，所以USB Type-A（由Hub扩展出来的）无法在U-Boot下使用。要从USB启动系统，需要通过Micro USB接口加OTG线材

## 刷入U-Boot

如果板子存在运行的系统且有root权限，直接参考上文的命令将U-Boot dd到eMMC即可

如果板子上运行的系统提权困难，或者不想自己编译U-Boot，直接可以使用[release界面](https://github.com/retro98boy/amlogic-devices/releases/tag/aoc-65t33z)提供的U-Boot线刷包刷入。注意要使用Amlogic Burn Tool v3版本，线刷口为Micro USB。推荐使用OTG线加A to A线（断掉供电线），这样刷完U-Boot后，拔掉A to A线直接插入启动U盘。动手能力强的可以把Micro USB换成Type-A免去OTG线材

注意，要U盘启动Armbian则刷入`aoc-65t33z-4g-u-boot-for-armbian-xxxx.burn.img`，要U盘启动CoreELEC则刷入`aoc-65t33z-4g-u-boot-for-coreelec-xxxx.burn.img`

## 安装系统到U盘

刷好对应的U-Boot到eMMC后，下载对应的系统镜像，解压后将.img刻录到U盘上。最后将U盘插入Micro USB再开机即可

## 安装系统到eMMC

首先刷入`aoc-65t33z-4g-u-boot-for-armbian-xxxx.burn.img`到eMMC，再制作Armbian启动U盘启动系统

这时Armbian相当于一个PE/LiveISO，只需要将想安装的系统.img通过dd或者Netcat刻录到/dev/mmcblkN上即可（因为这些系统.img头部都存在bootloader）

选择Armbain作为PE/LiveISO是因为它更通用且能通过包管理器安装额外工具。用CoreELEC做PE/LiveISO也可以，但是想使用Netcat就无法通过包管理器来安装了


