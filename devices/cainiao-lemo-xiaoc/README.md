# 固件

[Armbian](https://github.com/retro98boy/armbian-build)

[Batocera](https://github.com/retro98boy/batocera.linux)

**注意**：针对于开启了安全启动的设备，需要额外操作才能引导这些固件

# 硬件

菜鸟物流LEMO小C，Amlogic A311D SoC，4 GB DDR，16 GB eMMC，三个USB 2.0 Type-A，一个USB 3.2 Gen1 Type-A，千兆网口和RTL8821CS WiFi/BT

一个USB Type-C用于在USB下载模式下传输数据

存在调试串口的焊孔，可以焊上2.54排针方便调试

![overview](pictures/overview.jpg)

eMMC短接点

![emmc-short](pictures/emmc-short.jpg)

> 该设备可能存在不同批次，据网友反馈该设备开启了安全启动，但是作者买到的设备未开启安全启动
>
> 如何判断自己的设备是否开启了安全启动请参考[这里](https://github.com/retro98boy/onethingcloud-oes-linux?tab=readme-ov-file#%E5%AE%89%E5%85%A8%E5%90%AF%E5%8A%A8)
>
> 如果自己的设备开启了安全启动，就只能使用厂商的U-Boot来引导系统，且在玩机前一定要备份mmcblkNboot0/1和eMMC全盘

# 主线U-Boot

**注意**：不适用于开启了安全启动的设备

在[retro98boy/armbian-build](https://github.com/retro98boy/armbian-build)仓库搜索cainiao-lemo-xiaoc即可找到添加该设备支持的U-Boot补丁

打上补丁后，使用`make cainiao-lemo-xiaoc_defconfig && make CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)`编译得到u-boot.bin

如何使用u-boot.bin制作FIP参考[此处](../cainiao-cniot-core/README.md)，需要的一些FIP blobs在[此处](https://github.com/retro98boy/amlogic-fip-blobs)下载

本仓库的[Releases界面](https://github.com/retro98boy/amlogic-devices/releases/tag/cainiao-lemo-xiaoc)有制作好的FIP，.bin文件用于直接刻录，.burn.img用于Amlogic USB Burning Tool线刷

# 主线内核

在[retro98boy/armbian-build](https://github.com/retro98boy/armbian-build)仓库搜索cainiao-lemo-xiaoc即可找到该设备的主线内核dts

## 外设工作情况

| Component             | Status                     |
|-----------------------|----------------------------|
| GBE                   | Working                    |
| WiFi                  | Working                    |
| BT                    | Working                    |
| eMMC                  | Working                    |
| USB                   | Working                    |
| HDMI Display          | Working                    |
| HDMI Audio            | Working                    |
| Internal Speaker      | Working                    |
| Power Button          | Working                    |
| ADC Key               | Working                    |

## 音频

和CAINIAO CNIoT-CORE一样，详见[此处](../cainiao-cniot-core/README.md)的**音频**章节。唯一不同的只是CNIoT-CORE为单个HT6872功放带动一个扬声器，LEMO小C两个HT6873带动两个扬声器

# 安装系统

**注意**：不适用于开启了安全启动的设备

参考[此处](../cainiao-cniot-core/README.md)的**安装系统**章节

**注意**：通过U盘启动时，只能使用USB 3.2 Gen1 Type-A和它旁边的两个Type-A，因为靠近HDMI的那个Type-A的电源在主线U-Boot下未使能
