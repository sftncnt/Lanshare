# Lanshare

LanShare is a lightweight LAN file transfer utility written in C.  
It sends a broadcast to peers on the local network using UDP broadcast for discovery
and transfers files using TCP.

## Features

- Zero configuration
- Works across machines on the same LAN
- Written in C using POSIX sockets
- Checksum for detecting corruption

## Build

git clone https://github.com/sftncnt/lanshare

cd lanshare

make

## Usage

Receive files:

./lanshare receive 

Send a file:

./lanshare send <filepath>

Example:

./lanshare send photo.jpg

## How it works

Initiate a listener to receive the file using ./lanshare receive. The listener will create a UDP socket on port 3490 to listen for incoming connections. Running ./lanshare send <filepath> on a separate device or process will initiate a UDP socket that will send a broadcast message to its subnet with it's hostname and filename. A listener will automatically receive this broadcast and know how to unpack the message and display it to the user. After accepting the connection, a TCP socket will be created on the receiver end which the sender will connect to for the actual file transfer. During the TCP transmission, an initial header packet is sent with metadata such as filename and filesize for the receiver to be able to save the file with the same name and continue recv'ng in chunks until amount transferred is equal to the filesize. During transfer, a hash for the file is calculated on both ends. Once the sender finished sending all the chunks, the calculated hash on their end is send to the receiver. The receiver compares this hash with the one it calculated during transfer and deletes the file if integrity check fails. Once the file has been saved and check succeeds, the TCP connection is terminated and the proccess exits on both ends. 

## For Windows

This project is entirely implemented in POSIX sockets. I plan to add Wndows compatibility in the future but until then, cygwin can run this project pretty much the same. Just make sure to install gcc-core, make, libssl-devel, and cygwin-devel when downloading cygwin as these did not install automatically for me and the project won't work without them

## Future improvements

- Add windows support
- Automatic Peer discovery + listing of peers available in the network
