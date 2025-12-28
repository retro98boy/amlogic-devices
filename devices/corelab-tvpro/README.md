# 固件

[Armbian](https://github.com/retro98boy/armbian-build)

[CoreELEC](https://github.com/retro98boy/CoreELEC)

# 硬件

喽喽大佬逆向郎国A311D2核心板PIN定义后，自己画的载板，并提供基于Khadas VIM4源码移植的安卓系统

核心板存在T7 4G、T7 8G、T7C 4G、T7C 8G四种，作者主要适配T7 8G的系统

[anshi233](https://github.com/anshi233)大佬完成了HDMI RX tvserver Linux移植，并准备基于5.15 BSP内核开发硬件加速的ffmpeg，旨在将其打造成直播采集推流一体机。大佬的gitee地址：https://gitee.com/anshi233

![overview](pictures/overview.jpg)

## LINE OUT/IN切换

3.5mm接口是LINE OUT还是LINE IN由板子上的跳线决定，如图：

![line-out-in](pictures/line-out-in.jpg)

## 核心板

![som-top](pictures/som-top.jpg)

![som-bottom](pictures/som-bottom.jpg)

![short](pictures/short.jpg)

短接上图绿圈两点再上电会进入USB下载模式

短接上图红圈的电容两边会让eMMC无法工作。所以先短接，再上电，BootROM会跳过eMMC，直接从其他设备例如SD卡启动bootloader。但是和之前的SoC例如A311D不同，不会fallback到USB下载模式。要使用USB下载，还是得短接上图绿圈中的点。注意，如果没有特殊需求，尽量不要短接eMMC，不确定该操作是否安全

蓝圈里面是eMMC cmd上拉电阻

# U-Boot

参考[此处](../aoc-65t33z/README.md#U-Boot)，只需要将65t33z改成tvpro

# 内核

暂时只能使用供应商内核，最新版本是5.15。dts在[此处](https://github.com/retro98boy/khadas-common-drivers/blob/khadas-vims-5.15.y/arch/arm64/boot/dts/amlogic/t7-a311d2-tvpro-8g.dts)

## 音频

个人对5.15供应商内核的音频配置理解在[此处](../../platforms/a311d2/audio/README.md)

对于上文的dts，HDMITX、LINE IN、LINE OUT可以使用

测试HDMITX：

```
amixer -D hw:corelabtvpro cset name='HDMITX Audio Source Select' 'Spdif'
aplay -D plughw:corelabtvpro,0 /usr/share/sounds/alsa/Front_Right.wav
```

测试LINE OUT：

```
amixer -D hw:corelabtvpro cset name='tdmout_c binv set' '0'
# 线性音量调整
amixer -D hw:corelabtvpro cset name='DAC Digital Playback Volume' 255
# 如果最大音量还不够，增益可以调的更大
amixer -D hw:corelabtvpro cset name='DAC Extra Digital Gain' '0dB'
amixer -D hw:corelabtvpro cset name='DAC SOURCE SELECT' 'Lane0'
aplay -D plughw:corelabtvpro,3 /usr/share/sounds/alsa/Front_Right.wav
```

测试LINE IN：

```
amixer -D hw:corelabtvpro cset name='Linein left switch' AIL1
amixer -D hw:corelabtvpro cset name='Linein right switch' AIR1
# 输入增益可调整，最大31
amixer -D hw:corelabtvpro cset name='PGA IN Gain' 31
arecord -D hw:corelabtvpro,3 -c 2 -f S16_LE -r 48000 linein.wav
aplay -D plughw:corelabtvpro,3 linein.wav
```

推荐使用ALSA UCM来配置控件，PipeWire/Pulse Audio会解析配置文件，这样在桌面环境下可以手动选择输出设备和录制设备

[这里](https://github.com/retro98boy/armbian-build/tree/main/packages/bsp/corelab-tvpro)存在对应的ALSA UCM配置文件，下载后执行安装命令：

```
mkdir -p /usr/share/alsa/ucm2/Amlogic/corelab-tvpro
cp corelab-tvpro* /usr/share/alsa/ucm2/Amlogic/corelab-tvpro

mkdir /usr/share/alsa/ucm2/conf.d/corelab-tvpro
ln -sfv /usr/share/alsa/ucm2/Amlogic/corelab-tvpro/corelab-tvpro.conf /usr/share/alsa/ucm2/conf.d/corelab-tvpro/corelab-tvpro.conf
```

安装完成后重启设备即可

不安装桌面环境，直接在CLI下使用ALSA UCM也是可以的，执行`alsactl init && alsaucm set _verb "HiFi" set _enadev "HDMI" set _enadev "LINE OUT"`就会配置对应输出设备的控件

## GPU

供应商内核GPU驱动使用ARM私有blob，对于开源软件的兼容性并不好。好在可以改用Panfrost内核驱动加Mesa库。需要修改dts和内核配置，可以参考[此处](https://github.com/retro98boy/armbian-build/commit/18291b0f467b6a5d967a2b6cacb602c386c1cb52)

# 安装系统

[此处](../aoc-65t33z/README.md#安装系统)的安装思路对该设备完全适用。[release界面](https://github.com/retro98boy/amlogic-devices/releases/tag/corelab-tvpro)同样提供U-Boot线刷包

由于该设备有micro SD卡槽，所以安装系统的方式更多，可以：

- 直接将系统dd到micro SD卡上，插入设备开机使用。注意，要先擦除eMMC上的bootloader

- 直接从micro SD卡启动Armbian当作PE/LiveISO，再将系统.img刻录到eMMC实现安装系统到eMMC
