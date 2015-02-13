% Produces a nl x nt x n data cube out of the input trace matrix, where nl
% is the number of leaves (requesters) in the tree, nt is the number of
% time bins, and n is the number of unique object files in the trace. Each
% element of the cube represents the normalized fraction of requests coming
% at node nl for object n at the nt-th time interval.
%
% INPUTS:
% trace         input trace matrix, must be two-dimensional (time,x)
% k             k-ary tree
% d             depth of the tree
% nt            number of time bins
function cube = gen_data_cube(trace, k, d, nt)

no = max(trace(:,end));      % number of object files
nl = k^d;                   % number of leaves (requesters) in the tree

leaf_cut = 1/nl : 1/nl : 1;
time_cut = 1/nt : 1/nt : 1;

% map x locations to discrete requester id's at the bottom level
for i = 1 : length(leaf_cut)
    idx = trace(:,2) < leaf_cut(i) & trace(:,2) < 1;
    trace(idx,2) = i;
end
% sort with respect to the requester id, then chornological
[~,I] = sort(trace(:,2));
trace = trace(I,:);

% organize trace data into a data cube
cube = zeros(nl,nt,no);
beg_row = 1;
for i = 1 : nl
    if isempty(find(trace(beg_row:end,2) == i, 1)), continue; end
    next_leaf = i + 1;
    while next_leaf <= nl
        end_row = find(trace(beg_row:end,2) == next_leaf, 1) + beg_row - 1;
        if ~isempty(end_row), end_row = end_row - 1; break; 
        else next_leaf = next_leaf + 1;
        end
    end
    if next_leaf > nl, end_row = length(trace); end
    
    for j = 1 : nt
        idx = find(trace(beg_row:end_row,1) < time_cut(j)) + beg_row - 1;
        cube(i,j,:) = hist(trace(idx,3),1:no);
        if ~isempty(idx), beg_row = max(idx) + 1; end        
        if beg_row > end_row, break; end
    end
end

%% check the correctness of the operation
%
% check1 = sum(sum(data_cube,1),2);
% check1 = check1(:);
% check2 = hist(trace(:,3),1:no)';
% any(check1 - check2)

%% draw visualization of requests over time as a sanity check
%
% leaf = 20;
% for obj = 1 : 10
%     subplot(10,1,obj);
%     stem(data_cube(leaf,:,obj));
% end