#!/usr/bin/bash

echo "particle cloth (" $1 ") size:" $2 " max_iterations:" $3
echo "iterations	time(microseconds)	ops(megaflops)"

i=10
while [ $i -le $3 ]
do
	../../../bin/particle_cloth $1 $2 $i
	i=$((i+10))
done
