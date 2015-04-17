#!/bin/sh
doxygen_bin=~/programs/doxygen-1.8.6/bin
ptetex_bin=~/programs/texlive/bin/x86_64-linux
export PATH=$PATH:${doxygen_bin}:${ptetex_bin}

rm -rf latex html

${doxygen_bin}/doxygen $1

if [ -e latex ]; then
  cd latex
  make
  make pdf
  cd ../
fi

cp latex/refman.pdf ../reference.pdf

rm -rf latex html