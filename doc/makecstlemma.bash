METH=git
METH=https

if [ ! -d hashmap ]; then
    mkdir hashmap
    cd hashmap
    git init
    cd ..
fi
cd hashmap
git remote add origin $METH://github.com/kuhumcst/hashmap.git
git pull origin master
cd ..

if [ ! -d letterfunc ]; then
    mkdir letterfunc
    cd letterfunc
    git init
    cd ..
fi
cd letterfunc
git remote add origin $METH://github.com/kuhumcst/letterfunc.git
git pull origin master
cd ..

if [ ! -d parsesgml ]; then
    mkdir parsesgml
    cd parsesgml
    git init
    cd ..
fi
cd parsesgml
git remote add origin $METH://github.com/kuhumcst/parsesgml.git
git pull origin master
cd ..

if [ ! -d cstlemma ]; then
    mkdir cstlemma
    cd cstlemma
    git init
    cd ..
fi
cd cstlemma
git remote add origin $METH://github.com/kuhumcst/cstlemma.git
git pull origin master
cd src
make all
cd ..
cd ..
