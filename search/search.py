import pickle
import nltk
import re
import getopt
import sys
import math
import patentreader
import struct
import operator

# return description of most popular class


def getListofTupleelement(list):
    tempList = []
    for x,y in list:
        tempList.append(lookUpName(y))
    return tempList


def checkAgainstGoodAndBadList(list):
    goodFile = open(input_file_q[:-4] + '-qrels+ve.txt')
    badFile = open(input_file_q[:-4] + '-qrels-ve.txt')
    gscore = bscore = 0

    resList = getListofTupleelement(list)

    goodlist = []
    line = goodFile.readline()
    while line:
        goodlist.append(line[:-2])
        line = goodFile.readline()
    lengthGood = len(goodlist)
    goodFile.close()

    badlist = []
    line = badFile.readline()
    while line:
        badlist.append(line[:-2])
        line = badFile.readline()
    lengthBad = len(badlist)
    badFile.close()

    for item in goodlist:
        if item in resList:
            gscore += 1

    for item in goodlist:
        if item in resList:
            bscore += 1

    goodScore = float(gscore) / float(lengthGood)
    badScore = float(bscore) / float(lengthBad)

    return goodScore, badScore






def fetchTopClassDesc(result):
    classCounter = dict()
    for fileID in result:
        if len(lookUpIPC(fileID)) > 0:
            ipc = lookUpIPC(fileID)[0]
            if ipc in classCounter:
                classCounter[ipc] = classCounter[ipc] + 1
            else:
                classCounter[ipc] = 1


    top = (0,0)
    for k,v in classCounter.iteritems():
        if v > top[1]:
            top = (k,v)

    print "Top class is " + top[0] + " certainty: " + str(float(top[1]) / float(len(result)))[:4]
    print "Candidates: ", classCounter
    return ipcDict[top[0]]

def uniqueAndCounterList(query):
    stopWords = nltk.corpus.stopwords.words('english')
    queryWithoutDuplicates = []
    queryCounter = []
    for term in query:
        # skip stop words
        if term in stopWords:
            continue
        if term in queryWithoutDuplicates:
            pos = queryWithoutDuplicates.index(term)
            queryCounter[pos] = queryCounter[pos] + 1
        else:
            queryWithoutDuplicates.append(term)
            queryCounter.append(1)


    return queryWithoutDuplicates,queryCounter

def parseQuery(query):
    stemmer = nltk.PorterStemmer()

    # Split up in tokens
    sentences = nltk.sent_tokenize(query.lower())
    newQuery = []
    for sentence in sentences:
        newQuery += nltk.word_tokenize(sentence)

    # stem and remove special charaters
    returnQuery = []
    for term in newQuery:
        term = re.sub('[^a-z0-9]', '', term)
        term = stemmer.stem(term)
        if term == '':
            continue
        returnQuery.append(term)

    return returnQuery

def getPosting(term):
    postingsFile.seek(dictionary[term][0])
    title = []
    abstract = []
    for i in range(0,dictionary[term][1]):
        docID = struct.unpack('i',postingsFile.read(4))[0]
        termFreq = struct.unpack('i',postingsFile.read(4))[0]
        title.append((docID,termFreq))
    for i in range(0,dictionary[term][2]):
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


def threeRares(queryList):
    first = second = third = (2000,2000)
    for element in queryList:
        count = dictionary[element][1] + dictionary[element][2]

        if count < first[0]:
            third = second
            second = first
            first = (count,element)
        elif count < second[0]:
            third = second
            second = (count,element)
        elif count < third[0]:
            third = (count,element)
    return [first[1],second[1],third[1]]

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
            for i in range(0,dictionary[token][index]):
                docID = int(struct.unpack("i",postingsFile.read(4))[0])
                termFreq = int(struct.unpack("i",postingsFile.read(4))[0])
                # The structure is doc -> [0,2,0,0,3] for example. documents is a
                # dictionary with key docID and value a list of counters of the tokens seen
                # so far in the query. All lists pointed to by document is the same length.
                # and each position in the list points to a counter of the same token.
                if(docID not in documents):
                    documents[docID] = []
                    for j in range(0,length):
                        documents[docID].append(0)
                    # terms[token] is a dictionary which controls which index in the list a
                # corresponding counter should be stored in.
                documents[docID][terms[token]] = termFreq

    # Make the query scores list used to calculate the sum pr document
    scoreComparer = []
    endScore = []
    # initialize scoreComparer with all zeros
    for i in range(0,length):
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
    return [x[1] for x in endScore]

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

dictionary = build_dictionary()
mappingDict = None

ipcFile = open('ipcDict.txt')
ipcDict = pickle.load(ipcFile)
ipcFile.close()


# Open input/output files
postingsFile = open(input_file_p,'rb')
outputFile = open(output_file, 'w')


# Start handling query

# Read out title and description from query xml
title, desc = patentreader.parse_query(input_file_q)

# Remove first "Relevant documents will describe" of description
desc = desc[33:]

query = title
query = parseQuery(query)

q1 = parseQuery(title)
q2 = parseQuery(desc)

q1r = threeRares(q1)
q2r = threeRares(q2)

resultTitle = search(q1+q2r,1)

query = desc
query = parseQuery(query)



resultDesc = search(q1r+q2,2)

# print resultTitle
# print resultDesc
finalResult = []
for score, docID in resultTitle:
    found = 0
    for sc, dID in resultDesc:
        if docID == dID:
            finalResult.append((1*score + 1 * sc, docID))
            found = 1
            break
    if found == 0:
        finalResult.append((score,docID))

finalResult = sorted(finalResult, reverse = True)



result = finalResult
fileNameResult = []
for score, fileId in result:
    fileNameResult.append(fileId)
# print fetchTopClassDesc(fileNameResult)

# strippedres = []

# for element in result:
#     if lookUpIPC(element[1])[0] == 'B08B' or lookUpIPC(element[1])[0] == 'D06F':
#         strippedres.append(element)
# result = strippedres







# for score, fileID in result:
#     if fileID not in scoreDict:
#         scoreDict[fileID] = [score]
#     else:
#         scoreDict[fileID].append(score)

# result = search(query,2)[:20]
# for score, fileID in result:
#     if fileID not in scoreDict:
#         scoreDict[fileID] = [score]
#     else:
#         scoreDict[fileID].append(score)

# print scoreDict

# ipcDesc = fetchTopClassDesc(result)
# query += ipcDesc

# result = search(query,2)[:20]



# Count classes and vote on correct class

print checkAgainstGoodAndBadList(result)

postingsFile.close()
outputFile.close()



