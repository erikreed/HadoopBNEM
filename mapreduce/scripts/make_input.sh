if test "$1" == "" ; then                                                                     
        echo Usage: $0 dat_directory                                        
        exit 1                                                                                
fi  

mkdir -p in
rm -f in/*

head -2 $1/tab > in/tab_header

total_lines=$(cat $1/tab | wc -l)
((total_lines=total_lines - 2))

tail -$total_lines $1/tab > in/tab_content
cp $1/* in


#./split_tab.sh in/tab_content 10
#cd in;
#ls tab_content.part* > in.txt
#cd ..;

