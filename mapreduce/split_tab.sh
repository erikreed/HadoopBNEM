# unused
EXPECTED_ARGS=2
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` filename num_parts"
	exit $E_BADARGS
fi


fspec=$1
num_files=$2

# Work out lines per file.

total_lines=$(cat ${fspec} | wc -l)
((lines_per_file = (total_lines + num_files - 1) / num_files))

# Split the actual file, maintaining lines.

split -d -a 3 --lines=${lines_per_file} ${fspec} $1.part

# Debug information

echo "Total lines     = ${total_lines}"
echo "Lines  per file = ${lines_per_file}"    
