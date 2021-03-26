# mtcat

## Description
mtcat is a impimentation of cat that makes use of two threads. mtcat is
intended to be used on a group of large files, Althought it uses the same
syntax as cat its overhead for a single file makes it unfit to replace cat.

## Building
### Requirements
- c99 compiler & *nix libc
- make
- pthread library

for other make commands
- m4
- tar
- cp, rm, mv, chmod

### Make Build
```
$ make
# make install
```

## Usage
mtcat [-u] [*file*...]
see the manual for more details
### Examples
```
$ mtcat file1 file2          #prints file1 then file2
$ echo hello | mtcat file -  #prints file then stdin (hello)
$ mtcat -u pipe              #prints pipe as its read
```

## Bugs
```
$ mtcat pipe file1 pipe file2
```
this should print all of pipe then file1 and file2 but some of pipe will be
printed after file1.

## License
mtcat is under the MIT license see LICENSE file for details.