#write_tab.py

import sys
import pickle as p

if len(sys.argv) <= 3:
    print "\nUsage:\n python write_tab.py [bayes net data file] [.tab filename] [evidence file 1] ... [evidence file 1,000,000,000]"
    exit()

data_file = sys.argv[1]
tab_file = sys.argv[2]
evidence_files = sys.argv[3:len(sys.argv)]

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

class evidence:
    def __init__(self, data):
        tmp = data.split('\n')
        self.date = tmp[1].lstrip('<instantiation date="').rstrip('">')
        #print self.date
        self.nodes = dict()
        for i in tmp[2:len(tmp)-1]:
            i = i.lstrip('<inst id="').rstrip('"/>').split('" value = "')
            self.nodes[i[0]] = i[1]
        #print self.nodes

#The bayes net data file should include lists of all the nodes and potentials in the net file
#as well as the dictionary of node names to pseudonyms and values for every node,
[nodes, potentials, pseudonym, name] = p.load(open(data_file, 'r'))

evidence_nodes = set()
evidence_instances = []

#read in evidence files
for i in evidence_files:
    f = open(i, 'r')
    data = ""
    for line in f:
        data += line
    tmp = evidence(data)
    evidence_nodes = evidence_nodes.union(set(tmp.nodes.keys()))
    evidence_instances.append(tmp)

evidence_nodes = list(evidence_nodes)

print 'Number of total evidence nodes:', len(evidence_nodes)
print 'Number of evidence instances:', len(evidence_instances)

#return a string where each item in the list is separated by a \t character
def tab(l):
    ans = ''
    for i in range(len(l)):
        ans += str(l[i])
        if i != (len(l) - 1):
            ans += '\t'
    return ans

#write out .tab file
f = open(tab_file, 'w')
tmp = [pseudonym[x] for x in evidence_nodes]
f.write(tab(tmp))
f.write('\n\n')
for i in evidence_instances:
    tmp = []
    for j in evidence_nodes:
        if j in i.nodes:
            tmp.append(nodes[j].states.index(i.nodes[j]))
        else:
            tmp.append('')
    f.write(tab(tmp) + '\n')

