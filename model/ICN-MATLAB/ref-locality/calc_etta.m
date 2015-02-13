function etta = calc_etta(cube,k,d,CB,b)

[nl,nt,no] = size(cube);

% normalize number of requests to obtain request rates (the fraction of
% requests) belonging to a certain object at a given time and cache
% location among all the requests coming at that time to that specific
% cache.
norm_cube = cube;
total_reqs_space_time = sum(cube,3);
for i = 1 : no
    norm_cube(:,:,i) = max(norm_cube(:,:,i) ./ total_reqs_space_time, 0);
end

d_etta = zeros(nl,nt,no);                   % nl x nt x no
for i = 1 : nt
    pops = norm_cube(:,i,:);
    pops = pops(:,:);
    m = calc_miss_rates(pops,k,d,CB,b);     % nl x no x d
    d_etta(:,i,:) = detailed_etta(m);       % nl x nt x no
end

total_reqs = sum(cube(:));
cube = cube ./ total_reqs;

etta = cube .* d_etta;
etta = sum(etta(:));