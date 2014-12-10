#!/bin/bash

for i in {64..1}; do
	echo $i
	kill -s $i $1 
done
