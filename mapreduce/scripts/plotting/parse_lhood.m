close all
clear

net = 'alarm';

duration=600;
num = 300;
sem = cell(num, 1);
mem = cell(num, 1);
alem = cell(num, 1);
semTimes = cell(num, 1);
memTimes = cell(num, 1);
alemTimes = cell(num, 1);

parfor i=1:num
    
    % to process:
%     for i in *; do grep Best $i | cut -d":" -f2,3 | sed 's/ iters://' > $i.lhood; done
    fname1 = sprintf('results_lhood/%s.%d.sem.lhood', net, i);
    fname2 = sprintf('results_lhood/%s.%d.mem.lhood', net, i);
    fname3 = sprintf('results_lhood/%s.%d.alem.lhood', net, i);
    fname1t = sprintf('results_lhood/%s.%d.sem.time', net, i);
    fname2t = sprintf('results_lhood/%s.%d.mem.time', net, i);
    fname3t = sprintf('results_lhood/%s.%d.alem.time', net, i);
    
    if (exist(fname1, 'file') == 2)
        sem{i} = importdata(fname1);
        semTimes{i} = importdata(fname1t);
    end
    
    if (exist(fname2, 'file') == 2)
        mem{i} = importdata(fname2);
        memTimes{i} = importdata(fname2t);
    end
    
    if (exist(fname3, 'file') == 2)
        alem{i} = importdata(fname3);
        alemTimes{i} = importdata(fname3t);
    end
    
    fprintf('Done with %d/%d\n', i, num);
end

%% plot EM paths
close all
figure
hold all
for i=1:10
    if ~isempty(sem{i})
        tmp = sem{i};
        tmpT = semTimes{i}; 
        assert(length(tmpT) == length(tmp))
        
        last = tmp(1);
        lasti = 1;
        for j=1:length(tmp)
            if last > tmp(j)
                l1 = plot(tmpT(lasti:j-1) - tmpT(lasti), tmp(lasti:j-1), 'b');
                lasti = j;
            end
            last = tmp(j);
        end
        l1 = plot(tmpT(lasti:j-1) - tmpT(lasti), tmp(lasti:j-1), 'b');
    end
    if ~isempty(mem{i})
        tmp = mem{i};
        tmpT = memTimes{i};
        assert(length(tmpT) == length(tmp))
        
        last = tmp(1);
        lasti = 1;
        for j=1:length(tmp)
            if last > tmp(j)
                l2 = plot(tmpT(lasti:j-1) - tmpT(lasti), tmp(lasti:j-1), 'g');
                lasti = j;
            end
            last = tmp(j);
        end
        l2 = plot(tmpT(lasti:j-1) - tmpT(lasti), tmp(lasti:j-1), 'g');
    end
    if ~isempty(alem{i})
        tmp = alem{i};
        tmpT = alemTimes{i}; 
        assert(length(tmpT) == length(tmp))
        
        last = tmp(1);
        lasti = 1;
        for j=1:length(tmp)
            if last > tmp(j)
                l3 = plot(tmpT(lasti:j-1) - tmpT(lasti), tmp(lasti:j-1), 'r');
                lasti = j;
            end
            last = tmp(j);
        end
        l3 = plot(tmpT(lasti:j-1) - tmpT(lasti), tmp(lasti:j-1), 'r');
    end
end

legend([l1 l2 l3], 'SEM','MEM','ALEM')

%% plot final histograms
close all
sem_dat = [];
mem_dat = [];
alem_dat = [];
for i=1:num
    if ~isempty(sem{i})
        tmp = sem{i};
        sem_dat(length(sem_dat) + 1) = max(tmp(:, 1));
    end
    if ~isempty(mem{i})
        tmp = mem{i};
        mem_dat(length(mem_dat) + 1) = max(tmp(:, 1));
    end
    if ~isempty(alem{i})
        tmp = alem{i};
        alem_dat(length(alem_dat) + 1) = max(tmp(:, 1));
    end
end

scatter(1:length(sem_dat), sem_dat)
hold all
scatter(1:length(mem_dat), mem_dat)
scatter(1:length(alem_dat), alem_dat)
legend('SEM','MEM','ALEM')
