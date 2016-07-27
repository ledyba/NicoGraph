FROM ubuntu:16.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update
RUN apt install -y libboost-all-dev libgoogle-glog-dev build-essential g++
RUN apt install -y libgflags-dev libgoogle-perftools-dev
RUN apt install -y time

ENV HOME /home/nico
RUN useradd --create-home --shell /bin/bash --base-dir /home nico
USER nico
WORKDIR /build

