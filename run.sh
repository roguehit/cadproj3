#! /bin/bash
rm plot*

for j in 0.1 0.5 0.9 
do
	for i in 100 200 300 400 500 600 700 800 900 998
	do 
		./waf --run "scratch/aodvtest --size=$i --time=15 --intensity=$j --power=100" >> plot100_$j
	done
	echo Running for 100, Intensity $j
done

for j in 0.1 0.5 0.9 
do
	for i in 100 200 300 400 500 600 700 800 900 998
	do 
		./waf --run "scratch/aodvtest --size=$i --time=15 --intensity=$j --power=500" >> plot500_$j
	done
	echo Running for 500, Intensity $j
done

for j in 0.1 0.5 0.9 
do
	for i in 100 200 300 400 500 600 700 800 900 998
	do 
		./waf --run "scratch/aodvtest --size=$i --time=15 --intensity=$j --power=1000" >> plot1000_$j
	done
	echo Running for 1000, Intensity $j
done
