QUERYFILE="target/query_file.txt"

python2.7 search/search.py -d target/dictionary.txt -p target/postings.txt -q $QUERYFILE -o target/python.search.out -m target/mapping.txt
