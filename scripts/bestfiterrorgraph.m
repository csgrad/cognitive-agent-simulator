avgdata = [];
vardata = [];
stddata = [];
files = dir('*stats.txt');
p1 = linspace(0.09,0.89,5)';
p2 = linspace(0.1,0.9,5)';
p3 = linspace(0.11,0.91,5)';
for file = files'
    csv = dlmread(file.name);
    mu = mean(csv);
    vu = var(csv);
    su = std(csv(:,[5,6,7]));
    avgdata = [avgdata; mu(5:7)];
    vardata = [vardata; vu(5:7)];
    stddata = [stddata; su];
end

figure('Units', 'pixels', ...
    'Position', [100 100 840 720]);     % height of image generated
hold on;

ylim([-50, 2100])
h1 = errorbar(p1,avgdata(:,1),stddata(:,1),'.g','MarkerSize',24,'LineWidth',2);
h1Children = get(h1, 'children');
set(h1Children(2), 'linestyle','--')
h2 = errorbar(p2,avgdata(:,2),stddata(:,2),'.r','MarkerSize',24,'LineWidth',2);
h2Children = get(h2, 'children');
set(h2Children(2), 'linestyle','--')
h3 = errorbar(p3,avgdata(:,3),stddata(:,3),'.b','MarkerSize',24,'LineWidth',2);
h3Children = get(h3, 'children');
set(h3Children(2), 'linestyle','--')
h = [h1;h2];
h = [h;h3];

ls = lsline()
set(ls, 'LineWidth',3)
set(gca,'Units', 'pixels');
set(gca,'Position', [130 150 620 450]);     % height of chart figure
title({'Number of Creatures at Simulation End';''},'FontSize',28,'FontWeight','bold');
xlabel({'';'Probability of Cars';''},'FontSize',24,'FontWeight','bold');
set(gca,'XTickLabel',{'p=0.1','p=0.3', 'p=0.5', 'p=0.7', 'p=0.9'})
set(gca,'XTick',p2)
ylabel('Creatures','FontSize',24,'FontWeight','bold');
l = legend(h, 'Successful', 'Killed', 'Queued', 'Orientation', 'horizontal','Location','SouthOutside');
set(gca, 'FontSize', 22);
set(l, 'FontSize', 22);
set(gcf, 'PaperPositionMode', 'auto');
print(gcf,'-djpeg99','-loose','bestfit-skq');

exit
