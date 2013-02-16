%% erik reed
% parse alem files
matlabpool
files = dir('results');
data = cell(1, length(files) - 2);
parfor i=3:length(files)
    data{i-2}.filename = files(i).name;
    
    fsplit = strsplit('.', files(i).name);
    data{i-2}.bn = strrep(fsplit(1), '_','');
    data{i-2}.samples = str2double(fsplit(2));
    data{i-2}.hidden = str2double(fsplit(3));
    data{i-2}.alem = isempty(strfind(files(i).name, 'alem'));
    
    [status, result] = system(sprintf('grep Best "results/%s" | cut -d'' '' -f6,8', files(i).name));
    if status ~= 0
        fprintf('Error in parsing: %s\n', files(i).name);
        data{i-2}.dat = [];
    else
       data{i-2}.dat = str2num(result);
        if mod(i, 25) == 0
           fprintf('%d of %d\t(%.2f%%)\n', i, length(files), i / length(files) * 100);
        end 
    end
end

fprintf('Done!\n');
save alarm
matlabpool close
clear fsplit i status result


