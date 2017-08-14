# bfc4ntk

bfc4ntk is a Ntk firmware packer/unpacker based on BCL (Basic compression library)

## How to use
#### Syntax:

``bfc4ntk.exe <mode> <infile> <outfile> <optional>``


#### Modes:

##### c: Compress file (FullComp)
       * -c <infile> <outfile>
##### p: Compress file (PartComp):
       * -p <infile> <outfile> [PartComp offset, default: <auto detect>]
##### d: Decompress file (FullComp, 1 partition):
       * -d <infile> <outfile> [FullComp header offset, default: 0x00000000]
##### x: Decompress file (PartComp, 1 partition):
       * -x <infile> <outfile> [PartComp header offset, default: <auto detect>]
       
       
## Compiling

I will soon provide a Makefile, for now simply compile using:

```
gcc *.c bfclib/*.c -o bfc4ntk -O3 -Wall
```


## Authors

* Tobi@s


## License

All rights reserved

## Acknowledgments

* Marcus Geelnard [Basic Compression Library](http://bcl.comli.eu/)
* nutsey [GoPrawn](https://www.goprawn.com/member/3-nutsey)
