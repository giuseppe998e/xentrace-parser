# XenTrace binary data parser [![](https://img.shields.io/github/v/tag/giuseppe998e/xentrace-parser?style=flat-square)](https://github.com/giuseppe998e/xentrace-parser/tags)
This library parses XenTrace binary files by producing a list of events sorted by their TSC.  
This is part of a project for the final three-year degree exam at the University of Turin.  
Development is supervised by Dario Faggioli ([@dfaggioli](https://github.com/dfaggioli)) and Enrico Bini ([@ebni](https://github.com/ebni)).

## Building
### Testing/Development
```shell
$ git clone git@github.com:giuseppe998e/xentrace-parser.git
$ cd xentrace-parser/
$ make CFLAGS="-Wall -g"
```

### Optimized for usage
```shell
$ git clone git@github.com:giuseppe998e/xentrace-parser.git
$ cd xentrace-parser/
$ make
```

## License
This library is released under the `GNU Lesser General Public License v3 (or later)`.  
This library uses code from [Xen](https://xenbits.xen.org/gitweb/?p=xen.git;a=summary): `trace.h` released under the `MIT License`.