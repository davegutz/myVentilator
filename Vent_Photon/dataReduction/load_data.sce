// Load data
function [data_t, data_d] = load_csv_data(csv_file)
    // Load csv file of various columns but data of interest has fixed columns and key in first colum
    // Read all the value in one pass
    // then using csvTextScan is much more efficient
    text = mgetl(csv_file);
    clear n_commas
    for l = 1:size(text, 1)
        n_commas($+1) = size(strindex(text(l), ','), 2);
    end
    nc_data = median(n_commas);  // works for most cases
    i_data = find(n_commas==nc_data);  // all rows with nc_data length
    data_t = csvTextScan(text(i_data), ',', '.', 'string');
    data_d = csvTextScan(text(i_data), ',', '.', 'double');
endfunction


function [D, TD, first] = load_data(data_t, data_d)
    D.N = size(data_d, 1);
    time_adj = data_d(D.N, 2);
    TD.hmString = data_t(:, 1);
    D.time = data_d(:, 2) - time_adj;
    D.set = data_d(:, 3);
    D.Tp_Sense = data_d(:, 4);
    D.Ta_Sense = data_d(:, 5);
    D.cmd = data_d(:, 6);
    D.T = data_d(:, 7);
    D.OAT = data_d(:, 8);
    D.Ta_Obs = data_d(:, 9);
    D.err = data_d(:, 10);
    D.prop = data_d(:, 11);
    D.integ = data_d(:, 12);
    D.cont = data_d(:, 13);
    D.pcnt_pot = data_d(:, 14);
    D.duty = data_d(:, 15)/2.55;
    D.cmd_scaled = D.cmd/100*20+60;
    first = 1;
endfunction


function B = load_buffer(D, first, last);
    B.time = D.time(first:last);
    B.set = D.set(first:last);
    B.Tp_Sense = D.Tp_Sense(first:last);
    B.Ta_Sense = D.Ta_Sense(first:last);
    B.cmd = D.cmd(first:last);
    B.T = D.T(first:last);
    B.OAT = D.OAT(first:last);
    B.Ta_Obs = D.Ta_Obs(first:last);
    B.err = D.err(first:last);
    B.prop = D.prop(first:last);
    B.integ = D.integ(first:last);
    B.cont = D.cont(first:last);
    B.pcnt_pot = D.pcnt_pot(first:last);
    B.duty = D.duty(first:last);
    B.cmd_scaled = D.cmd_scaled(first:last);
endfunction

