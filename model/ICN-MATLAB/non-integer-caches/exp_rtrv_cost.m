function exp_t = exp_rtrv_cost(N,alpha,k,d,CB,b)
% Calculates the expected retrieval cost of objects using Che-approximation
% if on-path caching is done and hits take place along the path towards the
% root of the tree (content source).
% Input:
% N     number of objects in the system
% alpha the power law exponent
% k     k-ary tree
% d     number of levels in the hierarchy (depth of the tree)
% b     vector of cache budgets for each level of hierarchy from the bottom
%       to the top
% Output:
% exp_t the expected retrieval cost using the Markov chain model

m = che_aprx(N,alpha,k,d,CB,b);

% Using the Markov chain, calculate the expected hit times
t = ones(1,N);     % expected hit time for each object
for i = 2 : d
    t = t + prod(m(2:i,:),1);
end

q = (1:N).^(-alpha) ./ sum((1:N).^(-alpha));
exp_t = dot(t,q);

end

