#write_em2.py
#input as many nodes to share as you want by name

import sys
import os
import pickle as p

if len(sys.argv) < 4:
    print "\nUsage:\n python write_em.py .fg_file .dat_file output_file.em [shared_node_1 shared_node_2 ... (only allow one group of nodes to be shared)]"
    exit()

fg_file = sys.argv[1]
dat_file = sys.argv[2]
output_file = sys.argv[3]
stuffToShare = sys.argv[4:len(sys.argv)]

print "Stuff to share:", stuffToShare

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
    def __init__(self):
        self.members = []
    def addMember(self, p):
        self.members.append(p)
    def writeBlock(self, nodes, pseudonym):
        tmp = ""
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
        return tmp

[nodes, potentials, pseudonym, name] = p.load(open(dat_file, 'r'))
#pseudonym is a dict from actual node names to their psudonyms (integer node numbers)
#name is the opposite

#assemble sharedParamBlocks
spb = []
shared_group = sharedParamBlock()
for p in potentials:
    if p.node in stuffToShare:
        shared_group.addMember(p)
    else:
        tmp =sharedParamBlock()
        tmp.addMember(p)
        spb.append(tmp)

if not(shared_group.members == []):
    spb.append(shared_group)


outfile = open(output_file, 'w')
outfile.write('1')
outfile.write('\n\n')

outfile.write(str(len(spb)))
outfile.write('\n')

for i in spb:
    tmp = i.writeBlock(nodes, pseudonym)
    outfile.write(tmp)

#fixedOutFile = open(fixed_file, 'w')
#fixedOutFile.write(fixedNodes)
#fixedOutFile.close()





