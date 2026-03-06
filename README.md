# LanShare

LanShare is a lightweight LAN file transfer utility written in C.  
It sends a broadcast to peers on the local network using UDP broadcast for discovery
and transfers files using TCP.

## Features

- Zero configuration
- Works across machines on the same LAN
- Written in C using POSIX sockets

## Build

git clone https://github.com/<user>/lanshare
cd lanshare
make

## Usage

Receive files:

./lanshare receive <directory>

Send a file:

./lanshare send <file>

Example:

./lanshare send photo.jpg
