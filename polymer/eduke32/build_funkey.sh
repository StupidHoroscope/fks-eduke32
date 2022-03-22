#!/bin/bash

make veryclean
make

rm -rf opk/
mkdir opk/

cp ./rsrc/{eduke32.png,default.funkey-s.desktop} ./opk/
cp ./eduke32 ./opk/

rm -f *.opk
mksquashfs opk/* eduke32_funkey-s.opk
