tic;
clear; clc; close all;

N = 100;                  % number of objects in the system
alpha = 1;                % power-law exponent for content popularity
k = 4;                    % k-ary tree
d = 5;                    % number of levels in the hierarchy, the top-most being the source
CB = 2*4096;                % total caching budget

init_b = eye(d-1);        % initial cache allocation from the lowest level to the top
best_val = inf;
best_idx = 0;
for i = 1 : d-1
    this_val = exp_rtrv_cost(N,alpha,k,d,CB,init_b(i,:));
    if this_val < best_val
        best_val = this_val;
        best_idx = i;
    end
end
b = init_b(best_idx,:);

f = @(b)exp_rtrv_cost(N,alpha,k,d,CB,b);
Aeq = ones(1,d-1);
beq = 1;
options = optimset('Algorithm','active-set','TolFun',1e-6,'TolX',1e-6); % active-set, interior-point, sqp
[BSTAR,ETTA] = fmincon(f,b,[],[],Aeq,beq,zeros(d-1,1),ones(d-1,1),[],options);
BSTAR
ETTA
toc;