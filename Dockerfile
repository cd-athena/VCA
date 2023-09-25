# Download base image ubuntu 22.04
FROM --platform=amd64 ubuntu:22.04

# LABEL about the custom image
LABEL maintainer="christian.bauer@aau.at"
LABEL version="0.1"
LABEL description="This is a custom Docker Image for VCA."

# Disable Prompt During Packages Installation
ARG DEBIAN_FRONTEND=noninteractive

# Update and Upgrade Ubuntu Software repository
RUN apt-get update && \
    apt-get upgrade -y

# Install required packages
RUN apt install -y git nasm ffmpeg cmake build-essential
RUN rm -rf /var/lib/apt/lists/*
RUN apt clean

WORKDIR /vca

# copy required data to container
COPY source/ source/
COPY CMakeLists.txt .
COPY .clang-format .

# build sources
RUN cmake .
RUN cmake --build . -j

# change to directory containing the VCA binary and copy videos
WORKDIR /vca/source/apps/vca
RUN mkdir videos
COPY videos/* videos/