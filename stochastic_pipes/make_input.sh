mkdir -p in
rm in/*

head -2 dat/tab > in/tab_header

total_lines=$(cat dat/tab | wc -l)
((total_lines=total_lines - 2))

tail -$total_lines dat/tab > in/tab_content
cp dat/* in


#./split_tab.sh in/tab_content 10
#cd in;
#ls tab_content.part* > in.txt
#cd ..;

