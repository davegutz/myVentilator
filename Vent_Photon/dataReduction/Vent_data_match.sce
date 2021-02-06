// Copyright (C) 2021 - Dave Gutz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// Feb 1, 2021    DA Gutz        Created
// 
// Plot and analyze ventilator data
// Collect data by
//   cygwin> python curlParticle.py
//   This script contains curl link command.
//   You may have to be logged into https://console.particle.io/devices
//   to have necessary permissions to curl
// The Particle files are in ../src
clear
clear globals
mclose('all')
funcprot(0);
exec('filt_functions.sce');
exec('load_data.sce');
exec('overplot.sce');
exec('serial_print.sce');
exec('plotting.sce');
exec('export_figs.sci');
exec('heat_model.sce');

function serial_print_model_1()
    for k=0:size(M.set,1)-2
        serial_print_inputs(k);
        serial_print(k);
        printf('\n');
        mfprintf(doubtfd, '\n');
    end
end


// Deadband
function y = dead(x, hdb)
    // x    input
    // hdb  half of deadband
    // y    output
    y = max(x-hdb, 0) + min(x+hdb, 0);
endfunction


global figs D C P M
try close(figs); end
figs=[];
try
    xdel(winsid())
catch
end

// Local constants
exec('constants.sce');

// File to save results that look something like debug.csv
[doubtfd, err] = mopen('./doubtf.csv', 'wt');
if err then
    printf('********** close doubtf.csv ************\n');
end
mfprintf(doubtfd, 'doubtf.csv debug output of HR4C_data_reduce.sce\n');

// User inputs
run_name = 'largeStepDecr_2021_02_04'
run_name = 'vent_2021-02-05T17-00'
debug=2; plotting = %t; first_init_override = 1;

// Load data.  Used to be done by hand-loading a sce file then >exec('debug.sce');
//run_name = 'vent_2021-01-31T19-25';
data_file = run_name + '.csv';
[data_t, data_d] = load_csv_data(data_file);
if size(data_d,1)==0 then
    error('debug.csv did not load.  Quitting\n')
end
[D, TD, first_init] = load_data(data_t, data_d);
if exists('first_init_override') then
    first_init = first_init_override;
else
    first_init_override = 0;
end
first = first_init;
last = D.N;
B = load_buffer(D, first, last);
B.N = size(B.time, 1);
B.Tf = zeros(D.N, 1);

// Initialize model
M="";C="";
exec('heat_model_constants.sce');
[M, C] = heat_model_define();
[M, C] = heat_model_init(M, C);

// Main loop
time_past = B.time(1);
reset = %t;

for i=1:B.N,
    if i==1 then, reset = %t; end
    time = B.time(i);
    dt = time - time_past;
    time_past = time;
    // Inputs
    %cmd = B.cmd(i);
    if reset then, 
        duty = B.duty(1);
    else
        //duty = B.duty(i);
        duty = C.duty(i-1);
    end
    Tp = B.Tp_Sense(i);
    OAT = B.OAT(i);
    pcnt_pot = B.pcnt_pot(i);
    [a, b, c, e, M] = total_model(time, dt, Tp, OAT, duty, reset, M, i, B);
    
    // Control law
    %set = B.set(i);
    err = %set - M.Ta(i);
    err_comp = dead(err, C.DB)*C.G;
    C.prop(i) = max(min(err_comp * C.tau, 20), -20);
    if reset then,
        C.integ(i) = duty;
    else
        C.integ(i) = max(min(C.integ(i-1) + dt*err_comp, pcnt_pot - C.prop(i)), -C.prop(i));
    end
    cont = max(min(C.integ(i)+C.prop(i), pcnt_pot), 0);
    %cmd = max(min(min(pcnt_pot, cont), 100), 0);
    C.duty(i) = %cmd;
    reset = %f;
end

// Detail serial print
if debug>2 then serial_print_model_1(); end

// Plots
// Zoom last buffer
if plotting then
    if debug>1 then
        plot_all_model()
    end
//    zoom_model([-25000 0])
end

mclose('all')
export_figs(figs, run_name)
