close all
clear all
clc

%%Load data
PCB = load('PCB_data.csv');
DMM = load('DMM_data.csv');

DMM = [DMM; NaN(size(PCB,1)-size(DMM,1),1)];

% remove the outlayer
PCB(400) = NaN;

% plot both sets
plot([PCB DMM]);
legend('PCB','DMM')

saveas(gcf, 'rawdata', 'png')

% find the steps
a = find(diff(PCB)> 0.2);
b = find(diff(PCB)< -0.2);
intervals = [a; b];
intervals = sort(intervals);
% make sure only the steps are registered and not a small deviation
a = find(diff(intervals)>15);
intervals = intervals(a);

% for every interval between two steps do
for i = 1:size(intervals,1)-1
    % prin the interval
    disp(['interval: ' int2str(i) ' - ' int2str(intervals(i+1)-intervals(i)) ' elementen '])
    % select the values: remove the two first and last measurements
    list = [PCB(intervals(i)+2:intervals(i+1)-2)]'      
    % clean up the NaN
    list = list(~isnan(list))
    % calc the mean
    PCBmean(i) = mean(list);
    disp(['mean: ' int2str(PCBmean(i))])
    % cals the std
    PCBstd(i) = std(list);        
end
PCBmean = PCBmean(~isnan(PCBmean))
PCBstd = PCBstd(~isnan(PCBstd))
% plot
figure
plot(PCBmean,'o')
hold on

% same stuff as for the PCB measurements
a = find(diff(DMM)> 0.2);
b = find(diff(DMM)< -0.2);
intervals = [a; b];
intervals = sort(intervals);
a = find(diff(intervals)>13);
intervals = intervals(a);

for i = 1:size(intervals,1)-1
    disp(['interval: ' int2str(i) ' - ' int2str(intervals(i+1)-intervals(i)) ' elementen '])
    list = [DMM(intervals(i)+2:intervals(i+1)-2)]'
    DMMVmean(i) = mean(list);
    disp(['mean: ' int2str(DMMVmean(i))])    
end

DMMVmean = DMMVmean(~isnan(DMMVmean))
plot(DMMVmean,'x')
legend('PCB','DMM','location','south')
%saveas(gcf, 'data', 'png')

% calc the difference
difference  = [PCBmean'-DMMVmean']
% plot to do some regresion analyses
figure 
plot(DMMVmean(1:end-2),difference(1:end-2),'x')
ylabel('difference')
xlabel('DMM reading')
p = polyfit(DMMVmean(1:end-2),difference(1:end-2)',1)
disp(['y = ' sprintf('%0.5f',p(1)) ' x + ' sprintf('%0.5f',p(2))])
%saveas(gcf, 'difference', 'png')

% plot to do calibration
figure 
scatter(DMMVmean(1:end-2),PCBmean(1:end-2))
ylabel('PCB')
xlabel('DMM reading')
p = polyfit(DMMVmean(1:end-2),PCBmean(1:end-2),1)
disp(['y = ' sprintf('%0.5f',p(1)) ' x + ' sprintf('%0.5f',p(2))])
%saveas(gcf, 'scatter', 'png')
