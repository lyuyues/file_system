#!/bin/bash
make clean
echo "GOGOGO!\n"
make test02
./apps/test02
make test01
./apps/test01
echo "Test all done!\n"
