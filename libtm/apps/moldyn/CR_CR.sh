
PROG=moldyn
Code="$1/$PROG"
echo $Code

CD="1 2 3 4"
CR="1 2"
exp_run_t="1 2 3"

Func_Loop()
{
for _CR in $CR
do
     for _CD in $CD
     do

       for _exp_run_t in $exp_run_t
        do
           eval "$Code"
           echo "$_CD $_CR $_exp_run_t"
        done
     done
done
}

Func_Loop_gp()
{
cd $1
for _CR in $CR
do
     for _CD in $CD
     do

       for _exp_run_t in $exp_run_t
        do
           $Code 4 > /dev/null
           #-p prints just the flat file and the grep exclude the header and gprof explanations
           gprof -p "$PROG" | grep -i [0-9]
           #echo "$_CD $_CR $_exp_run_t"
        done
     done
done
  cd -
}

####MAIN ####

if [[ "$2" -eq "1" ]]
then
   echo "Exec with gprof"
   Func_Loop_gp $1
else
   echo "Exec without gprof"
   Func_Loop
fi
