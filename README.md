# CSTLEMMA - the CST Lemmatiser


**This distribution contains the following directories and files:**

* doc
    * This directory contains documentation of the program.
* src
    * This directory contains source code and a Makefile.
    * You also need the source code in the following repositories, installed alongside and at the same level as cstlemma:
        * https://github.com/kuhumcst/parsesgml
        * https://github.com/kuhumcst/letterfunc
        * https://github.com/kuhumcst/hashmap      
* Changelog
    * A document describing changes between versions.
* COPYING
    * The full text of the GNU public licence.
* README.md
    * This file.



**CSTLEMMA has been compiled and run on the following platforms:**
<table>
<thead><td>
Platform</td><td>Compiler(s)</td></thead>
<tr><td>
Windows</td><td>Borland C++ 5 and Microsoft Visual C++ 6.0 and later</td></tr><tr><td>
Linux</td><td>GNU C++ 3.3.1 and later</td></tr></table>

Both 32 and 64 bit versions can be made.

**Installation**

* Linux:
    1. Download (e.g. git pull) cstlemma, parsesgml, letterfunc and hashmap. If you are going to use the Makefile that comes with cstlemma, locate each of these packages in separate subdirectories under the same directory, and call these subdirectories cstlemma, parsesgml, letterfunc and hashmap, respectively. You can use https://github.com/kuhumcst/cstlemma/blob/master/doc/makecstlemma.bash to do all of this automatically.
    2. Change directory to the 'cstlemma/src' directory.
    3. Run 'make' or 'make cstlemma'. To get rid of object files, run 
    4. 'make clean'.

**Running**

For running the CST lemmatiser you need as a minimum a file containing flex
rules. The absolute minimal set of flex rules is the empty set, in which case
the lemmatiser assumes that all words in your input text are perfectly
lemmatised already.

Thus, for checking that the lemmatiser runs OK, you could do the following:

        touch my_empty_rule_file
        cstlemma -L -t- -f my_empty_rule_file -i my_text_file.txt

This would create a file my_text_file.txt.lemma that has two tab-separated
columns: the left column contains a word from your text and the right column
contains the same word, converted to lower-case. The -L option tells the
program lemmatise (as opposed to generating flex rules or creating a binary
dictionary). The -t- option tells the program not to expect tagged input. The
-f and -i options tells the program which rules and which input text to read.

You can hand-craft the rules or let the lemmatiser generate flex rules from
a full-form dictionary. The full-form dictionary can also be used to generate
a binary dictionary, which the program can use to even better lemmatise your
input text.

If you want to lemmatise a Danish text, please contact us. We have a full form
dictionary with 70000 head words that we have used to train the lemmatiser for
the Danish language.


**Contact info**

For questions and remarks about the program, please feel free to contact us.

Our postal address is:

    Center for Sprogteknologi
    University of Copenhagen
    Njalsgade 140
    2300 Copenhagen S.
    Denmark

On the internet, you can visit us at [www.cst.ku.dk](http://www.cst.ku.dk)
Here you can also try the CST lemmatiser for Danish and many other languages.
