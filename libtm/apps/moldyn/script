#!/bin/bash

#------------------Set configurations for the scripts----------------#

GP_var=0
CD_CR=0

#MCode:          [ Make command for the Code ] 
#TMP:            [ Path for temp files ]
#OUTPUT:         [ file for output ]
#Action:         [ gprof | CD/CR | ExecTime | TM_Stat ]
#Version:        [ Seq | LibTM | NotTM

TM_Path="/nfs/ug/homes-2/r/rodri357/RESEARCH_2013/Repos/libtm"

MCODE="make -C $TM_Path moldyn"
GP="make -C $TM_Path moldyngp"
TMP="/nfs/ug/homes-2/r/rodri357/RESEARCH_2013/Repos/libtm/apps/moldyn/TMP"
OUTPUT="/nfs/ug/homes-2/r/rodri357/RESEARCH_2013/Repos/libtm/apps/moldyn/OUTPUT"
Action="gprof CD/CR ExecTime"
Version="LibTM"   
   
#------------------ END configurations for the scripts----------------#

#Compile the code
#eval "$MCODE"

#------------------------ Set Action ---------------------------------#

if [[ "$(echo "$Action" | grep -c gprof)" -eq "1" ]]
then
   eval "$GP"
   GP_var=1
   echo "yes2"
else
   eval "$MCODE"
   GP_var=0
fi

if [[ "$(echo "$Action" | grep -c CD/CR)" -eq "1" ]]
then
   CD_CR=1
   ./CD_CR.sh $TM_Path $GP_var 
   echo "yes"
fi

if [[ "$(echo "$Action" | grep -c ExecTime)" -eq "1" ]]
then
   Time.sh $TM_Path $CD_CR
   #./CD_CR.sh $TM_Path $GP
   # Compile with gprof
fi

if [[ "$(echo "$Action" | grep -c TM_Stat)" -eq "1" ]]
then
   # Compile with gprof
   echo "yes4"
fi

#------------------------ Set Version ---------------------------------#
if [[ "$(echo "$Version" | grep -c Seq)" -eq "1" ]]
then
   # insert loop script
   echo "yes5"
fi

if [[ "$(echo "$Version" | grep -c LibTM)" -eq "1" ]]
then
   # Compile with gprof
   echo "yes6"
fi

if [[ "$(echo "$Version" | grep -c NotTM)" -eq "1" ]]
then
   # Compile with gprof
   echo "yes7"
fi
