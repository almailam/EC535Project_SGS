#!/bin/sh

mknod /dev/mygpio c 61 0
insmod mygpio.ko