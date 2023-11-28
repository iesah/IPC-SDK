#!/bin/sh
mkfs.ubifs -e 0x1F000 -c 384 -m 2048 -d root-uclibc-nand-toolchain720 -o root-uclibc-nand-toolchain720.ubifs
sleep 1
ubinize -o root-uclibc-nand-toolchain720.img -m 2048 -p 128KiB -s 2048 ubinize.cfg -v
sleep 1
rm root-uclibc-nand-toolchain720.ubifs
