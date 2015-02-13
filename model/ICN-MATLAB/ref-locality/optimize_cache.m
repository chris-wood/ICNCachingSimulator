function [BSTAR,ETTA] = optimize_cache(trace,k,d,nt,CB,cache_policy)

b = [1,zeros(1,d-2)];   % edge by default

cube = gen_data_cube(trace, k, d, nt);         % nl x nt x no elements

if strcmp(cache_policy,'optm')
    f = @(b)calc_etta(cube,k,d,CB,b);
    Aeq = ones(1,d-1);
    beq = 1;
    options = optimset('Display','off','Algorithm','active-set','TolFun',1e-6,'TolX',1e-6); % active-set, interior-point, sqp
    [BSTAR,ETTA] = fmincon(f,b,[],[],Aeq,beq,zeros(d-1,1),ones(d-1,1),[],options);
else
    assert(strcmp(cache_policy,'edge'));
    BSTAR = [1,zeros(1,d-2)];
    ETTA = calc_etta(cube,k,d,CB,BSTAR);
end