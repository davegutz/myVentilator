// Plotting
default_loc = [40 30];

function plot_data(%zoom, %loc)
    global figs D C P
    %size = [610, 460];
    if ~exists('%zoom') then
        %zoom = '';
    end
    if ~exists('%loc') then
        %loc = default_loc + [50, 50];
    end
    figs($+1) = figure("Figure_name", 'Data', "Position", [%loc, %size]);
    subplot(211)
    overplot(['P.D.Tp_Sense', 'P.D.set', 'P.D.Ta_Sense', 'P.D.OAT'], ['r-', 'g-', 'm-', 'b-'], 'Data temperatures', 'time, s', %zoom)
    subplot(212)
    overplot(['P.D.prop', 'P.D.integ', 'P.D.cont', 'P.D.duty'], ['r--', 'g--', 'b-', 'c-'], 'Data duty', 'time, s', %zoom)
endfunction

function plot_compare(%zoom, %loc)
    global figs
    %size = [610, 460];
    if ~exists('%zoom') then
        %zoom = '';
    end
    if ~exists('%loc') then
        %loc = default_loc + [100, 100];
    end

    figs($+1) = figure("Figure_name", 'Data v Model -'+data_file, "Position", [%loc, %size]);
//    subplot(221)
//    overplot(['P.D.ir_rate'], ['b-'], 'Rate Compare', 'time, s', %zoom, [-10000 5000])
//    subplot(222)
//    overplot(['P.D.ir_filt'], ['b-'], 'Filt Compare', 'time, s', %zoom, [-10000 5000])
//    subplot(223)
//    overplot(['P.D.red', 'P.D.ir'], ['r-', 'b-'], 'Data Compare')
//    subplot(224)
//    overplot(['P.D.hr', 'P.D.spo2'], ['r-', 'b-'], 'Data')

endfunction

function plot_all(%zoom, %loc)
    global figs D C P
    if ~exists('%zoom') then
        %zoom = '';
        %loc = [40, 30];
        else if ~exists('%loc') then
            %loc = default_loc;
        end
    end

    P.D.set = struct('time', D.time, 'values', D.set);
    P.D.Tp_Sense = struct('time', D.time, 'values', D.Tp_Sense);
    P.D.Ta_Sense = struct('time', D.time, 'values', D.Ta_Sense);
    P.D.duty = struct('time', D.time, 'values', D.duty);
    P.D.T = struct('time', D.time, 'values', D.T);
    P.D.OAT = struct('time', D.time, 'values', D.OAT);
    P.D.Ta_Obs = struct('time', D.time, 'values', D.Ta_Obs);
    P.D.err = struct('time', D.time, 'values', D.err);
    P.D.prop = struct('time', D.time, 'values', D.prop);
    P.D.integ = struct('time', D.time, 'values', D.integ);
    P.D.cont = struct('time', D.time, 'values', D.cont);

    P.B.set = struct('time', B.time, 'values', B.set);
    P.B.Tp_Sense = struct('time', B.time, 'values', B.Tp_Sense);
    P.B.Ta_Sense = struct('time', B.time, 'values', B.Ta_Sense);
    P.B.duty = struct('time', B.time, 'values', B.duty);
    P.B.T = struct('time', B.time, 'values', B.T);
    P.B.OAT = struct('time', B.time, 'values', B.OAT);
    P.B.Ta_Obs = struct('time', B.time, 'values', B.Ta_Obs);
    P.B.err = struct('time', B.time, 'values', B.err);
    P.B.prop = struct('time', B.time, 'values', B.prop);
    P.B.integ = struct('time', B.time, 'values', B.integ);
    P.B.cont = struct('time', B.time, 'values', B.cont);

    plot_data(%zoom, %loc + [30 30]);
    //plot_compare(%zoom, %loc + [60 60]);
endfunction

function zoom(%zoom, %loc)
    global figs
    if ~exists('%zoom') then
        %zoom = '';
        %loc = default_loc + [650, 0];
        else if ~exists('%loc') then
            %loc = default_loc + [650, 0];
        end
    end
    plot_all(%zoom, %loc);
endfunction
