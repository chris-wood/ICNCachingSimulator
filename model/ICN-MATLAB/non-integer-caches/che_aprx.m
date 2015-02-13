function [m, t_c] = che_aprx(N, alpha, k, d, CB, b)
% Performs Che-approximation for a linear hierarchy of k levels of caches
% Input:
% N     number of objects in the system
% alpha the power law exponent
% k     k-ary tree
% d     number of levels in the hierarchy (depth of tree)
% CB    total cache budget available
% b     vector of cache budgets for each level of hierarchy from the bottom
%       to the top
% Output:
% m     matrix of miss probabilities for k layers and N objects
% t_c   vector of characteristic times for each layer---the characteristic
%       time is the time it gets the cache to be filled by 'b' unique items

q = (1:N).^(-alpha) ./ sum((1:N).^(-alpha));
% loglog(1:N, q);

m = ones(d,N);      % matrix of miss probabilities for d levels and N objects
t_c = zeros(1,d);   % vector of characteristic times for each level

syms t;
options = optimset('Display','off','TolFun',1e-8);
for i = 1 : d-1
    if i > 1
        q = k * q .* (m(i,:));
    else            % for the zero-th level where the users sit
        q = q .* (m(i,:));
    end
    
    sol = fsolve(@(t)sum(1-exp(-q*t)) - CB*b(i)/k^(d-i), 0, options);
    %
    % vpasolve() solves the che-approximation using symbolic toolbox and 
    % is much slower than fsolve().
    % sol = single(vpasolve(sum(1-exp(-q*t)) - CB*b(i)/k^(d-i) == 0, t, [0 Inf]));
    
    if isempty(sol)
        t_c(i) = 0;
    else
        t_c(i) = sol;
    end
    
    m(i+1,:) = exp(-q * t_c(i));
    % loglog(1:N, 1-m(i+1,:)); hold on;    % plot the hit rates
end