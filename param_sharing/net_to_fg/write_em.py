#write_em.py
#input parameters specifically for sharing in adapt

import sys
import os
import pickle as p

if len(sys.argv) < 4:
    print "\nUsage:\n python write_em.py .fg_file .dat_file output_file.em [extras]\nextras:\n-S adaptvartype adaptcomponenttype\n    shares all nodes that have both adaptvartype and adaptcomponenttype\n-F node_name\n    fixes node_name"
    exit()

fg_file = sys.argv[1]
dat_file = sys.argv[2]
output_file = sys.argv[3]

fixed_file = os.path.splitext(output_file)[0] + ".fixed"

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
    def theRightStuff(self, stuff):
        if not(eval(self.properties['adaptvartype']) == stuff[0]):
            return False
        if not(eval(self.properties['adaptcomponenttype']) == stuff[1]):
            return False
        return True

class potential:
    def __init__(self):
        self.data = None
        self.node = None
        self.parents = None
    def theRightStuff(self, stuff, nodes):
        return nodes[self.node].theRightStuff(stuff)

#sharedParamBlocks are lists of potentials
class sharedParamBlock:
    def __init__(self, stuff):
        self.stuff = stuff
        self.members = []
    def addMember(self, p):
        self.members.append(p)
    def writeBlock(self, nodes, pseudonym, fixedNodes):
        tmp = ""
        if self.members[0].node in stuffToFix:
            tmp += "FixedProbEstimation [target_dim="
            fixedNodes += str(pseudonym[self.members[0].node]) + " "
        else:
            tmp += "CondProbEstimation [target_dim="
        target_dim = len(nodes[self.members[0].node].states)

        tmp += str(target_dim)
        tmp += ",total_dim="
        total_dim = target_dim
        for i in [nodes[x] for x in self.members[0].parents]:
            total_dim *= len(i.states)
        tmp += str(total_dim)
        tmp += ",pseudo_count=1]\n"

        tmp += str(len(self.members))
        tmp += "\n"

        for i in self.members:
            tmp += str(pseudonym[i.node])
            tmp += " "
            tmp += str(pseudonym[i.node])
            for j in range(len(i.parents)):
                tmp += " "
                tmp += str(pseudonym[i.parents[j]])
            tmp += "\n"
        return tmp, fixedNodes

[nodes, potentials, pseudonym, name] = p.load(open(dat_file, 'r'))
#pseudonym is a dict from actual node names to their psudonyms (integer node numbers)
#name is the opposite

fixedNodes = ""

def toShare(stuff, stuffToShare):
    for i in stuffToShare:
        if stuff[0] == i[0] and stuff[1] == i[1]:
            return True
    return False

#assemble sharedParamBlocks
spb = []
for p in potentials:
    withoutAHome = True
    for i in spb:
        if toShare(i.stuff, stuffToShare) and p.theRightStuff(i.stuff, nodes):
            i.addMember(p)
            withoutAHome = False
    if withoutAHome:
        tmp = sharedParamBlock([eval(nodes[p.node].properties['adaptvartype']),
                                eval(nodes[p.node].properties['adaptcomponenttype'])])
        tmp.addMember(p)
        spb.append(tmp)

outfile = open(output_file, 'w')

outfile.write('1')
outfile.write('\n\n')

outfile.write(str(len(spb)))
outfile.write('\n')

for i in spb:
    tmp, fixedNodes = i.writeBlock(nodes, pseudonym, fixedNodes)
    outfile.write(tmp)

fixedOutFile = open(fixed_file, 'w')
fixedOutFile.write(fixedNodes)
fixedOutFile.close()





