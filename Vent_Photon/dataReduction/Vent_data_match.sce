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

// Initialize model
M="";
exec('heat_model_constants.sce');
M = heat_model_define();
M = heat_model_init(M);

// Main loop
time_past = B.time(1);
reset = %t;

for i=1:B.N,
    time = B.time(i);
    dt = time - time_past;
    time_past = time;
    // Inputs
    cmd = B.cmd(i);
    Tp = B.Tp_Sense(i);
    OAT = B.OAT(i);
    if i==1 then, reset = %t; end
    [a, b, c, e, M] = total_model(time, dt, Tp, OAT, cmd, reset, M, i);
    reset = %f;
end

// Detail serial print
if debug>2 then serial_print_model_1(); end

// Plots
// Zoom last buffer
if plotting then
    zoom_model([-25000 0])
    if debug>1 then
        plot_all_model()
    end
end

mclose('all')
export_figs(figs, run_name)
