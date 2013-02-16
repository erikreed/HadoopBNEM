close all
clear

net = 'mini_shared3';

shared = [0 2 3 4 5 6];
samples = 2:2:500;

all_dat = cell(1,length(shared));


for s=1:length(shared)
    s_data = zeros(length(samples), 4);
    for i=1:length(samples)
        dat = importdata(sprintf('%s/s%d/rand_trials/n%d/0', net, shared(s), samples(i)));
        dat = dat.data;
        % lhood, L2, KL-divergence
        s_data(i, :) = [samples(i) dat(end, 2:4)];
        if ~mod(i,10)
        fprintf('%d of %d (%.2f%%)\n', (s-1)*length(samples) + i, length(samples)*length(shared),...
            ((s-1)*length(samples) + i) / (length(samples)*length(shared))*100)
        end
    end
    all_dat{s} = s_data;
end

%% plot
close all
ind = 2;

for s=1:length(shared)
    % samples, lhood, L2, KL-divergence
    plot(all_dat{s}(:,1), all_dat{s}(:,ind))
    hold all
end

xlabel('Number of Samples')
if ind == 2
    ylabel('Log-Likelihood')
else
    ylabel('L2 distance')
end

legend(arrayfun(@(s)sprintf('%d shared', s), shared(1:end),'uniformoutput',false));


%% plot diff versus none-shared
% samples, lhood, L2, KL-divergence
close all

ind = 2;

for s=2:length(shared)
    plot(all_dat{s}(:,1), - all_dat{1}(:,ind) + all_dat{s}(:,ind))
    hold all
end

xlabel('Number of Samples')
if ind == 2
    ylabel('Change in Log-Likelihood')
else
    ylabel('Change in L2 distance')
end
legend(arrayfun(@(s)sprintf('%d shared', s), shared(2:end),'uniformoutput',false));

%%

for s=1:max_shared
    filename = sprintf('%s-%d.csv', net, s);
    dlmwrite(filename, all_dat{s}, 'precision', 10);
end
