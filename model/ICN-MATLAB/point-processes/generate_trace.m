function points = generate_trace(dim,n,alpha,mean_children,file_name,qq_plot)

% dim              spatio-temporal dimension; 2-D space + time ==> dim = 3;
% n                number of objects
% alpha            power-law parameter for object popularities
% mean_children    mean number of children for every generation in Hawkes process
% output           (optional) to specify the output file name
% qq_plot          (optional) to determine if draw qq_plot or not

zipf_prop = (1:n)'.^(-alpha);
zipf_norm = sum((1:n).^(-alpha));
max_intensity = (log10(n) - 1) * (2090 + 18000 * (0.9 - mean_children));  % intensity of the Hawkes proces for the most popular object
lambda = max_intensity * (1:n)'.^(-alpha);

points = zeros(0,dim+1); % time, x, y, ... , obj_id
for i = 1 : n
    xi = hawkes_process(dim, lambda(i), mean_children);
    xi(:,dim+1) = i;
    points = cat(1, points, xi);
end

% truncate off-window points
points(any(points(:,1:dim)<0,2),:) = [];
points(any(points(:,1:dim)>1,2),:) = [];

% sort events in chornological order
[~,I] = sort(points(:,1));
points = points(I,:);

% write trace to file
if nargin == 4, file_name = sprintf('traces/%d_dim/%d_obj/trace_%.1f.dat',dim,n,mean_children); end
write_coordinates_to_file(points,file_name);

%figure; scattercloud(points(:,2),points(:,3),25,1,'none');

%% Generate synthetic power-law data points
np = length(points);
u = rand(np,1);
zipf_cut = [0;cumsum(zipf_prop) / zipf_norm];
counts = histc(u,zipf_cut);
synthetic = zeros(0,1);
for i = 1 : n
    if counts(i) > 0
        synthetic = cat(1, synthetic, i * ones(counts(i),1));
    end
end

if nargin > 5 && qq_plot
    figure;
    qqplot(points(:,dim+1),synthetic);
    box on; axis tight;
    xlim([0 n]); ylim([0 n]);
end