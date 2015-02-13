clc; close all; clear all;
lambda = @(x) 6*(x(:,1).^2 + x(:,2).^2);
lamstar = 6;
N=poissrnd(lamstar); x = rand(N,2); % homogeneous PP
ind = find(rand(N,1) < lambda(x)/lamstar);
xa = x(ind,:); % thinned PP
% figure; plot(xa(:,1),xa(:,2),'+');
figure; scattercloud(xa(:,1),xa(:,2));

write_coordinates_in_file(xa(:,1)',xa(:,2)','nonhomo_poisson_low.dat');