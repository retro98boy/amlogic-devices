# docker buildx build -t retro98boy/xiaoyi-pro-boot:latest .

FROM ubuntu:focal

ARG DEBIAN_FRONTEND=noninteractive
SHELL ["/usr/bin/bash", "-c"]

RUN sed -i 's/archive.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list \
 && apt update \
 && apt upgrade -y \
 && apt install -y git python3 python3-setuptools python3-usb \
 && git clone https://github.com/superna9999/pyamlboot /root/pyamlboot \
 && cd /root/pyamlboot \
 && git reset --hard d2857f4371bcdb7a90d885f24987e03b6d647a8a

COPY ./boot-g12-chainload.py /root/pyamlboot

# 清理
RUN apt autopurge -y && apt autoclean

ENV PATH="${PATH}:/root/pyamlboot"

WORKDIR /data
