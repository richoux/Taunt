#!/bin/bash

for i in maps/*1.?.txt
do
		echo "Processing $i..."
		./bin/taunt "$i"
done
