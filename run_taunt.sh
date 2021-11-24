#!/bin/bash

for i in maps/*1.?.txt
do
		filename=$(basename "$i")
		echo "Processing $filename..."
		./bin/taunt "$i"
		name="${filename%.*}"
		info=$(file "maps/${name}.jpg" | awk -F', ' '{print $8}')
		width=$(echo $info | awk -Fx '{print $1}')
		height=$(echo $info | awk -Fx '{print $2}')
		inkscape -D -o "maps/taunted/${name}_contour.png" -w $width -h $height "maps/taunted/${name}_contour.svg"
		convert -composite "maps/${name}.jpg" "maps/taunted/${name}_contour.png" -gravity center "maps/taunted/${name}_contour.png"
done

