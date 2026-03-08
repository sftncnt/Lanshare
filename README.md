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

Lanshare has two modes: 
### Send mode:
- Sender sends a UDP broadcast to the subnet with its' hostname and name of the file to share.
- Once it receives an acknowledgement for the broadcast, it connects to the peer that sent the acknowledgement via TCP.
- Once connection is established, a header packet is sent containing file name, file size, and length of file name. This is used to allocate buffers for saving the file
- The file is then sent in chunks to the client, with the hash of the file being computed as it is sent.
- After full file is sent, the calculated hash is also sent and connection is terminated.

### Receive mode:
- Receiver initiates a UDP socket to listen for incoming broadcasts on.
- Once a broadcast is received, the message is unpacked to display the file name and host name of the sender along with a prompt to accept the connection.
- If accepted, an acknowledgement is sent back to the receiver and a TCP socket is initialized on port 3490 which the sender will connect to
- After a connection is established, metadata about the sending file is received and unpacked.
- File is received in chunks with a hash of the file being computed as the chunks come in.
- Once full file is received, a separate hash of the file computed on the sender's end is received
- This hash is compared with the computed hash to check for any corruption. If hashes don't match, file is removed.
- After hash comparison, connection is terminated and process is exited.

## Additional Notes
Make sure both devices are connected to the same private wifi network. This won't work if even one peer's network connection is set to public. Aditionally, if your router is configured to have AP isolation or Client isolation then this project will not work as this setting is a safegaurd against device-to-device traffic. This was enabled on my 5ghz band but connecting to my regular 2.4ghz band network worked.

## For Windows

This project is entirely implemented in POSIX sockets. I plan to add Wndows compatibility in the future but until then, cygwin can run this project pretty much the same. Just make sure to install gcc-core, make, libssl-devel, and cygwin-devel when downloading cygwin as these did not install automatically for me and the project won't work without them

## Future improvements

- Add windows support
- Improved peer discovery + listing of peers available in the network
- Connection interruption handling + Resuming download
