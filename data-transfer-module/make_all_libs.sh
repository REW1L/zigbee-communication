#!/bin/bash

if [ -z $1 ]; then 
  ROOT=`pwd`
else
  ROOT=$1
fi

mkdir -p $ROOT/libs

cd $ROOT/third-party/

if [ -z "`find . | grep -e 'RF24$'`" ]; then
  `git clone https://github.com/TMRh20/RF24/`;
fi
cd RF24
make all
sudo make install
mv ./*\.so* $ROOT/libs/
cd ..

if [ -z "`find . | grep -e 'RF24Network$'`" ]; then
  `git clone https://github.com/TMRh20/RF24Network/`;
fi
cd RF24Network
make all
sudo make install
mv ./*\.so* $ROOT/libs/
cd ..

if [ -z "`find . | grep -e 'RF24Mesh$'`" ]; then
  `git clone https://github.com/TMRh20/RF24Mesh/`;
fi
cd RF24Mesh
make all
sudo make install
mv ./*\.so* $ROOT/libs/
cd ..

sudo chmod -R 777 $ROOT/libs/
