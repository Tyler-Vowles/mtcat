# mtcat

## Description
**mtcat** is a impimentation of cat that makes use of two threads. mtcat is intended to be used on a group of large files to make use of multiple threads.

## Building
### requirements
- c99 compiler & *nix libc
- make
- pthread library
for distribution build
- tar

``
make
sudo make install
``

## Usage
### Example
``
mtcat file1 file2 file3    #prints file1 then file2 and finally file3
echo hello | mtcat - file1 #prints standard input then file1
mtcat -u pipe1             #prints pipe1 as it reads
``

## License
mtcat is under the MIT license see LICENSE file for details.