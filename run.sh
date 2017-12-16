#!/bin/sh
# 
# File:   run.sh
# Author: laxmikant
#
# Created on Oct 28, 2017, 10:30:44 PM
#
cd kernel_module  
make 
sudo make install  
cd ..  
cd library 
make 
sudo make install  
cd .. 
cd benchmark 
make
