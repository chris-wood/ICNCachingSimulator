%%
clc; clear all; close all;
X = hawkes_process(3,40,0.95);

%%
bins = 8;
XX = X;
XX(any(XX(:,1:3)<0,2),:) = [];
XX(any(XX(:,1:3)>1,2),:) = [];

T = sort(XX(:,3));
T = T / max(T);

%%
figure; hold on;
scatter3(XX(:,3),XX(:,2),XX(:,1),'.k');
z = hist3(XX(:,2:3),[bins,bins]);
contourf(linspace(0,1,bins),linspace(0,1,bins),z);
colormap(flipud(gray));
xlim([0,1]); ylim([0,1]); zlim([0,1]);

%%
t = linspace(0,1,31);
z = histc(T,t);
z = z/max(z)/3;
figure; bar(t,z); colormap(flipud(gray));
for i = 1 : length(t)
    fprintf('(%.2f,0,%.2f) ',z(i),t(i));
end
fprintf('\n');

%%
hist(T,linspace(0,1,31));