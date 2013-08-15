#!/bin/bash

PROG=moldyn
Code="$1/$PROG"
echo $Code

CD="1 2 3 4"
CR="1 2"
exp_run_t="1 2 3"

Func_CDCR()
{
cd $1 
for _CR in $CR
do
     for _CD in $CD
     do
       for _exp_run_t in $exp_run_t
        do
          #time $Code 4
          { time $PROG 4; } 2> out.txt
          echo "$(cat out.txt | grep -i real | awk '{print $2}') $_CD $_CR"  >> Timing.txt
        done
     done
done
cd -
}

Func_NO_CDCR()
{
cd $1 

{ time $PROG 4; } 2> out.txt
echo "$(cat out.txt | grep -i real | awk '{print $2}')" >> Timing.txt

cd -
}

####MAIN ####

if [[ "$2" -eq "1" ]]
then
   echo "CD CR"
   Func_CDCR $1
else
   echo "NO"
   Func_NO_CDCR $1
fi
