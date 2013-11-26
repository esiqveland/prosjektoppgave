
import load_common
import sys
import random

__author__ = 'eivind'

def generateAndWriteQuery(dictionary, outputfile, num=100000):
    out = open(outputfile, 'w+')
    query = load_common.Query(dict_filepath=dictionary)
    for i in xrange(0,num-1):
        out.write(query.generateQuery(random.randrange(1,6))+"\n")
    out.write(query.generateQuery(random.randrange(1,6)))

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "usage: {} path-to-dict outputfile [number_queries]".format(sys.argv[0])
        sys.exit(-1)
    dictfile = sys.argv[1]
    outfile = sys.argv[2]
    if len(sys.argv) == 4:
        generateAndWriteQuery(dictionary=dictfile, outputfile=outfile, num=int(sys.argv[3]))
    else:
        generateAndWriteQuery(dictionary=dictfile, outputfile=outfile)
