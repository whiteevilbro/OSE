#!/bin/bash

ACTUAL_KERNEL_SIZE=$(wc -c < ./$1/os.bin)
echo -e "\t\e[1mKERNEL SIZE: $ACTUAL_KERNEL_SIZE\e[0m"
if [ $ACTUAL_KERNEL_SIZE -ge $(($2*1024)) ]; then
  echo EXPECTED_KERNEL_SIZE: $2 kb;
  echo ACTUAL_KERNEL_SIZE: $(($ACTUAL_KERNEL_SIZE / 1024)) kB;
  exit 127;
else
  echo -e "\t\t\e[1mOK\e[0m";
fi