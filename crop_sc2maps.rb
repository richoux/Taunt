#!/usr/bin/ruby

def usage
  puts  "Usage: " + $0 + " SC2map.txt"
end

# We must have at least a file name
if ARGV.length == 0
  usage
  exit
end

file = File.open(ARGV[0])

@input_matrix = []

first_row = 0
last_row = 0
first_column = 1000
last_column = 0
counter = 0

# For each line in file
file.each do |line|
  if line.include?('0') or line.include?('2')
    if first_row == 0
      first_row = counter
    end
    if last_row < counter
      last_row = counter
    end
    if line.include?('0')
      if first_column > line.index('0')
        first_column = line.index('0')
      end
      if last_column < line.rindex('0')
        last_column = line.rindex('0')
      end
    else
      if first_column > line.index('2')
        first_column = line.index('2')
      end
      if last_column < line.rindex('2')
        last_column = line.rindex('2')
      end
    end
  end
  
  counter = counter + 1
  @input_matrix << line.chomp.split('')
end

for x in (first_row-3)..(last_row+3) do
  for y in (first_column-3)..(last_column+3) do
    print "#{@input_matrix[x][y]}"
  end
  puts ""
end

exit
