%% plot increasing samples, alem vs non alem

bnet = 'asia';

HIDDEN = 3;

alhood = zeros(5,HIDDEN);
lhood = zeros(5,HIDDEN);
aiters = zeros(5,HIDDEN);
iters = zeros(5,HIDDEN);

alhood_std = zeros(5,HIDDEN);
lhood_std = zeros(5,HIDDEN);
aiters_std = zeros(5,HIDDEN);
iters_std = zeros(5,HIDDEN);

for k=1:5
    for h=1:HIDDEN
        
        alem = 1;
        samples = 100 * 2^(k-1);
        
        % for asia:
        % for h=1:3
        hidden = h*2;
        
        ind = [];
        
        for i=1:length(data)
            if strcmp(bnet, data{i}.bn)
                if data{i}.samples == samples && data{i}.hidden == hidden && ...
                        alem == data{i}.alem
                    ind = [ind i];
                end
            end
        end
        
        ldat = zeros(1,length(ind));
        idat = zeros(1,length(ind));
        for i=1:length(ind)
            [x,y] = size(data{ind(i)}.dat);
            if x == 0 || y == 0
                ldat(i) = 0;
                idat(i) = 0;
            else
                ldat(i) = data{ind(i)}.dat(end,1);
                idat(i) = data{ind(i)}.dat(end,2);
            end
        end
        alhood(k,h) = mean(ldat(ldat ~= 0));
        alhood_std(k,h) = std(ldat(ldat ~= 0));
        aiters(k,h) = mean(idat(idat ~= 0));
        aiters_std(k,h) = std(idat(idat ~= 0));
        
        alem = 0;
        ind = [];
        for i=1:length(data)
            if strcmp(bnet, data{i}.bn)
                if data{i}.samples == samples && data{i}.hidden == hidden && ...
                        alem == data{i}.alem
                    ind = [ind i];
                end
            end
        end
        
        ldat = zeros(1,length(ind));
        idat = zeros(1,length(ind));
        for i=1:length(ind)
            [x,y] = size(data{ind(i)}.dat);
            if x == 0 || y == 0
                ldat(i) = 0;
                idat(i) = 0;
            else
                ldat(i) = data{ind(i)}.dat(end,1);
                idat(i) = data{ind(i)}.dat(end,2);
            end
        end
        lhood(k,h) = mean(ldat(ldat ~= 0));
        lhood_std(k,h) = std(ldat(ldat ~= 0));
        iters(k,h) = mean(idat(idat ~= 0));
        iters_std(k,h) = std(idat(idat ~= 0));
    end
end
%%
close all
errorbar((ones(2,1) * log([100,200,400,800,1600]))', lhood(:,1:2), lhood_std(:,1:2))
hold all
errorbar((ones(2,1) * log([100,200,400,800,1600]))', alhood(:,1:2), alhood_std(:,1:2))
legend('MEM - 2 hidden', 'MEM - 4 hidden', 'ALEM - 2 hidden', 'ALEM - 4 hidden')

%%
close all
errorbar((ones(2,1) * log([100,200,400,800,1600]))', iters(:,1:2), iters_std(:,1:2))
hold all
errorbar((ones(2,1) * log([100,200,400,800,1600]))', aiters(:,1:2), aiters_std(:,1:2))
legend('MEM - 2 hidden', 'MEM - 4 hidden', 'ALEM - 2 hidden', 'ALEM - 4 hidden')

%%
set(gcf,'Position', [200 200 350 275]);
set(gcf,'PaperPositionMode','auto')
print(gcf,'-dpng','-r400', 'iters_vs_lhood2');


%%

bnet = 'asia';
alem = 1;
samples = 800;
hidden = 4;
 
ind = [];

 
for i=1:length(data)
    if strcmp(bnet, data{i}.bn)
        if data{i}.samples == samples && data{i}.hidden == hidden && ...
                alem == data{i}.alem
            ind = [ind i];
        end
    end
end
            
% find lhood
lhood = [];
iters = [];
for i=1:length(ind)
   [x,y] = size(data{ind(i)}.dat);
   if x > 2
       lhood = [lhood data{ind(i)}.dat(end,1)];
       iters = [iters data{ind(i)}.dat(end,2)];
   end
end

elhoodm = zeros(1,15);
elhoods = zeros(1,15);
for i=1:15
   elhoodm(i) = mean(lhood(iters == i));
   elhoods(i) = std(lhood(iters == i));
end

errorbar(3:15, elhoodm(3:end), elhoods(3:end))

