METH=git
METH=https
BRANCH=$(git branch --show-current)

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

if [ ! -d cstlemma ]; then
    mkdir cstlemma
    cd cstlemma
    git init
    git remote add origin $METH://github.com/kuhumcst/cstlemma.git
    git checkout $BRANCH
    cd ..
fi
cd cstlemma
git pull origin $BRANCH
cd src

# make the default compile target (dynamic linking)
make
cd ..
cd ..
