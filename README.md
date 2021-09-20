# CSTLEMMA - the CST Lemmatiser


**This distribution contains the following directories and files:**

* doc
    * This directory contains documentation of the program.
* src
    * This directory contains source code and a Makefile.
    * You also need to clone source code from the following repositories to `lib/parsesgml`, `lib/letterfunc` and `lib/hashmap` respectively:
        * https://github.com/kuhumcst/parsesgml
        * https://github.com/kuhumcst/letterfunc
        * https://github.com/kuhumcst/hashmap
    * NOTE: Fetching libraries and compiling the code can be done automatically by running `makecstlemma.bash`.
    * ALTERNATIVELY: Place the three included source directories in the same root directory that the `cstlemma` directory is in and then run `make` inside `cstlemma/src`.
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

* Linux/Mac:
    1. Download (e.g. git clone) cstlemma, parsesgml, letterfunc and hashmap. If you are going to use the Makefile that comes with cstlemma, locate each of these packages in separate subdirectories under the same directory, and call these subdirectories cstlemma, parsesgml, letterfunc and hashmap, respectively. You can run `makecstlemma.bash` to do all of this automatically.
    2. Change directory to the 'cstlemma/src' directory.
    3. Run 'make' (both Linux and Mac) or 'make all' (only Linux). To get rid of object files, run
    4. 'make clean'.

**Running**

For running the CST lemmatiser you need as a minimum a file containing flex
rules. The absolute minimal set of flex rules is the empty set, in which case
the lemmatiser assumes that all words in your input text are perfectly
lemmatised already.

Thus, for checking that the lemmatiser runs OK, you could do the following:

        touch my_empty_rule_file
        ./cstlemma -L -f my_empty_rule_file -i my_text_file.txt

This would create a file my_text_file.txt.lemma that has two tab-separated
columns: the left column contains a word from your text and the right column
contains the same word, converted to lower-case. The -L option tells the
program lemmatise (as opposed to generating flex rules or creating a binary
dictionary). The -f and -i options tell the program which rules and which input
text to read.

The lemmatiser "cstlemma" can generate flex rules from a full-form dictionary,
but a better way to obtain flex rules for use by cstlemma is to use the program
called "affixtrain" (https://github.com/kuhumcst/affixtrain). The flex rules
that cstlemma can produce only look at words from the end ("suffix"-oriented).
Affixtrain, on the other hand, can look at the beginning, the end and in several
places inside a word, all at once. For languages like German and Dutch, were
morphological changes deep inside words often follow strict rules, affixtrain
has a clear advantage over the old suffix based algorithm implemented in
cstlemma, but also languages that only have suffix morphology can have words
that carry important information in other places than near the end about word
class and therefore morphology. A disadavantage of training flex rules with
affixtrain is that it can take a long time, perhaps days if the training data
consists of millions of unique full form - lemma pairs.

The full-form dictionary used to train flex rules can also be used to generate
a binary dictionary, which the program can use to even better lemmatise your
input text. For this task you can use cstlemma (with the -D option).

**Online availability**

CSTLEMMA is demonstrated at CST's website
(http://ada.sc.ku.dk/tools/index.php?lang=en)
and an integrated webservice in the CLARIN-DK infrastructure
(https://clarin.dk/).

**References**

Bart Jongejan and Hercules Dalianis. 2009. Automatic
training of lemmatization rules that handle morphological
changes in pre-, in- and suffixes alike. In
*Proceedings of the Joint Conference of the 47th
Annual Meeting of the ACL and the 4th International
Joint Conference on Natural Language Processing
of the AFNLP*, pages 145–153. Association
for Computational Linguistics.
https://www.aclweb.org/anthology/P09-1017.pdf

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
