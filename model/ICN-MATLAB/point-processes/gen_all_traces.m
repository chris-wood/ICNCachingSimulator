clc; clear all; close all;
dim = 2;
n = 100;
alpha = 1;
mean_children = 0.0 : 0.1 : 0.9;
num_run = 10;

for run = 2 % 1 : num_run
    for mc = mean_children
        output_name = sprintf('traces/%d_dim/%d_obj/trace_%.1f_%d.dat',dim,n,mc,run);
        generate_trace(dim,n,alpha,mc,output_name);
    end
end