function write_coordinates_to_file(coordinates,filename)

dim_labels = {'time,','x,','y,','z,'};
dim = size(coordinates,2);
desc = '# ';
form = '';
i = 1;
while i < dim
    desc = strcat(desc, dim_labels{i});
    form = strcat(form, '%.8f,');
    i = i + 1;
end

desc = strcat(desc, 'obj_id\n');
form = strcat(form, '%d\n');

fileID = fopen(filename,'w');
fprintf(fileID,desc);
fprintf(fileID,form, coordinates');
fclose(fileID);