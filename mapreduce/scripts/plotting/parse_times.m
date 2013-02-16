filename = 'water.120.15.1.alem.log';
[status, s] = system(sprintf('cat "%s" | cut -d'' '' -f3 | head -1 | sed ''s/elapsed//g''', filename));

assert(status == 0)
hms = str2num(strsplit(':', s));
t = hms(1) + 60*hms(2) + 3600*hms(3);
