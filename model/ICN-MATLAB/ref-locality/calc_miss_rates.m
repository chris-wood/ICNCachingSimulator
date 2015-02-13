% pops          matrix of normalized object popularities nl x n (one
% time-slice at a time)
% k             k-ary tree
% d             depth of the tree
% CB            total caching budget
% b             vector of cache allocation along the tree
function [m,t_c] = calc_miss_rates(pops,k,d,CB,b)

[nl,no] = size(pops);
m = ones(nl,no,d);
t_c = zeros(nl,d);   % vector of characteristic times

syms t;
options = optimset('Display','off','TolFun',1e-8);
for j = 1 : d-1
    for beg_row = 1 : k^j : nl
        end_row = beg_row + k^j - 1;
        sel_rows = beg_row:k^(j-1):end_row;
        pops(beg_row,:) = sum(pops(sel_rows,:) .* (m(sel_rows,:,j)),1);

        sol = fsolve(@(t)sum(1-exp(-pops(beg_row,:)*t)) - CB*b(j)/k^(d-j), 0, options);
        
        if isempty(sol)
            t_c(beg_row,j+1) = 0;
        else
            t_c(beg_row,j+1) = sol;
        end

        m(beg_row:end_row,:,j+1) = repmat(exp(-pops(beg_row,:) * t_c(beg_row,j+1)), end_row - beg_row+1, 1);
    end
end