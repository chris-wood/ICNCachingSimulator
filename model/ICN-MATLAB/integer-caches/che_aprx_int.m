function [m, t_c] = che_aprx_int(N,alpha,k,d,b)
% Performs Che-approximation for a linear hierarchy of k levels of caches
% Input:
% N     number of objects in the system
% alpha the power law exponent
% k     k-ary tree
% d     number of levels in the hierarchy (depth of tree)
% b     vector of cache budgets for individual caches at each level of the 
%       hierarchy from the bottom to the top
% Output:
% m     matrix of miss probabilities for k layers and N objects
% t_c   vector of characteristic times for each layer---the characteristic
%       time is the time it gets the cache to be filled by 'b' unique items

q = (1:N).^(-alpha) ./ sum((1:N).^(-alpha));
% loglog(1:N, q);

m = ones(d,N);      % matrix of miss probabilities for d levels (starting 
                    % from 0 to d-1) and N objects. Miss probability for
                    % level 0 (i.e. users) is always 1.
t_c = zeros(1,d);   % vector of characteristic times for each level

syms t;
for i = 1 : d-1
    q = k * q .* (m(i,:));
    
    options = optimset('Display','off','TolFun',1e-8);
    sol = fsolve(@(t)sum(1-exp(-q*t)) - b(i), 0, options);
    
    if isempty(sol)
        t_c(i) = 0;
    else
        t_c(i) = sol;
    end
    
    m(i+1,:) = exp(-q * t_c(i));
    % loglog(1:N, 1-m(i+1,:)); hold on;    % plot the hit rates
end