#!/bin/bash

export LD_LIBRARY_PATH=/home/dragon/oss/gpgme-1.5.1/src/.libs

ulimit -c unlimited
./test_gpgme
