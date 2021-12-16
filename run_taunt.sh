#!/bin/bash

for i in maps/*1.?.txt maps/*LE.txt
do
		filename=$(basename "$i")
		echo ""
		echo "Processing $filename..."
		./bin/taunt "$i" 2> /dev/null
		name="${filename%.*}"
		info=$(file "maps/${name}.jpg" | awk -F', ' '{print $(NF-1)}')
		width=$(echo $info | awk -Fx '{print $1}')
		height=$(echo $info | awk -Fx '{print $2}')

		# inkscape -D -o "maps/taunted/${name}_contour.png" -w $width -h $height "maps/taunted/${name}_contour.svg" 2> /dev/null
		# convert -composite "maps/${name}.jpg" "maps/taunted/${name}_contour.png" -gravity center "maps/taunted/${name}_contour.png"

		# inkscape -D -o "maps/taunted/${name}_height.png" -w $width -h $height "maps/taunted/${name}_height.svg" 2> /dev/null
		# convert -composite "maps/${name}.jpg" "maps/taunted/${name}_height.png" -gravity center "maps/taunted/${name}_height.png"

		# inkscape -D -o "maps/taunted/${name}_resources.png" -w $width -h $height "maps/taunted/${name}_resources.svg" 2> /dev/null
		# convert -composite "maps/${name}.jpg" "maps/taunted/${name}_resources.png" -gravity center "maps/taunted/${name}_resources.png"

		# inkscape -D -o "maps/taunted/${name}_regions_small_chokes.png" -w $width -h $height "maps/taunted/${name}_ghosted.svg" 2> /dev/null
		# convert -composite "maps/${name}.jpg" "maps/taunted/${name}_regions_small_chokes.png" -gravity center "maps/taunted/${name}_regions_small_chokes.png"

		inkscape -D -o "maps/taunted/${name}_regions_same_areas.png" -w $width -h $height "maps/taunted/${name}_ghosted.svg" 2> /dev/null
		convert -composite "maps/${name}.jpg" "maps/taunted/${name}_regions_same_areas.png" -gravity center "maps/taunted/${name}_regions_same_areas.png"
done

