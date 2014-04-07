p1 = linspace(0.1,0.9,5)'
%colors = 'rbgky';
%colors = [ [1 0 0] | [0 0 1] | [0 1 0] | [0 0 0] | [1 1 0] ]
colors = [ 1 0 0 ; 0 0.6 1; 0 1 0; 0 0 0; 1 0.5 0 ]
markers = 'o+*.xs';
styles = char('--',':','-.','--',':','-.')
i = 1;
h = []
g = []
figure('Units', 'pixels', ...
    'Position', [100 100 840 750]);     % height of image generated
hold on;
set(gca,'Units', 'pixels');
set(gca,'Position', [130 180 620 450]);     % height of chart figure

for p = p1'
   success = []
   hit = []
   queued = []
   throughput = []
   pattern = sprintf('*car%.1f*configindependent.txt', p);
   files = dir(pattern)
   for file = files'
      csv = dlmread(file.name, '', 1, 0)
      time = csv(:,1)
      throughput = cat(2, throughput, (csv(:,2)./csv(:,1)))
      success = cat(2, success, csv(:,2))
      hit = cat(2, hit, csv(:,3))
      queued = cat(2, queued, csv(:,4))
   end
   success = success'
   hit = hit'
   queued = queued'
   throughput = throughput'
   
   %tp = success./csv(:,0)
   mu = mean(throughput)
   mu = mu'
   vu = var(throughput)
   vu = vu'
   su = std(throughput)
   su = su'
   
   %h1 = plot(mu);
   %h1 = errorbar(mu,su)  
   [h1,g1] = stdshade(throughput,0.5,colors(i,:),styles(i,:),markers(i),50-(i*2))
   hold on;
   h = [h;h1]
   g = [g;g1]
   i = i+1;
end

%filename = sprintf('p%.1f.jpg', p);
title({'Throughput of Creatures';''},'FontSize',28,'FontWeight','bold');
xlabel({'';'Time';''},'FontSize',24,'FontWeight','bold');
ylabel('Throughput','FontSize',24,'FontWeight','bold');
set(gca, 'FontSize', 22);
xlim([0 1520])

ax1=axes('visible','off')
xlim(ax1,[0 1520])
box off
l = legend(ax1,h, '0.1', '0.3', '0.5', '0.7', '0.9', 'Orientation', 'horizontal','Location','SouthOutside');
pos=get(l,'position');
pos(2) = pos(2) - 0.05
set(l,'position',pos);
set(ax1, 'FontSize', 22);
set(l, 'FontSize', 22);
set(l,'box','off');

ax2=axes('visible','off')
xlim(ax2,[0 1520])
l2 = legend(ax2, g, '0.1', '0.3', '0.5', '0.7', '0.9', 'Orientation', 'horizontal','Location','SouthOutside');
pos(2) = pos(2) - pos(4)
%pos(1) = pos(1) + 0.01
set(l2,'position',pos);
set(l2,'box','off');
set(ax2, 'FontSize', 22);
set(l2, 'FontSize', 22);


set(gcf, 'PaperPositionMode', 'auto');

print(gcf,'-djpeg99','-loose','tpt');

exit

