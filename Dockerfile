# FROM alpine:3.17.0 AS build

# RUN apk update && \
#     apk add --no-cache \
#         build-base=0.5-r3 \
#         cmake=3.24.3-r0 \
#         boost1.80-dev=1.80.0-r3

# WORKDIR /vca

# COPY source/ .source/
# COPY CMakeLists.txt .

# WORKDIR /vca/build

# RUN cmake -DCMAKE_BUILD_TYPE=Release .. && \
#     cmake --build . --parallel 8

# FROM alpine:3.17.0

# RUN apk update && \
#     apk add --no-cache \
#     libstdc++=12.2.1_git20220924-r4 \
#     boost1.80-program_options=1.80.0-r3

# RUN addgroup -S vca_user && adduser -S vca_user -G vca_user
# USER vca_user

# COPY --chown=vca_user:vca_user --from=build ./vca/build/src/apps ./app/

# # ENTRYPOINT [ "./app" ]

# syntax=docker/dockerfile:1
# Download base image ubuntu 22.04
FROM --platform=amd64 ubuntu:22.04

# LABEL about the custom image
LABEL maintainer="christian.bauer@aau.at"
LABEL version="0.1"
LABEL description="This is a custom Docker Image for VCA."

# Disable Prompt During Packages Installation
ARG DEBIAN_FRONTEND=noninteractive

# Update Ubuntu Software repository
RUN apt-get update && \
    apt-get upgrade -y

RUN apt install -y git nasm ffmpeg cmake build-essential
RUN rm -rf /var/lib/apt/lists/*
RUN apt clean

WORKDIR /vca

COPY source/ source/
COPY CMakeLists.txt .
COPY .clang-format .

RUN mkdir build
# RUN cd build
RUN cmake .
RUN cmake --build .

WORKDIR /vca/source/apps/vca
COPY videos/* .