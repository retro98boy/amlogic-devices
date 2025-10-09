# 固件

[Armbian](https://github.com/armbian/community)

[Batocera](https://github.com/retro98boy/batocera.linux)

# 硬件

菜鸟物流LEMO CORE A311D版，可拆成主机和底座两个部分

![overview](pictures/overview.jpg)

## 主机

Amlogic A311D SoC，2 GB DDR，16 GB eMMC

无SD卡槽，所以A311D只能从eMMC加载FIP

千兆网口和RTL8822CS WiFi/BT

一个USB Type-C用于在USB下载模式下供电和传输数据

侧边存在金属触点形式的USB接口，和Type-C共用主机的USB 2.0总线，通过SGM7227切换，拉低GPIOA_14即可切换到侧边的USB触点，此时Type-C中的数据通道失效。如果要焊接，推荐先将触点的表面刮成粗糙状便于上锡

侧边还存在6 x WS2812 LED，通过GPIOH_4驱动，驱动见**呼吸灯**章节

![side-connector](pictures/side-connector.jpg)

![four-dots](pictures/four-dots.jpg)

![usb-a-pin](pictures/usb-a-pin.jpg)

| 内部连接器 | 侧边金属触点 |            |
|----------|------------|------------|
| 1        |            | GPIOH_4    |
| 2        | 1          | GND        |
| 3        | 4          | USB 2.0 D- |
| 4        | 3          | USB 2.0 D+ |
| 5        | 2          | USB 5V     |
| 6        |            | LED 5V     |

侧边面板内部构造：

![rm-panel](pictures/rm-panel.jpg)

![rm-panel2](pictures/rm-panel2.jpg)

## 底座

底座有不同型号，但软件应该通用

带USB HUB，显示（HDMI或VGA），供电（USB Type-C或DC）

## 调试点位

调试串口，焊接推荐专用细飞线

![debug-uart](pictures/debug-uart.jpg)

eMMC短接点

![emmc-short](pictures/emmc-short.jpg)

# 主线U-Boot

在[armbian/build](https://github.com/armbian/build)仓库搜索cainiao-cniot-core即可找到添加该设备支持的U-Boot补丁

打上补丁后，使用`make cainiao-cniot-core_defconfig && make CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)`编译得到u-boot.bin

## 制作带主线U-Boot的FIP

使用解包软件（见下文）从设备的USB刷写包中获得厂家的FIP：

```
# burn-img-extract下的DDR.USB即为厂家的FIP
aml_image_v2_packer -d CAINIAO_A311D_RTL8211F_FLINK6222B_2G_原机全分区线刷救砖包仅加当贝桌面.img burn-img-extract
```

也可以尝试在开机的设备上直接使用dd命令提取eMMC上的FIP

一般user area和boot area的FIP相同，二选一即可：

```
# boot area
# 如果是Android，需要改为/dev/block/mmcblk1boot0
dd if=/dev/mmcblk1boot0 of=somewhere/vendor-fip bs=512 skip=1

# user area
# 如果是Android，需要改为/dev/block/mmcblk1
dd if=/dev/mmcblk1 of=somewhere/vendor-fip bs=512 skip=1 count=8192
```

然后使用[gxlimg](https://github.com/repk/gxlimg)解包FIP：

```
gxlimg -e vendor-fip fip
```

最后替换厂家的U-Boot为主线U-Boot并重新打包成FIP：

```
rm fip/bl33.enc
# 这一步可能存在Bug，见下文的解决方案
gxlimg -t bl3x -s path-to-u-boot.bin fip/bl33.enc
gxlimg \
-t fip \
--bl2 fip/bl2.sign \
--ddrfw fip/ddr4_1d.fw \
--ddrfw fip/ddr4_2d.fw \
--ddrfw fip/ddr3_1d.fw \
--ddrfw fip/piei.fw \
--ddrfw fip/lpddr4_1d.fw \
--ddrfw fip/lpddr4_2d.fw \
--ddrfw fip/diag_lpddr4.fw \
--ddrfw fip/aml_ddr.fw \
--ddrfw fip/lpddr3_1d.fw \
--bl30 fip/bl30.enc \
--bl31 fip/bl31.enc \
--bl33 fip/bl33.enc \
--rev v3 fip-with-mainline-uboot.bin
```

> [当前版本](https://github.com/repk/gxlimg/tree/a921d8141fb053b882adc87bff0969dac67572b2)的gxlimg对bl3x的处理有Bug，解决该Bug的PR：[amlsblk: Don't assume hashsz is aligned to file block](https://github.com/repk/gxlimg/pull/26) or [fix array out-of-bounds access in bl3x signature code](https://github.com/repk/gxlimg/pull/25)，两个PR都能解决该Bug，二选一即可
>
> 也可使用[aml_encrypt_g12b](https://github.com/LibreELEC/amlogic-boot-fip/tree/master/khadas-vim3)来替代gxlimg处理u-boot.bin：aml_encrypt_g12b --bl3sig --input u-boot.bin --output fip/bl33.enc --level v3 --type bl33
>
> 一些和gxlimg类似的仓库：[angerman/meson64-tools](https://github.com/angerman/meson64-tools) [afaerber/meson-tools](https://github.com/afaerber/meson-tools)

# 主线内核

在[armbian/build](https://github.com/armbian/build)仓库搜索cainiao-cniot-core即可找到该设备的主线内核dts

## 外设工作情况

| Component             | Status                     |
|-----------------------|----------------------------|
| GBE                   | Working                    |
| WiFi                  | Working                    |
| BT                    | Working                    |
| PWM Fan               | Working                    |
| eMMC                  | Working                    |
| USB                   | Working                    |
| HDMI Display          | Working                    |
| HDMI Audio            | Working                    |
| Internal Speaker      | Working                    |
| Power Button          | Working                    |
| ADC Key               | Working                    |
| Breathing Light       | Working                    |
| USB 2.0 Side Contacts | Working                    |

## 音频

A311D的音频配置十分灵活，整个音频路径可见datasheet中的**Audio Path**章节

主线内核使用axg-sound-card驱动，在用户层使用amixer/alsamixer配置好音频通路即可播放音乐

使用`aplay -l`可以看见card有三个device，分别对应FRDDR_A，FRDDR_B，FRDDR_C

```
**** List of PLAYBACK Hardware Devices ****
card 0: cainiaocniotcor [cainiao-cniot-core], device 0: fe.dai-link-0 (*) []
  Subdevices: 0/1
  Subdevice #0: subdevice #0
card 0: cainiaocniotcor [cainiao-cniot-core], device 1: fe.dai-link-1 (*) []
  Subdevices: 0/1
  Subdevice #0: subdevice #0
card 0: cainiaocniotcor [cainiao-cniot-core], device 2: fe.dai-link-2 (*) []
  Subdevices: 1/1
  Subdevice #0: subdevice #0
```

将音频路由恢复到关闭状态：

```
amixer -D hw:cainiaocniotcor cset name='ACODEC Playback Volume' 0
amixer -D hw:cainiaocniotcor cset name='ACODEC Left DAC Sel' 'Left'
amixer -D hw:cainiaocniotcor cset name='ACODEC Mute Ramp Switch' off
amixer -D hw:cainiaocniotcor cset name='ACODEC Playback Channel Mode' 'Stereo'
amixer -D hw:cainiaocniotcor cset name='ACODEC Ramp Rate' 'Fast'
amixer -D hw:cainiaocniotcor cset name='ACODEC Right DAC Sel' 'Right'
amixer -D hw:cainiaocniotcor cset name='ACODEC Unmute Ramp Switch' off
amixer -D hw:cainiaocniotcor cset name='ACODEC Volume Ramp Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SINK 1 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SINK 2 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SINK 3 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SRC 1 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SRC 2 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SRC 3 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SINK 1 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SINK 2 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SINK 3 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SRC 1 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SRC 2 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SRC 3 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_C SINK 1 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_C SINK 2 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_C SINK 3 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_C SRC 1 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_C SRC 2 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='FRDDR_C SRC 3 EN Switch' off
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A Gain Enable Switch' off
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A Lane 0 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A Lane 1 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A Lane 2 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A Lane 3 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A SRC SEL' 'IN 0'
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B Gain Enable Switch' off
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B Lane 0 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B Lane 1 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B Lane 2 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B Lane 3 Volume' 0
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B SRC SEL' 'IN 0'
amixer -D hw:cainiaocniotcor cset name='TOACODEC Lane Select' 0
amixer -D hw:cainiaocniotcor cset name='TOACODEC OUT EN Switch' off
amixer -D hw:cainiaocniotcor cset name='TOACODEC SRC' 'I2S A'
amixer -D hw:cainiaocniotcor cset name='TOHDMITX Switch' off
amixer -D hw:cainiaocniotcor cset name='TOHDMITX I2S SRC' 'I2S A'
amixer -D hw:cainiaocniotcor cset name='TOHDMITX SPDIF SRC' 'SPDIF A'
```

设置FRDDR_A到HDMI的音频路由：

```
# FRDDR_A -> TDMOUT_A -> TOHDMITX -> HDMI
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SINK 1 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SRC 1 EN Switch' on
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A SRC SEL' 'IN 0'
amixer -D hw:cainiaocniotcor cset name='TOHDMITX Switch' on
amixer -D hw:cainiaocniotcor cset name='TOHDMITX I2S SRC' 'I2S A'
```

此时可以通过`aplay -D plughw:cainiaocniotcor,0 /usr/share/sounds/alsa/Front_Center.wav`向HDMI播放声音

设置FRDDR_B到内置扬声器的音频路由：

```
# FRDDR_B -> TDMOUT_B -> TOACODEC -> ACODEC -> Internal Speaker
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SINK 1 SEL' 'OUT 1'
amixer -D hw:cainiaocniotcor cset name='FRDDR_B SRC 1 EN Switch' on
amixer -D hw:cainiaocniotcor cset name='TDMOUT_B SRC SEL' 'IN 1'
amixer -D hw:cainiaocniotcor cset name='TOACODEC Lane Select' 0
amixer -D hw:cainiaocniotcor cset name='TOACODEC OUT EN Switch' on
amixer -D hw:cainiaocniotcor cset name='TOACODEC SRC' 'I2S B'
amixer -D hw:cainiaocniotcor cset name='ACODEC Playback Volume' 255
```

此时可以通过`aplay -D plughw:cainiaocniotcor,1 /usr/share/sounds/alsa/Front_Center.wav`向内置扬声器播放声音，且与上面的HDMI音频通路互不影响，可以同时播放

也可以设置FRDDR_A到HDMI和内置扬声器的音频路由：

```
# FRDDR_A -> TDMOUT_A -> TOHDMITX and TOACODEC -> HDMI and Internal Speaker
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SINK 1 SEL' 'OUT 0'
amixer -D hw:cainiaocniotcor cset name='FRDDR_A SRC 1 EN Switch' on
amixer -D hw:cainiaocniotcor cset name='TDMOUT_A SRC SEL' 'IN 0'
amixer -D hw:cainiaocniotcor cset name='TOHDMITX Switch' on
amixer -D hw:cainiaocniotcor cset name='TOHDMITX I2S SRC' 'I2S A'
amixer -D hw:cainiaocniotcor cset name='TOACODEC Lane Select' 0
amixer -D hw:cainiaocniotcor cset name='TOACODEC OUT EN Switch' on
amixer -D hw:cainiaocniotcor cset name='TOACODEC SRC' 'I2S A'
amixer -D hw:cainiaocniotcor cset name='ACODEC Playback Volume' 255
```

这样执行`aplay -D plughw:cainiaocniotcor,0 /usr/share/sounds/alsa/Front_Center.wav`，HDMI和内置扬声器会同时播放声音

如果安装了桌面环境，会发现在系统设置里面往往只有一个模拟输出设备。因为该设备只有一张card，PipeWire/Pulse Audio并不会识别card的device。这时可以通过ALSA UCM来管理声音实例和音频路由

首先通过`sudo apt install alsa-ucm-conf`安装ALSA UCM的通用配置。在`/usr/share/alsa/ucm2/ucm.conf`中存在：

```
# The probed path when hw-card is found:
#
#   ucm2/conf.d/[${CardDriver}|${KernelDriver}]/${CardLongName}.conf
#   ucm2/conf.d/[${CardDriver}|${KernelDriver}]/[${CardDriver}|${KernelDriver}].conf
#   ucm2/${KernelModule}/${KernelModule}.conf (obsolete)
#   ucm2/${CardDriver}/${CardLongName}.conf (obsolete)
#   ucm2/${CardDriver}/${CardDriver}.conf (obsolete)
```

通过`alsactl info`和`aplay -l`可以得到card的驱动名和声卡名分别为`axg-sound-card`和`cainiao-cniot-core`，结合上面ucm.conf中的注释，可以将ALSA UCM配置文件放在`/usr/share/alsa/ucm2/conf.d/axg-sound-card/cainiao-cniot-core.conf`

[这里](https://github.com/armbian/build/tree/main/packages/bsp/cainiao-cniot-core)存在写好的ALSA UCM文件，将其放在`/usr/share/alsa/ucm2/Amlogic/axg-sound-card`目录下，并使用`ls -sfv /usr/share/alsa/ucm2/Amlogic/axg-sound-card/cainiao-cniot-core.conf /usr/share/alsa/ucm2/conf.d/axg-sound-card/cainiao-cniot-core.conf`创建符号连接即可完成配置文件的安装

安装ALSA UCM配置文件后，在桌面环境的设置App里面便可以看到HDMI和Internal Speaker两个设备，可以随意切换，且可以同时使用

如果要在CLI下使用ALSA UCM打开音频通路，可执行`alsactl init && alsaucm set _verb "HiFi" set _enadev "HDMI" set _enadev "Speaker"`

## 呼吸灯

主线内核将GPIOH_4复用成SPI MOSI，然后在user space通过操作spidev来控制呼吸灯，简单的demo在本仓库的breathing-light目录下，使用方法：

```
gcc -o ws2812 ws2812.c -lm && ./ws2812
```

可以在PC上交叉编译：

```
aarch64-linux-gnu-gcc --static -o ws2812-static-arm64 ws2812.c -lm
```

## NPU

参考[此处](https://github.com/armbian/build/pull/8689)驱动并测试NPU

# 安装系统

Amlogic USB Burning Tool使用专有的固件格式，虽然固件扩展名为.img，但它并不是disk image文件

存在一些可以对其解包/打包的软件：

[Amlogic binary only executable](https://github.com/khadas/utils/blob/master/aml_image_v2_packer)

[hzyitc/AmlImg](https://github.com/hzyitc/AmlImg)

[7Ji/ampack](https://github.com/7Ji/ampack)

将该设备的所有img系统镜像都重打包成USB刷写包并不划算

可以将带主线U-Boot的FIP打包成USB刷写包，用于将设备恢复到有主线U-Boot的状态，主线U-Boot中带有以太网，USB，eMMC的驱动，这足够做很多的事了

## 写入主线U-Boot

仓库的Releases界面有制作好的FIP，.bin文件用于直接刻录，.burn.img用于Amlogic USB Burning Tool线刷。该U-Boot使能了主机侧边的USB触点，可以在侧边USB触点接上U盘启动系统

### 通过dd命令

如果设备上存在工作的系统例如Android或者Linux，且具有root用户权限，只需要将fip-with-mainline-uboot.bin写入eMMC即可：

```
dd if=path-to-fip-with-mainline-uboot.bin of=/dev/mmcblk1 bs=512 seek=1
```

同时推荐将FIP刻录到eMMC的boot area，因为A311D在eMMC用户区找不到FIP后，会尝试从eMMC的boot area查找FIP

在Linux上，可以：

```
# 检查是否有权限写入boot area
blockdev --getro /dev/mmcblk1boot0
blockdev --getro /dev/mmcblk1boot1

# 解锁boot area的写入权限
echo 0 | sudo tee /sys/block/mmcblk1boot0/force_ro
echo 0 | sudo tee /sys/block/mmcblk1boot1/force_ro

dd if=path-to-fip-with-mainline-uboot.bin of=/dev/mmcblk1boot0 bs=512 seek=1
dd if=path-to-fip-with-mainline-uboot.bin of=/dev/mmcblk1boot1 bs=512 seek=1
```

### 通过USB下载模式

如果无法获得设备上已有系统的root用户权限，可以使用Amlogic USB Burning Tool搭配制作的USB刷写包fip-with-mainline-uboot.burn.img来直接将主线U-Boot刻录到eMMC。USB下载流程见**USB下载模式刻录eMMC**

> Amlogic USB Burning Tool会将FIP同时写入到eMMC的user area和boot area

## 写入系统镜像

### 写入系统镜像到U盘

如果经常尝试各种不同的系统，考虑到eMMC的寿命以及便捷性，你可能会选择从U盘启动系统。只需要将.img文件刻录到U盘，插入设备并开机即可，因为eMMC上的主线U-Boot会自动按照顺序扫描eMMC，USB，网口等外设并找到合适的启动环境，例如U-Boot script，extlinux，TFTP

> 如果A311D还是从eMMC启动，说明eMMC上有可启动的系统。要让A311D从U盘驱动，需要摧毁eMMC上的系统，例如删除eMMC上的MBR分区表。或者在U-Boot命令行界面输入`setenv boot_targets usb0 && boot`，如果你引出了UART线缆

### 写入系统镜像到eMMC

#### 方法一

该方法需要U盘和此设备接入网络

先将Armbian系统镜像写入U盘，使用U盘启动此设备。再将要安装的系统镜像通过网络传输到U盘中，最后使用dd命令将系统刻录到eMMC：

```
dd if=path-to-your-os-img of=/dev/mmcblk1 status=progress
```

或者使用一个小技巧来避免将系统镜像传输到U盘

在U盘启动的Armbian上安装Netcat，然后执行：

```
nc -l -p 1234 > /dev/mmcblk1
```

在Linux PC上安装Netcat，然后执行：

```
nc -w 3 your-cainiao-cniot-core-ip 1234 < path-to-your-os-img
```

> Netcat有GNU和OpenBSD两个变种，如果在PC端安装的是GNU Netcat，则命令应该为：
> ```
> nc -w 3 -c your-cainiao-cniot-core-ip 1234 < path-to-your-os-img
> ```
>
>  安装OpenBSD Netcat的命令：
> ```
> # ArchLinux
> sudo pacman -S openbsd-netcat
> # Ubuntu/Debian
> apt install netcat-openbsd
> ```
>
>  安装GNU Netcat的命令：
> ```
> # ArchLinux
> sudo pacman -S gnu-netcat
> # Ubuntu/Debian
> sudo apt install netcat-traditional
> ```

#### 方法二

该方法需要用户将设备UART连接至PC，且此设备通过以太网络和PC接入同一个LAN

先将要安装的系统镜像放到Linux PC上并重命名，这里假设为batocera.img

然后在Linux PC上将其分割，因为此设备的内存不足以一次性保存整个img：

```
split -b 512M batocera.img batocera.img.
# 假设batocera.img被分割成了batocera.img.aa batocera.img.ab batocera.img.ac，记住它
```

在PC上准备一个TFTP服务器

Windows用户一般使用[Tftpd64](https://pjo2.github.io/tftpd64/)

在Linux上也有类似的软件[puhitaku/tftp-now](https://github.com/puhitaku/tftp-now)，在Releases下载二进制安装到PATH，然后执行`sudo tftp-now serve -root dir-to-batocera.img.aX`即可

在设备上电的时候，通过在UART会话中敲击空格键进入U-Boot命令行界面

设置此设备的网络：

```
setenv ipaddr 172.24.0.241
setenv netmask 255.255.252.0
setenv gatewayip 172.24.0.1
setenv serverip 172.24.0.248
```

上面的172.24.0.248是PC的IP

设置临时存储img的RAM地址：

```
setenv loadaddr 0x20000000
```

开始从PC TFTP服务器下载img并写入到eMMC：

```
setenv mmcscript 'setenv files "batocera.img.aa batocera.img.ab batocera.img.ac"; setenv offset 0x0; for file in ${files}; do tftpboot ${loadaddr} ${file}; setexpr nblk ${filesize} / 0x200; mmc write ${loadaddr} ${offset} ${nblk}; setexpr offset ${offset} + ${nblk}; done'
mmc dev 1 && run mmcscript
```

最后重启此设备即可

# USB下载模式刻录eMMC

主机上面的USB Type-C为USB下载口

短接eMMC短接点再上电可以让设备进入USB下载模式

或者在设备运行时通过命令擦除eMMC user/boot area的头部再重启，也可以让设备进入USB下载模式。参考命令：

```
# under Linux
sudo dd if=/dev/zero of=/dev/mmcblk1 bs=1MiB count=4 status=progress

echo 0 | sudo tee /sys/block/mmcblk1boot0/force_ro
echo 0 | sudo tee /sys/block/mmcblk1boot1/force_ro

sudo dd if=/dev/zero of=/dev/mmcblk1boot0 bs=1MiB count=4 status=progress
sudo dd if=/dev/zero of=/dev/mmcblk1boot1 bs=1MiB count=4 status=progress

# under U-Boot cmd
mmc dev 1
mmc erase 0 8192
mmc partconf 1 1 1 1
mmc erase 0 8192
mmc partconf 1 1 2 2
mmc erase 0 8192
```

最后使用Amlogic USB Burning Tool v2.x.x下载镜像

# 一些链接

[Amlogic SoC Boot Flow](https://docs.u-boot.org/en/v2025.04/board/amlogic/boot-flow.html)

[LibreELEC/amlogic-boot-fip](https://github.com/LibreELEC/amlogic-boot-fip)

[aml_encrypt_g12b functionality](https://github.com/repk/gxlimg/issues/7)

[mmc command](https://docs.u-boot.org/en/stable/usage/cmd/mmc.html)

[U-Boot for Amlogic W400 (S922X)](https://github.com/u-boot/u-boot/blob/master/doc/board/amlogic/w400.rst)
