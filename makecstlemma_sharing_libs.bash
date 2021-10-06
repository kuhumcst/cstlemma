#!/bin/bash
# This script is for those who also want to build any of the following kuhumcst tools:
#     affixtrain, all2lower, makeUTF8, taggerXML, repetitiveness-checker, rtfreader
# You can use this script in three ways
# (1) If you do not already have the cstlemma repo:
#     Copy this script to the folder where you want to create cstlemma and run it.
#     The script creates a number of folders and repos in those folders.
# (2) If you have already have cloned the cstlemma repo but not built it:
#     Change directory to the cstlemma folder and run this script.
#     The script creates a few additional folders at the same level as cstlemma.
#     Those folders contain source code that is shared by other kuhumcst repos.
# (3) If you previously have built cstlemma: change directory to the cstlemma folder and run this script

METH=https

if [ -d ../cstlemma ]
then
    # Hmm, we might already be in the cstlemma folder.
    # No problem.
    cd ..
elif [ ! -d cstlemma ]
then
    # We do not seem to have the cstlemma folder
    # We make it here.
    mkdir cstlemma
    cd cstlemma
    git init
    git remote add origin $METH://github.com/kuhumcst/cstlemma.git
    cd ..
fi

if [ ! -d hashmap ]; then
    mkdir hashmap
    cd hashmap
    git init
    git remote add origin $METH://github.com/kuhumcst/hashmap.git
    cd ..
fi
cd hashmap
git pull origin master
cd ..

if [ ! -d letterfunc ]; then
    mkdir letterfunc
    cd letterfunc
    git init
    git remote add origin $METH://github.com/kuhumcst/letterfunc.git
    cd ..
fi
cd letterfunc
git pull origin master
cd ..

if [ ! -d parsesgml ]; then
    mkdir parsesgml
    cd parsesgml
    git init
    git remote add origin $METH://github.com/kuhumcst/parsesgml.git
    cd ..
fi
cd parsesgml
git pull origin master
cd ..

cd cstlemma
git pull origin master
# make the default target
cd src
make
cd ..
