from __future__ import division
from math import *
import re 
import collections
import sys

# command line arguments
# <input file> [<session>] 
if len(sys.argv) < 2:
    sys.exit("ERROR: user input file required as parameter")
userFile = sys.argv[1]
sessionFile = ''
if len(sys.argv) > 2:
    sessionFile = sys.argv[2]

# load user data 
userData = [line.rstrip('\n') for line in open(userFile)]
# load new session data
if sessionFile:
    unknownSession = open(sessionFile).readline().rstrip('\n') 

# divide data into shell sessions, start with *SOF*, end with *EOF*
sessions = []
aSesh = []
for line in userData:
    if line == '**SOF**':
        # clear individual session list
        aSesh = []
    elif line == '**EOF**':
        # add individual session to sessions list
        sessions.append(aSesh)
    # ignore the following... 
    # don't care about exit's
    elif line in ('exit', ';'):
        continue
    # don't care about <n> for int n, these were sanitized from user commands
    # and stand in for file names/sensitive data
    elif re.match("^\<[0-9]+\>$", line) != None:
        continue
    # else, add it to the individual session list
    else:
        aSesh.append(line)

# Remove empty sessions
sessions = [s for s in sessions[0:] if s]

userDataSize = len(sessions)
trainSetSize = int(floor(userDataSize * 0.70))
trainSet = sessions[0:trainSetSize]
allTrainCmds = [cmd for session in trainSet for cmd in session]
cmdCounter = collections.Counter(allTrainCmds)

# compute weights, store them in a dict trainWeights 
trainWeights = {}
for i, train in enumerate(trainSet):
    freqTrain = collections.Counter(train)
    weightTrain = {}
    for word, freq in freqTrain.items():
        if word in cmdCounter:
            # term frequency -- inverse document frequency (tf-idf)
            weightTrain[word] = freq * log(trainSetSize / cmdCounter[word])
        else:
            weightTrain[word] = freq 
    trainWeights[i] = weightTrain


def computeNeighbors(test, k):
    testWeights = {}
    for word, freq in collections.Counter(test).items():
        if word in cmdCounter:
            testWeights[word] = freq * log(trainSetSize / cmdCounter[word])
        else:
            testWeights[word] = freq 

    similarities = {}
    for i, train in enumerate(trainSet):
        similarities[i] = 0
        if len(train) <= 0: continue 
        # compute cosine similarity
        # get common words
        commonCmds = list(set(train) & set(test))
        if len(commonCmds) <= 0: continue

        sumOfWeights = reduce(lambda x, y: x + y, map(lambda sharedCmd: testWeights[sharedCmd] * trainWeights[i][sharedCmd], commonCmds)) 
        normTest = sqrt(reduce(lambda x, y: x + y, map(lambda w: pow(w, 2), testWeights.values()))) 
        normTrain = sqrt(reduce(lambda x, y: x + y, map(lambda w: pow(w, 2), trainWeights[i].values())))
        cosSim = sumOfWeights / (normTest * normTrain)
        similarities[i] = cosSim

    kNN = sorted(similarities.iteritems(), key=lambda (k, v): (v, k), reverse=True)[0:k]
    return kNN


# Test set, compute threshold
k = 3
means = []
testSetSizeStart = trainSetSize + 1
testSetSizeEnd = int(floor(trainSetSize + (userDataSize - trainSetSize) / 2.0))
for t in sessions[testSetSizeStart:testSetSizeEnd]:
    if not t: 
        continue
    testKNN  = computeNeighbors(t, k)
    means.append(sum(map(lambda v: v[1], testKNN)) / len(testKNN))

threshold = 1 - sum(means) / len(means)


# Unknown session, format string for input to computeNeighbors()
def formatSession(sess):
    formatted = []
    for cmd in sess.split(';'):
        clist = cmd.split()
        if len(clist) > 0:
            if clist[0] != 'exit' and '/' not in clist[0]:
                formatted.append(clist[0])
            for sub in clist[1:]:
                #if starts with '-' it's an arg and keep only the arg
                if sub.startswith('-'):
                    formatted.append(sub)
                elif sub in ('&', '&&', '|', '<', '<<', '>', '>>'):
                    formatted.append(sub)
                elif any(s in sub for s in ['&', '&&', '|', '<', '<<', '>', '>>']):
                    formatted.append(sub)
    return formatted


# Validation set
def validationTest():
    valResults = {}
    for i, v in enumerate(sessions[testSetSizeEnd + 1:]):
        if not v:
            continue
        valKNN = computeNeighbors(v, k)
        valResults[i + testSetSizeEnd + 1] = (sum(map(lambda x: x[1], valKNN)) / len(valKNN)) >= threshold

    for v in valResults:
        print '[1, ' + str(int(valResults[v])) + '],',


if not sessionFile: 
    validationTest()
else:
    newSession = formatSession(unknownSession)
    resultKNN = computeNeighbors(newSession, k)
    print 'Anomaly detection (1 = true, 0 false): ' + str(int(sum(map(lambda x: x[1], resultKNN)) / len(resultKNN) >= threshold))

