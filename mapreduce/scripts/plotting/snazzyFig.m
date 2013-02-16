function [] = snazzyFig(varargin)
% snazzy_figure = (figure_handle)
%   author: erik reed
%   
% Snazzies up the given figure!

if isempty(varargin)
    f = gcf;
else
    f = cell2mat(varargin(1));
end

figure(f)
% set(f,'position', [100 100 500 375]);

a = get(gca,'children');
for i=1:length(a)
    try % fails on non plot-function types
        set(a(i),'LineSmoothing','on');
    catch
    end
end

% axis fonts
try
    xl = get(gca,'xlabel');
    set(xl, 'FontName'   , 'AvantGarde');
    set(xl, 'FontSize'   , 10);
catch
end
try
    yl = get(gca,'ylabel');
    set(yl, 'FontName'   , 'AvantGarde');
    set(yl, 'FontSize'   , 10);
catch
end

% legend font
try
    ll = legend;
    set(ll, 'FontName'   , 'AvantGarde');
    set(ll, 'FontSize'   , 10);
catch
end

% grab title
try
    titl = get(gca,'title');
    set( titl, 'FontSize'   , 12,'FontWeight' , ...
        'bold','FontName' , 'AvantGarde');
catch
end

set( gca, 'FontName'   , 'Helvetica' );
set(gca, 'FontSize'   , 8 );

numTicks = 4;

% xticks = get(gca,'xlim');
% xticks = abs(xticks(1) - xticks(2))/numTicks;
yticks = get(gca,'ylim');
ytick = abs(yticks(1) - yticks(2))/numTicks;
set(gca, ...
    'Box'         , 'off'     , ...
    'TickDir'     , 'out'     , ...
    'TickLength'  , [.02 .02] , ...
    'XMinorTick'  , 'on'      , ...
    'YMinorTick'  , 'on'      , ...
    'YGrid'       , 'on'      , ...
    'XColor'      , [.3 .3 .3], ...
    'YColor'      , [.3 .3 .3], ...
    'YTick'       , yticks(1):ytick:yticks(2), ...
    'LineWidth'   , 1);

% set(gcf, 'PaperPositionMode', 'auto');

end