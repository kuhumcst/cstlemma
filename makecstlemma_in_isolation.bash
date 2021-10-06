METH=git
METH=https

if [ ! -d lib ]; then
    mkdir lib
fi

if [ ! -d lib/hashmap ]; then
    mkdir lib/hashmap
    cd lib/hashmap
    git init
    git remote add origin $METH://github.com/kuhumcst/hashmap.git
    cd ../..
fi
cd lib/hashmap
git pull origin master
cd ../..

if [ ! -d lib/letterfunc ]; then
    mkdir lib/letterfunc
    cd lib/letterfunc
    git init
    git remote add origin $METH://github.com/kuhumcst/letterfunc.git
    cd ../..
fi
cd lib/letterfunc
git pull origin master
cd ../..

if [ ! -d lib/parsesgml ]; then
    mkdir lib/parsesgml
    cd lib/parsesgml
    git init
    git remote add origin $METH://github.com/kuhumcst/parsesgml.git
    cd ../..
fi
cd lib/parsesgml
git pull origin master
cd ../..


# make the default compile target (dynamic linking)
cd src
make
cd ..
