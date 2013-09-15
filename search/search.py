import pickle
import getopt
import sys
import math
import struct
import operator
import time

# return description of most popular class

def getListofTupleelement(list):
    tempList = []
    for x,y in list:
        tempList.append(lookUpName(y))
    return tempList



def uniqueAndCounterList(query):
    queryWithoutDuplicates = []
    queryCounter = []
    for term in query:
        if term in queryWithoutDuplicates:
            pos = queryWithoutDuplicates.index(term)
            queryCounter[pos] = queryCounter[pos] + 1
        else:
            queryWithoutDuplicates.append(term)
            queryCounter.append(1)

    return queryWithoutDuplicates,queryCounter

def parseQuery(query):
    query = query.lower()
    return query.split()


def getPosting(term):
    postingsFile.seek(dictionary[term][0])
    title = []
    abstract = []
    for i in xrange(0,dictionary[term][1]):
        docID = struct.unpack('i',postingsFile.read(4))[0]
        termFreq = struct.unpack('i',postingsFile.read(4))[0]
        title.append((docID,termFreq))
    for i in xrange(0,dictionary[term][2]):
        docID = struct.unpack('i',postingsFile.read(4))[0]
        termFreq = struct.unpack('i',postingsFile.read(4))[0]
        abstract.append((docID,termFreq))
    return (title,abstract)

# Looks up the fileId in the dictionary and
def lookUpName(fileId):
    global mappingDict

    if mappingDict == None:
        tempFile = open(mapping_file)
        mappingDict = pickle.load(tempFile)

    if fileId in mappingDict:
        return mappingDict[fileId][0]
    else:
        return 0

def lookUpIPC(fileId):
    global mappingDict

    if mappingDict == None:
        tempFile = open(mapping_file)
        mappingDict = pickle.load(tempFile)

    if fileId in mappingDict:
        return mappingDict[fileId][1]
    else:
        return 0

# Returns a string of the elements in the list
def listToString(list):
    s = ''
    for elements in list:
        s += elements + ' '
    return s[:-1]

# Blame user for not understanding
def usage():
    print "usage: " + sys.argv[0] + " -d dictionary-file -p postings-file -q file-of-queries -o output-file-of-results -m mapping_file"

# Reads in the dictionary into memory
def build_dictionary():
    f = open(input_file_d,'r')
    lines = f.readlines()
    dictionary = {}
    for line in lines:
        entry = line.split()
        dictionary[entry[0]] = (int(entry[1]),int(entry[2]),int(entry[3]))
    f.close()
    return dictionary


# index = 1 -> title
# index = 2 -> abstract
def search(query, index):

    # remove duplicates and generate a counterList of the query terms
    query, queryCounter = uniqueAndCounterList(query)
    print "Searching for query: ", query

    # Create the document -> freqList structure used from now on
    documents = {}
    terms = {}
    counter = 0
    length = len(query)
    for token in query:
        terms[token] = counter
        counter += 1
        # If the token exists in the part of the dictionary set by index
        if token in dictionary and dictionary[token][index] != 0:
            # Get the token from the postings list
            seekOffset = dictionary[token][0]
            # If index corresponds to abstract then we must first pass
            # through the title postings for the token in the postings list
            if index == 2:
                seekOffset = seekOffset + dictionary[token][1]*8
                # Navigate to the correct position in the postings file
            postingsFile.seek(seekOffset,0)
            # Loop over all the postings for the token in the title or
            # the abstract depending on index and save them in the
            # document structure
            for i in xrange(0,dictionary[token][index]):
                docID = int(struct.unpack("i",postingsFile.read(4))[0])
                termFreq = int(struct.unpack("i",postingsFile.read(4))[0])
                # The structure is doc -> [0,2,0,0,3] for example. documents is a
                # dictionary with key docID and value a list of counters of the tokens seen
                # so far in the query. All lists pointed to by document is the same length.
                # and each position in the list points to a counter of the same token.
                if(docID not in documents):
                    documents[docID] = []
                    for j in xrange(0,length):
                        documents[docID].append(0)
                    # terms[token] is a dictionary which controls which index in the list a
                # corresponding counter should be stored in.
                documents[docID][terms[token]] = termFreq

    # Make the query scores list used to calculate the sum pr document
    scoreComparer = []
    endScore = []
    # initialize scoreComparer with all zeros
    for i in xrange(0,length):
        scoreComparer.append(0)
    docLength = 0
    # Find tf,df,idf and wt
    for term,value in terms.iteritems():
        tf_raw = queryCounter[query.index(term)]
        tf_wt = float(1 + math.log(tf_raw,10))
        # If the term does not exist in the dictionary we should not try to devide
        # by its document frequency which would be 0. Therefor this special case.
        if term in dictionary and dictionary[term][index] != 0:
            df = float(dictionary[term][index])
            idf = math.log(float(dictionary['*'][1]) / df, 10)
        else:
            df = 0.0
            idf = 0.0
        wt = float(tf_wt * idf)
        scoreComparer[value] = wt
        docLength = docLength + (wt * wt)
        # Cosine normalize
    for term,value in terms.iteritems():
        if scoreComparer[value] == 0:
            scoreComparer[value] = 0
        else:
            scoreComparer[value] = scoreComparer[value]/math.sqrt(docLength)

            # Pr document calculations
        # score multiplication and addition
    for document in documents:
        Sum = 0
        wtlist = []
        wtSum = 0
        for term,value in terms.iteritems():
            tf_raw = documents[document][value]
            if tf_raw != 0:
                wt = float(1 + math.log(tf_raw,10))
                wtlist.append(wt)
                wtSum += wt * wt
        for wt in wtlist:
            Sum += wt/math.sqrt(wtSum) * scoreComparer[value]


        endScore.append((Sum,document))

    # Sort results
    endScore = sorted(endScore,reverse=False,key=operator.itemgetter(1))
    endScore = sorted(endScore,reverse=True,key=operator.itemgetter(0))
    lengthTemp = len(endScore)

    return endScore

### --- MAIN --- ###

# Handle input arguents
mapping_file = input_file_d = input_file_p = input_file_q = output_file = None
try:
    opts, args = getopt.getopt(sys.argv[1:], 'd:p:q:o:m:')
except getopt.GetoptError, err:
    usage()
    sys.exit(2)
for o, a in opts:
    if o == '-d':
        input_file_d = a
    elif o == '-p':
        input_file_p = a
    elif o == '-q':
        input_file_q = a
    elif o == '-o':
        output_file = a
    elif o == '-m':
        mapping_file = a
    else:
        assert False, "unhandled option"

print input_file_d, input_file_p, input_file_q, output_file
if input_file_d == None or input_file_p == None or output_file == None or input_file_q == None:
    usage()
    sys.exit(2)

# ------ VARIABLES ------- #
starttime = time.time()

dictionary = build_dictionary()

endtime = time.time()
print "build_dict time: ", endtime - starttime

mappingDict = None


# Open input/output files
postingsFile = open(input_file_p,'rb')

# Start handling query
title = None
# Read out title and description from query xml
with open(input_file_q) as myfile:
    title = myfile.readline()

starttime = time.time()

query = parseQuery(title)
print query
result = search(query,1)
print result

finalResult = sorted(result, reverse = True)


postingsFile.close()

endtime = time.time()
print "search time:", endtime - starttime


