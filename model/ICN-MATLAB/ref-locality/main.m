clc;
dim = 2;
nobj = 100;
mean_children = (0.0:0.1:0.9);
k = 4;                         % k-ary tree
d = 5;                         % number of levels in the cache hierarchy (with root being source, that is non-cache). leaves are excluded.
nt = [1,2,4,5,6,8,9,10,12,13]; % number of time bins (divide time into nt equal size bins)
CB = 2.^(8:11);                % total caching budget
b = [1,zeros(1,d-2)];          % edge by default
cache_policy = 'optm';
% cache_policy = 'edge';

outfile = strcat(cache_policy,sprintf('_%dobj_%ddeg_%ddpt.txt',nobj,k,d));
fid = fopen(outfile,'w');
idx = 0;
fprintf(fid,'beta\tbudget\tb*\t\t\t\tETTA\n');
fprintf(fid,'-------------------------------------------------------\n');
for beta = mean_children
    idx = idx + 1;
    infile = sprintf('traces/%d_dim/%d_obj/trace_%.1f_1.dat',dim,nobj,beta);
    trace = dlmread(infile,',',1,0); % skip first line
    for budget = CB
        [BSTAR,ETTA] = optimize_cache(trace,k,d,nt(idx),budget,cache_policy);
        fprintf(fid,'%.1f\t',beta);
        fprintf(fid,'%d\t',budget);
        fprintf(fid,'%.4f ',BSTAR); fprintf(fid,'\t');
        fprintf(fid,'%.4f\n',ETTA);
    end
end
fclose(fid);