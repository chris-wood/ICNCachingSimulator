function X = hawkes_process(dim,lambda,mean_children)
% dim               output on dim dimensional space
% lambda            intensity of initial points (centers)
% mean_children     mean number of children of each point

X = zeros(10^5,dim);           % initialise the points
N = poissrnd(lambda);          % number of centers
X(1:N,:) = rand(N,dim);        % generate the centers

total_so_far = N;              % total number of points generated
next = 1;
while next < total_so_far
    nextX = X(next,:);         % select next point
    N_children = poissrnd(mean_children);    % number of children
    NewX = repmat(nextX,N_children,1) + 0.02*randn(N_children,dim);
    X(total_so_far+(1:N_children),:) = NewX; % update point list
    total_so_far = total_so_far+N_children;
    next = next+1;
end

X = X(1:total_so_far,:);       % cut off unused rows