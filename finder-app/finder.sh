#!/bin/bash

# AESD Assignment 1 
# Author: Gautama Gandhi
# Description: This file is the finder.sh

filesdir=$1
searchstr=$2

#Check number of arguments

if [ $# -ne 2 ]
then
    echo "Incorrect number of parameters. Expected 2!"
    exit 1
fi

#Checking if file directory is valid

if [ ! -d "$filesdir" ]
then
    echo "File directory ${filesdir} not present"
    exit 1
fi

# Variables for file count and matching line count
filecount=0
matching_line_count=0

#Using grep to get file count and matching line count
filecount=$(grep -r -l $searchstr $filesdir | wc -l)
matching_line_count=$(grep -r $searchstr $filesdir | wc -l)

echo "The number of files are ${filecount} and the number of matching lines are ${matching_line_count}"

exit 0
