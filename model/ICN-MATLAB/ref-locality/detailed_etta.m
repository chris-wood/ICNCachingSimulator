% INPUT
% miss_rates        a nl x no x d matrix containing miss rates of each
%                   cache nl at level d for object no
%
% OUTPUT
% t                 a nl x no matrix whose (i,j) element indicates the ETTA
%                   for object j at node i.
function t = detailed_etta(miss_rates)

[nl,no,d] = size(miss_rates);

t = ones(nl,no);
for i = 2 : d
    t = t + prod(miss_rates(:,:,2:i),3);
end