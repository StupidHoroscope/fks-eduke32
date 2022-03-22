#!/bin/bash

make veryclean
make

rm -rf opk/
mkdir opk/

cp ./{eduke32,eduke32.png,default.funkey-s.desktop} ./opk/

rm -f *.opk
mksquashfs opk/* eduke32_funkey-s.opk
