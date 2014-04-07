function [h,g] = stdshade(amatrix,alpha,acolor,style,marker,gap,F,smth)
% usage: stdshading(amatrix,alpha,acolor,F,smth)
% plot mean and sem/std coming from a matrix of data, at which each row is an
% observation. sem/std is shown as shading.
% - acolor defines the used color (default is red) 
% - F assignes the used x axis (default is steps of 1).
% - alpha defines transparency of the shading (default is no shading and black mean line)
% - smth defines the smoothing factor (default is no smooth)
% smusall 2010/4/23

if exist('acolor','var')==0 || isempty(acolor)
    acolor='r'; 
end

if exist('F','var')==0 || isempty(F); 
    F=1:size(amatrix,2);
end

if exist('smth','var'); if isempty(smth); smth=1; end
else smth=1;
end  

if ne(size(F,1),1)
    F=F';
end

amean=smooth(nanmean(amatrix),smth)';
astd=nanstd(amatrix); % to get std shading
% astd=nanstd(amatrix)/sqrt(size(amatrix,1)); % to get sem shading


%plot(F,amean+astd, 'Color',acolor, 'linestyle',style,'LineWidth',3)
g = plot(F(1:gap:end),amean(1:gap:end)+astd(1:gap:end),'Color',acolor,'Marker',marker,'LineWidth',3,'linestyle','none');
%plot(F,amean-astd, 'Color',acolor, 'linestyle',style,'LineWidth',3)
plot(F(1:gap:end),amean(1:gap:end)-astd(1:gap:end),'Color',acolor,'Marker',marker,'LineWidth',3,'linestyle','none');

%mainline markers
%plot(F(1:gap:end),amean(1:gap:end),'Color',acolor,'Marker',marker,'linestyle','none');

%% commented out by jason
%if exist('alpha','var')==0 || isempty(alpha) 
%    fill([F fliplr(F)],[amean+astd fliplr(amean-astd)],acolor,'linestyle','none');
%    acolor='k';
%else fill([F fliplr(F)],[amean+astd fliplr(amean-astd)],acolor, 'FaceAlpha', alpha,'linestyle','none');    
%end

if ishold==0
    check=true; else check=false;
end

hold on; h = plot(F,amean,'Color',acolor,'LineWidth',3); %% change color or linewidth to adjust mean line


if check
    hold off;
end

end



