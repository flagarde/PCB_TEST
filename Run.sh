#!/bin/bash
shopt -s nullglob
for f in /data/NAS/RPCH4/*.dat
do
    echo "Runing - $f"
    bin/Read $f
done
