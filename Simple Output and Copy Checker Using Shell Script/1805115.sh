#!/bin/bash
id=1805121

max_score=$1
max_student_id=$2

#if the command line arguments are absent, then setting the default values
if [ -z "$max_score" ]; then
    max_score=100
fi

if [ -z "$max_student_id" ]; then
    max_student_id=5
fi

echo "Enter the input file name without .zip extension"
read file
unzip $file.zip
cd $file     

#initializing students' marks with max score

declare -a student_marks=()
for((i=0; i<max_student_id; i++))
do 
    student_marks+=("$max_score")
done

mkdir copy
#evaluating student's scripts

for((i=0; i<max_student_id; i++))
do
    if [ -f ./Submissions/$id/$id.sh ]; then
        x=$(bash ./Submissions/$id/$id.sh)
        echo "$x" > $id.txt
        mis_match=$(diff -w $id.txt AcceptedOutput.txt | grep -e '<' -e '>' | wc -l)
        rm $id.txt 
        marks=$(($max_score - $mis_match * 5))
        if((marks < 0)); then
            student_marks[$i]=0
        else
            student_marks[$i]="$marks"
        fi

        #copy the script file in offline folder
        cp ./Submissions/$id/$id.sh ./copy/$id.sh
    else
        student_marks[$i]="0"
    fi
    ((id++))
done


#copy checker
cd copy
id=1805121
x=-1
for((i=0; i<max_student_id; i++))
do
    if [ -f ./$id.sh ]; then
        cid=1805121
        for((j=0; j<max_student_id; j++))
        do
            if [ -f ./$cid.sh ]; then
                if ((id != cid)); then
                    match=$(diff -BZ $id.sh $cid.sh | grep -e '<' -e '>' | wc -l)
                    if((match == 0)); then
                        score=$((0-student_marks[$i]))
                        student_marks[$i]="$score"
                        break
                    fi
                fi
            fi
            ((cid++))
        done
    fi
    ((id++))
done
cd ..
rm -r copy
cd ..
echo "student_id, score" > output.csv

# accessing using indices
id=1805121
for i in ${!student_marks[@]}; 
do
  echo "$id, ${student_marks[$i]}" >> output.csv
  ((id++))
done
