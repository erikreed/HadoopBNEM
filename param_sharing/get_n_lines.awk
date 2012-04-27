#!/usr/bin/awk
BEGIN {
    if (!n) {
        print "Usage: sample.awk -v n=[size]"
        exit
    }
    t = n
    srand()

}

NR <= n {
    pool[NR] = $0
    places[NR] = NR
    next

}

NR > n {
    t++
    M = int(rand()*t) + 1
    if (M <= n) {
        READ_NEXT_RECORD(M)
    }

}

END {
    if (NR < n) {
        print "sample.awk: Not enough records for sample" \
            > "/dev/stderr"
        exit
    }
    # gawk needs a numeric sort function
    # since it doesn't have one, zero-pad and sort alphabetically
    pad = length(NR)
    for (i in pool) {
        new_index = sprintf("%0" pad "d", i)
        newpool[new_index] = pool[i]
    }
    x = asorti(newpool, ordered)
    for (i = 1; i <= x; i++)
        print newpool[ordered[i]]

}

function READ_NEXT_RECORD(idx) {
    rec = places[idx]
    delete pool[rec]
    pool[NR] = $0
    places[idx] = NR  
}
