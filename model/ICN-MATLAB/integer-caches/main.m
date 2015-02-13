tic;
clear; clc; close all;

N = 10000;                      % number of objects in the system
alpha = 1;                      % power-law exponent for content popularity
k = 2;                          % k-ary tree
d = 6;                          % number of levels in the hierarchy, the top-most being the source
CB = k^10;                      % total caching budget

f = @(b)exp_rtrv_cost_int(N,alpha,k,d,b);
A = [2.^(d-1:-1:1);-2.^(d-1:-1:1)];
b = [CB;-CB];
initPop = repmat((CB./(2.^(d-1:-1:1))),d-1,1).*eye(d-1);
options = gaoptimset('StallGenLimit',50,...
                     'TolFun',1e-6,...
                     'Generations',2000,...
                     'PopulationSize',100,...
                     'InitialPopulation',initPop,...
                     'Display','iter');
lb = zeros(d-1,1);
ub = CB*ones(d-1,1) ./ 2.^(d-1:-1:1)';
[b_star,exp_ret] = ga(f,d-1,A,b,[],[],lb,ub,[],(1:d-1),options);
lev_budget = (b_star .* 2.^(d-1:-1:1)) / CB;
toc;