%% erikreed
% use in the results folder
clear all
close all

%% Parameters (TODO)
ONLY_PLOT_SOME_BNETS = false;
BNETS_TO_PLOT = [{'asd'},{'ponies'}];
% strcmp({listing1.name}','pigs')
MIN_SAMPLES = -1;
MAX_SAMPLES = intmax;

MIN_HIDDEN = -1;
MAX_HIDDEN = intmax;

CPT_PLOT_SAMPLES = log2(50);

%% Run
listing1 = dir;

for i=3:length(listing1)
    if listing1(i).isdir
        bn_name = listing1(i).name;
        if strcmp(bn_name,'cpt_counts') || listing1(i).name(1) == '.'
            continue;
        end
        bn_name = strrep(bn_name, '_', '\_');
        listing2 = dir(listing1(i).name);
        currentdir = listing1(i).name;
        
        %         figure
        figure((i-1)*3)
        subplot(3,1,1)
        hold all
        %         xlabel('Number of samples');
        ylabel('Number of iterations');
        title(strcat('Net: ',bn_name));
        
        subplot(3,1,2)
        hold all
        %         xlabel('Number of samples');
        ylabel('Log-likelihood');
        
        subplot(3,1,3)
        hold all
        xlabel('Log_2 Number of samples');
        ylabel('Error (L2 norm)');
        
        figure((i-1)*3+1)
        subplot(3,1,1)
        hold all
        %         xlabel('Number of samples');
        ylabel('Number of iterations');
        title(strcat('Net: ',bn_name));
        
        subplot(3,1,2)
        hold all
        %         xlabel('Number of samples');
        ylabel('Log-likelihood');
        
        subplot(3,1,3)
        hold all
        xlabel('Number of hidden nodes');
        ylabel('Error (L2 norm)');
        
        figure((i-1)*3+2)
        subplot(3,1,1)
        hold all
        %         xlabel('Number of samples');
        ylabel('Number of iterations');
        title(strcat('Net: ',bn_name, ', samples: ', round(2^CPT_PLOT_SAMPLES)));
        
        subplot(3,1,2)
        hold all
        %         xlabel('Number of samples');
        ylabel('Log-likelihood');
        
        subplot(3,1,3)
        hold all
        xlabel('CPT parameters (num\_parents or CPT entries)');
        ylabel('Error (L2 norm)');
        
        %         subplot(4,1,4)
        %         hold all
        %         xlabel('Log-likelihood');
        %         ylabel('Error (L2 norm)');
        
        legendNames = cell(length(listing2)-2,1);
        legendNames2 = [];
        
        %shared
        num_hidden_s = [];
        num_hidden_s_data = [];
        %hidden
        num_hidden_h = [];
        num_hidden_h_data = [];
        
        for j=3:length(listing2)
            if listing2(j).isdir
                currentdir2 = strcat(currentdir, '\', listing2(j).name);
                fprintf(strcat(strrep(currentdir2,'\','\\'),'\n'))
                shared_num = listing2(j).name(2:end);
                if (listing2(j).name(1)) == 's'
                    currentType = 's';
                    num_hidden_s = [num_hidden_s str2double(shared_num)];
                    legendNames(j-2) = cellstr(strcat(shared_num,', sharing'));
                elseif (listing2(j).name(1)) == 'h'
                    currentType = 'h';
                    num_hidden_h = [num_hidden_h str2double(shared_num)];
                    legendNames(j-2) = cellstr(strcat(shared_num,', no\_sharing'));
                else
                    currentType = 'unknown';
                    legendNames(j-2) = cellstr(strcat('unknown, ',shared_num));
                    fprintf 'unfamiliar prefix\n'
                    continue
                end
                listing3 = dir(strcat(currentdir2, '\', ...
                    'rand_trials'));
                % stores mean, std
                num_iters = zeros(length(listing3)-2,2);
                num_samples = zeros(length(listing3)-2,1);
                num_likelihood  = zeros(length(listing3)-2,2);
                num_error = zeros(length(listing3)-2,2);
                
                for k=3:length(listing3)
                    if listing3(k).isdir
                        currentdir3 = strcat(currentdir2, '\rand_trials\', listing3(k).name);
                        sample_num = listing3(k).name(2:end);
                        
                        num_samples(k-2) = str2double(sample_num);
                        runs_listing = dir(strcat(currentdir3));
                        num_iters_run = zeros(length(runs_listing)-2,1);
                        num_iters_err = zeros(length(runs_listing)-2,1);
                        num_iters_l = zeros(length(runs_listing)-2,1);
                        for runfile=3:length(runs_listing)
                            filepath = strcat(currentdir3,'\',runs_listing(runfile).name);
                            tmp = importdata(filepath);
                            data = tmp.data;
                            num_iters_run(runfile-2) = data(end,1);
                            num_iters_l(runfile-2) = data(end,2);
                            num_iters_err(runfile-2) = data(end,3);
                        end
                        num_iters(k-2,1) = mean(num_iters_run);
                        num_iters(k-2,2) = std(num_iters_run);
                        num_likelihood(k-2,1) = mean(num_iters_l);
                        num_likelihood(k-2,2) = std(num_iters_l);
                        num_error(k-2,1) = mean(num_iters_err);
                        num_error(k-2,2) = std(num_iters_err);
                    end
                end
                num_samples = log2(num_samples);
                [num_samples,i2]=sort(num_samples);
                if currentType=='h'
                    num_hidden_h_data = [num_hidden_h_data,
                        [{num_samples},{num_iters(i2,1)},{num_iters(i2,2),...
                        num_likelihood(i2,1),num_likelihood(i2,2),...
                        num_error(i2,1),num_error(i2,2)}]];
                end
                if currentType=='s'
                    num_hidden_s_data = [num_hidden_s_data,
                        [{num_samples},{num_iters(i2,1)},{num_iters(i2,2),...
                        num_likelihood(i2,1),num_likelihood(i2,2),...
                        num_error(i2,1),num_error(i2,2)}]];
                end
                % figure w/ num_samples x-axis
                lineStyle = ':*';
                if currentType == 'h'
                    lineStyle = '--*';
                end
                figure((i-1)*3)
                
                subplot(3,1,1)
                hold all
                errorbar(num_samples,num_iters(i2,1),num_iters(i2,2), lineStyle);
                hold all
                subplot(3,1,2)
                errorbar(num_samples,num_likelihood(i2,1),num_likelihood(i2,2), lineStyle);
                hold all
                subplot(3,1,3)
                errorbar(num_samples,num_error(i2,1),num_error(i2,2), lineStyle);
                %
                %                 subplot(4,1,4)
                %                 errorbar(num_likelihood(:,1),num_error(:,1),num_error(:,2), '*');
            end
        end
        figure((i-1)*3+1)
        allData = cell2mat(num_hidden_s_data);
        allDataH = cell2mat(num_hidden_h_data);
        cpt_data = importdata(strcat('cpt_counts\',listing1(i).name,'.csv'));
        % num_hidden,cpt_size,num_parents
        cpt_data = cpt_data.data;
        [tmp,i4]=sort(cpt_data(:,1));
        cpt_data = cpt_data(i4,:); % sort in ascending order
        % total CPT size / number of parents
        %         cpt_data_ratio = cpt_data(end-length(num_hidden_h)+1:end,2) ...
        %             ./cpt_data(end-length(num_hidden_h)+1:end,3);
        cpt_data_ratio = cpt_data(end-length(num_hidden_h)+1:end,3);
        
        % for each number of samples (of shared)
        for ij = 1:length(num_samples)
            % collect data
            % note this value numXpoints should divide evenly
            numXpoints = length(num_hidden_s_data)/length(num_samples);
            numSamplesStr = num2str(round(2^num_samples(ij)));
            
            samplesIndicies = cell2mat(num_hidden_s_data(:,1)) == num_samples(ij);
            nSamplesData = allData(samplesIndicies,:);
            
            figure((i-1)*3+1)
            % shared
            [tmp_numHidden,i3]=sort(num_hidden_s);
            legendNames2 = [legendNames2; cellstr(strcat(numSamplesStr,'s'))];
            subplot(3,1,1)
            hold all
            ebl = errorbar(tmp_numHidden,nSamplesData(i3,2),nSamplesData(i3,3), ':*');
            
            subplot(3,1,2)
            hold all
            errorbar(tmp_numHidden,nSamplesData(i3,4),nSamplesData(i3,5), ':*');
            
            subplot(3,1,3)
            hold all
            errorbar(tmp_numHidden,nSamplesData(i3,6),nSamplesData(i3,7), ':*');
            
            % CPT figure
            if num_samples(ij) == CPT_PLOT_SAMPLES
                [tmp_numHidden,i3]=sort(num_hidden_h);
                if ~isequal(cpt_data(end-length(num_hidden_h)+1:end,1),num_hidden_h(i3)')
                    error('CPT file does not match!')
                end
                figure((i-1)*3+2)
                subplot(3,1,1)
                hold all
                errorbar(cpt_data_ratio,nSamplesData(i3,2),nSamplesData(i3,3), '*');
                subplot(3,1,2)
                hold all
                errorbar(cpt_data_ratio,nSamplesData(i3,4),nSamplesData(i3,5), '*');
                subplot(3,1,3)
                hold all
                errorbar(cpt_data_ratio,nSamplesData(i3,6),nSamplesData(i3,7), '*');
            end
            % End of CPT figure
            ebl_color = get(ebl, 'Color');
            
            
            % non-shared
            samplesIndicies = cell2mat(num_hidden_h_data(:,1)) == num_samples(ij);
            nSamplesData = allDataH(samplesIndicies,:);
            
            % plot num_parents vs. iter
            %             errorbar(cpt_data(1:length(num_hidden_h),3), ...
            %                nSamplesData(i3,2),nSamplesData(i3,3), '*');
            
            figure((i-1)*3+1)
            legendNames2 = [legendNames2; cellstr(strcat(numSamplesStr,'h'))];
            subplot(3,1,1)
            hold all
            errorbar(tmp_numHidden,nSamplesData(i3,2),nSamplesData(i3,3), '--*', ...
                'color',ebl_color);
            
            subplot(3,1,2)
            hold all
            errorbar(tmp_numHidden,nSamplesData(i3,4),nSamplesData(i3,5), '--*', ...
                'color',ebl_color);
            
            subplot(3,1,3)
            hold all
            errorbar(tmp_numHidden,nSamplesData(i3,6),nSamplesData(i3,7), '--*', ...
                'color',ebl_color);
            
            % CPT begin
            if num_samples(ij) == CPT_PLOT_SAMPLES
                figure((i-1)*3+2)
                subplot(3,1,1)
                hold all
                errorbar(cpt_data_ratio,nSamplesData(i3,2),nSamplesData(i3,3), '*', ...
                    'color',ebl_color);
                
                subplot(3,1,2)
                hold all
                errorbar(cpt_data_ratio,nSamplesData(i3,4),nSamplesData(i3,5), '*', ...
                    'color',ebl_color);
                
                subplot(3,1,3)
                hold all
                errorbar(cpt_data_ratio,nSamplesData(i3,6),nSamplesData(i3,7), '*', ...
                    'color',ebl_color);
            end
            % CPT end
        end
        
        
        %         subplot(3,1,1)
        %         hold all
        %         plot(cpt_data(:,1),cpt_data(:,3),':')
        %         plot(cpt_data(:,1),cpt_data(:,2),':')
        
        
        figure((i-1)*3)
        subplot(3,1,1)
        if min(num_samples) ~= max(num_samples)
            xlim([min(num_samples),max(num_samples)])
        end
        subplot(3,1,2)
        legend(legendNames);
        if min(num_samples) ~= max(num_samples)
            xlim([min(num_samples),max(num_samples)])
        end
        subplot(3,1,3)
        if min(num_samples) ~= max(num_samples)
            xlim([min(num_samples),max(num_samples)])
        end
        %         subplot(4,1,4)
        %         xlim([min(num_likelihood(:,1)),max(num_likelihood(:,1))])
        %         subplot(3,1,2)
        %         legend(legendNames);
        %         subplot(3,1,3)
        %         legend(legendNames);
        print('-dpng', listing1(i).name)
        
        figure((i-1)*3+1)
        subplot(3,1,1)
        if min(num_hidden_s) ~= max(num_hidden_s)
            xlim([min(num_hidden_s),max(num_hidden_s)])
        end
        subplot(3,1,2)
        legend(legendNames2);
        if min(num_hidden_s) ~= max(num_hidden_s)
            xlim([min(num_hidden_s),max(num_hidden_s)])
        end
        subplot(3,1,3)
        if min(num_hidden_s) ~= max(num_hidden_s)
            xlim([min(num_hidden_s),max(num_hidden_s)])
        end
        print('-dpng', strcat(listing1(i).name,'2'))
        %         close all
        
        figure((i-1)*3+2)
        subplot(3,1,2)
        legend('shared','hidden');
        print('-dpng', strcat(listing1(i).name,'3'))
    end
end

%%