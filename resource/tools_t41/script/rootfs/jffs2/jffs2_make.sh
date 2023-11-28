#!/bin/sh
mkfs.jffs2 -o system.jffs2 -r system -e 0x10000 -s 0x1000 -n -l -X zlib --pad=0x10000
