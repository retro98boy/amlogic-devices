#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define NUM_LEDS 6
#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_SPEED_HZ 2400000 // 2.4MHz，接近WS2812的800kHz（每个bit 3字节模拟）
#define BYTES_PER_LED 24     // 每个LED 24位（8位R、8位G、8位B）
#define RESET_PULSE_US 60    // WS2812复位脉冲，>50µs

// 颜色结构体
typedef struct {
  uint8_t r, g, b;
} Color;

// SPI句柄
int spi_fd;

// 初始化SPI
int init_spi() {
  spi_fd = open(SPI_DEVICE, O_RDWR);
  if (spi_fd < 0) {
    perror("Failed to open SPI device");
    return -1;
  }

  uint32_t speed = SPI_SPEED_HZ;
  uint8_t mode = SPI_MODE_0;
  uint8_t bits = 8;

  if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0 ||
      ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
      ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
    perror("Failed to set SPI settings");
    close(spi_fd);
    return -1;
  }
  return 0;
}

// 将逻辑1/0转换为WS2812的SPI字节序列（3字节模拟1位）
void bit_to_spi_bytes(uint8_t bit, uint8_t *bytes) {
  if (bit) {
    // 逻辑1: T1H=0.85µs, T1L=0.4µs，SPI 2.4MHz下约为2周期高，1周期低
    bytes[0] = 0xF0; // 11110000
    bytes[1] = 0x00; // 00000000
    bytes[2] = 0x00; // 00000000
  } else {
    // 逻辑0: T0H=0.4µs, T0L=0.85µs，SPI 2.4MHz下约为1周期高，2周期低
    bytes[0] = 0x80; // 10000000
    bytes[1] = 0x00; // 00000000
    bytes[2] = 0x00; // 00000000
  }
}

// 将颜色转换为SPI字节流
void color_to_spi_bytes(Color color, uint8_t *buffer) {
  int idx = 0;
  // WS2812顺序：G -> R -> B
  uint8_t colors[3] = {color.g, color.r, color.b};
  for (int i = 0; i < 3; i++) {
    for (int j = 7; j >= 0; j--) {
      uint8_t bit = (colors[i] >> j) & 1;
      bit_to_spi_bytes(bit, &buffer[idx]);
      idx += 3;
    }
  }
}

// 计算颜色渐变（线性插值）
Color interpolate_color(float t, Color start, Color end) {
  Color result;
  result.r = (uint8_t)(start.r + (end.r - start.r) * t);
  result.g = (uint8_t)(start.g + (end.g - start.g) * t);
  result.b = (uint8_t)(start.b + (end.b - start.b) * t);
  return result;
}

// 发送数据到WS2812
void send_led_data(Color *colors, int num_leds) {
  uint8_t *buffer =
      malloc(num_leds * BYTES_PER_LED * 3); // 每个LED 24位，每位3字节
  if (!buffer) {
    perror("Failed to allocate buffer");
    return;
  }

  // 填充缓冲区
  for (int i = 0; i < num_leds; i++) {
    color_to_spi_bytes(colors[i], &buffer[i * BYTES_PER_LED * 3]);
  }

  // 发送数据
  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)buffer,
      .len = num_leds * BYTES_PER_LED * 3,
      .speed_hz = SPI_SPEED_HZ,
      .bits_per_word = 8,
  };

  if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
    perror("Failed to send SPI data");
  }

  free(buffer);
  usleep(RESET_PULSE_US); // 复位脉冲
}

int main() {
  if (init_spi() < 0) {
    return -1;
  }

  Color start_color = {0xff, 0, 0}; // 纯红色
  Color end_color = {0, 0, 0xff};   // 纯蓝色
  Color leds[NUM_LEDS];
  float phase_offset = 2.0 * M_PI / NUM_LEDS;
  float cycle_duration = 0.01; // 0.5秒

  while (1) {
    float t = (float)(clock() % (CLOCKS_PER_SEC * (long)cycle_duration)) /
              (CLOCKS_PER_SEC * cycle_duration);
    for (int i = 0; i < NUM_LEDS; i++) {
      float phase = t * 2.0 * M_PI + i * phase_offset;
      float t_offset = (sinf(phase) + 1.0) / 2.0;
      leds[i] = interpolate_color(t_offset, start_color, end_color);
    }
    send_led_data(leds, NUM_LEDS);
    usleep(10 * 1000); // 10ms刷新，约100Hz
  }

  close(spi_fd);
  return 0;
}
