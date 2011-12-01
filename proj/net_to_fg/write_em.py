#write_em.py

import sys
import os
import pickle as p

if len(sys.argv) < 4:
    print "\nUsage:\n python write_em.py .fg_file .dat_file output_file.em [extras]\nextras:\n-S adaptvartype adaptcomponenttype\n    shares all nodes that have both adaptvartype and adaptcomponenttype\n-F node_name\n    fixes node_name"
    exit()

fg_file = sys.argv[1]
dat_file = sys.argv[2]
output_file = sys.argv[3]

fixed_file = os.path.splitext(dat_file)[0] + ".fixed"

stuffToShare = []
stuffToFix = []

i=4
while i<len(sys.argv):
    if sys.argv[i] == "-S":
        stuffToShare.append([sys.argv[i+1], sys.argv[i+2]])
        i += 2
    elif sys.argv[i] == "-F":
        stuffToFix.append(sys.argv[i+1])
        i += 1
    else:
        print "Error: extras should be either -S of -F. You appear to have entered", sys.argv[i]
        exit()
    i += 1

print "Stuff to share:", stuffToShare
print "Stuff to fix:", stuffToFix

class node:
    def __init__(self):
        self.properties = None
        self.states = None
        self.name = None

class potential:
    def __init__(self):
        self.data = None
        self.node = None
        self.parents = None

[nodes, potentials, pseudonym, name] = p.load(open(dat_file, 'r'))
#pseudonym is a dict from actual node names to their psudonyms (integer node numbers)
#name is the opposite

#print nodes
#print potentials
#for i in potentials:
#    print i.node
#    print i.parents

#sharedParamBlocks are lists of potentials

fixedNodes = ""

#p is a potential
def printSharedParamBlock(p):
    tmp = ""
    if p.node in stuffToFix:
        tmp += "FixedProbEstimation [target_dim="
        global fixedNodes # i know...
        fixedNodes += str(pseudonym[p.node]) + " "
    else:
        tmp += "CondProbEstimation [target_dim="
    target_dim = len(nodes[p.node].states)
    tmp += str(target_dim)
    tmp += ",total_dim="
    total_dim = target_dim
    for i in [nodes[x] for x in p.parents]:
        total_dim *= len(i.states)
    tmp += str(total_dim)
    tmp += ",pseudo_count=1]\n"
    tmp += "1\n"
    tmp += str(pseudonym[p.node])
    tmp += " "
    tmp += str(pseudonym[p.node])
    for i in range(len(p.parents)):
        tmp += " "
        tmp += str(pseudonym[p.parents[i]])
    tmp += "\n"
    return tmp

print printSharedParamBlock(potentials[0])

outfile = open(output_file, 'w')

outfile.write('1')
outfile.write('\n\n')

outfile.write(str(len(potentials)))
outfile.write('\n')

for i in potentials:
    outfile.write(printSharedParamBlock(i))

fixedOutFile = open(fixed_file, 'w')
fixedOutFile.write(fixedNodes)
fixedOutFile.close()





