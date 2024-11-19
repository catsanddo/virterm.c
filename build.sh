#!/bin/sh

mkdir -p include

cp src/virterm.h include/virterm.h
echo >> include/virterm.h
echo '#ifdef VIRTERM_IMPLEMENTATION' >> include/virterm.h
sed '1d' src/virterm.c >> include/virterm.h
echo '#endif' >> include/virterm.h
