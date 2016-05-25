close all
clear all
clc

%%Load data
PCB = load('PCB_data_before.csv');
DMM = load('DMM_data_before.csv');

DMM = [DMM; NaN(size(PCB,1)-size(DMM,1),1)];

%filter the peak values
PCB = medfilt1(PCB);

% find the steps for PCB
a = find(diff(PCB)> 0.2);
b = find(diff(PCB)< -0.2);
intervals_PCB = [a; b];
intervals_PCB = sort(intervals_PCB);
% make sure only the steps are registered and not a small deviation
a = find(diff(intervals_PCB)>15);
intervals_PCB = intervals_PCB(a);

% find the steps for DMM
a = find(diff(DMM)> 0.2);
b = find(diff(DMM)< -0.2);
intervals_DMM = [a; b];
intervals_DMM = sort(intervals_DMM);
% make sure only the steps are registered and not a small deviation
a = find(diff(intervals_DMM)>12);
intervals_DMM = intervals_DMM(a);

% % plot both sets
% figure
% plot([PCB DMM]);
% legend('PCB','DMM')


disp(['intervals PCB: ' int2str(size(intervals_PCB,1)) ', intervals DMM: ' int2str(size(intervals_DMM,1))])

% for every interval between two steps do
for i = 1:size(intervals_PCB,1)-1
    % prin the interval
    %disp(['interval: ' int2str(i) ' - ' int2str(intervals_PCB(i+1)-intervals_PCB(i)) ' elementen '])
    % select the values: remove the two first and last measurements
    list = [PCB(intervals_PCB(i)+2:intervals_PCB(i+1)-2)]';      
    % clean up the NaN
    list = list(~isnan(list));
    % calc the mean
    PCBmean(i) = mean(list);
    %disp(['mean: ' int2str(PCBmean(i))])
end
PCBmean = PCBmean(~isnan(PCBmean));
PCBmean = PCBmean';

% for every interval between two steps do
for i = 1:size(intervals_DMM,1)-1
    % prin the interval
    %disp(['interval: ' int2str(i) ' - ' int2str(intervals_DMM(i+1)-intervals_DMM(i)) ' elementen '])
    % select the values: remove the two first and last measurements
    list = [DMM(intervals_DMM(i)+2:intervals_DMM(i+1)-2)]';      
    % calc the mean
    DMMmean(i) = mean(list);
    %disp(['mean: ' int2str(DMMmean(i))])
end
DMMmean = DMMmean(~isnan(DMMmean));
DMMmean = DMMmean';

% % plot
% figure
% plot(PCBmean,'ob')
% hold on;
% plot(DMMmean,'xr')
% legend('PCB','DMM','location','south')

% plot to do calibration
% figure 
% scatter(DMMmean(1:end-2),PCBmean(1:end-2))
% ylabel('PCB')
% xlabel('DMM reading')
p = polyfit(PCBmean(1:end-2),DMMmean(1:end-2),1);
disp(['y = ' sprintf('%0.5f',p(1)) ' x + ' sprintf('%0.5f',p(2))])
%saveas(gcf, 'scatter', 'png')

D = abs(DMMmean-PCBmean).^2;
MSE = sum(D(:))/numel(DMMmean)
