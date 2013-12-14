Prosjektoppgave Høsten 2013
av Eivind Siqveland og Knut Nygaard
####

To build C code, run make.

To start search engine, run:

    > ./bin/search_bin [port]

port is optional, and defaults to 32003.

To index, run
    ./index.sh

It will index documents in ./corpus/


Compile av rapport bruker latexmk, legges inn med

    sudo tlmgr install latexmk

eller

    sudo brew install latex-mk
