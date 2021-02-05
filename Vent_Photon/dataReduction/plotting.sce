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
    overplot(['P.D.prop', 'P.D.integ', 'P.D.cont', 'P.D.pcnt_pot', 'P.D.cmd', 'P.D.duty'], ['r--', 'g--', 'b-', 'b--', 'c-', 'm--'], 'Data cmd', 'time, s', %zoom)
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
    overplot(['P.D.Tp_Sense', 'P.D.set', 'P.D.Ta_Sense', 'P.D.cmd_scaled'], ['r-', 'g-', 'm-', 'k-'], 'Data temperatures', 'time, s', %zoom)

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
    P.D.cmd = struct('time', D.time, 'values', D.cmd);
    P.D.T = struct('time', D.time, 'values', D.T);
    P.D.OAT = struct('time', D.time, 'values', D.OAT);
    P.D.Ta_Obs = struct('time', D.time, 'values', D.Ta_Obs);
    P.D.err = struct('time', D.time, 'values', D.err);
    P.D.prop = struct('time', D.time, 'values', D.prop);
    P.D.integ = struct('time', D.time, 'values', D.integ);
    P.D.cont = struct('time', D.time, 'values', D.cont);
    P.D.pcnt_pot = struct('time', D.time, 'values', D.pcnt_pot);
    P.D.duty = struct('time', D.time, 'values', D.duty);
    P.D.cmd_scaled = struct('time', D.time, 'values', D.cmd_scaled);

    P.B.set = struct('time', B.time, 'values', B.set);
    P.B.Tp_Sense = struct('time', B.time, 'values', B.Tp_Sense);
    P.B.Ta_Sense = struct('time', B.time, 'values', B.Ta_Sense);
    P.B.cmd = struct('time', B.time, 'values', B.cmd);
    P.B.T = struct('time', B.time, 'values', B.T);
    P.B.OAT = struct('time', B.time, 'values', B.OAT);
    P.B.Ta_Obs = struct('time', B.time, 'values', B.Ta_Obs);
    P.B.err = struct('time', B.time, 'values', B.err);
    P.B.prop = struct('time', B.time, 'values', B.prop);
    P.B.integ = struct('time', B.time, 'values', B.integ);
    P.B.cont = struct('time', B.time, 'values', B.cont);
    P.B.pcnt_pot = struct('time', B.time, 'values', B.pcnt_pot);
    P.B.duty = struct('time', B.time, 'values', B.duty);
    P.B.cmd_scaled = struct('time', B.time, 'values', B.cmd_scaled);

    plot_data(%zoom, %loc + [30 30]);
    plot_compare(%zoom, %loc + [60 60]);
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

function plot_heat(%zoom, %loc)
    global figs C
    %size = [610, 460];
    if ~exists('%zoom') then
        %zoom = '';
    end
    if ~exists('%loc') then
        %loc = default_loc + [50, 50];
    end
    figs($+1) = figure("Figure_name", 'Heat Data', "Position", [%loc, %size]);
    subplot(221)
    overplot(['P.C.cmd', 'P.C.cfm'], ['k-', 'b-'], 'Flow', 'time, s', %zoom)
    subplot(222)
    overplot(['P.C.Tp', 'P.C.Tml', 'P.C.Tms', 'P.C.Tdso', 'P.C.Ta', 'P.C.Tw', 'P.C.OAT'], ['k-', 'g--', 'm--', 'k-', 'c-', 'k-', 'g-'], 'Data cmd', 'time, s', %zoom)
    subplot(223)
    overplot(['P.C.Qdli', 'P.C.Qdsi', 'P.C.Qwi', 'P.C.Qwo', 'P.C.QaiMQao'], ['k-', 'b-', 'r-', 'g--', 'k--'], 'Flux', 'time, s', %zoom)
    subplot(224)
    overplot(['P.C.Ta', 'P.C.Tw'], ['b-', 'k-'], 'Duct', 'time, s', %zoom)


    figs($+1) = figure("Figure_name", 'Duct', "Position", [%loc, %size]);
    subplot(221)
    overplot(['P.C.cfm'], ['b-'], 'Flow', 'time, s', %zoom)
    subplot(222)
    overplot(['P.C.Tdso'], ['k-'], 'Tdso', 'time, s', %zoom)
    subplot(223)
    overplot(['P.C.Qdli', 'P.C.Qdlo'], ['k-', 'r-'], 'Flux', 'time, s', %zoom)
    subplot(224)
    overplot(['P.C.Qdsi', 'P.C.Qdso'], ['k-', 'r-'], 'Flux', 'time, s', %zoom)

endfunction


function plot_model(%zoom, %loc)
    global figs C
    %size = [610, 460];
    if ~exists('%zoom') then
        %zoom = '';
    end
    if ~exists('%loc') then
        %loc = default_loc + [50, 50];
    end

    figs($+1) = figure("Figure_name", 'Heat Model', "Position", [%loc, %size]);
    subplot(221)
    overplot(['P.M.cmd', 'P.M.cfm'], ['k-', 'b-'], 'Flow', 'time, s', %zoom)
    subplot(222)
    overplot(['P.M.Tp', 'P.M.Ta', 'P.M.Tw', 'P.M.OAT'], ['r-', 'k-', 'g-', 'b-'], 'Temps', 'time, s', %zoom)
    subplot(223)
    overplot(['P.M.Qai', 'P.M.Qao', 'P.M.Qwi', 'P.M.Qwo'], ['r-', 'g-', 'r-', 'g-'], 'Flux', 'time, s', %zoom)
    subplot(224)
    overplot(['P.M.Ta', 'P.B.Ta_Sense', 'P.M.Tw'], ['k-', 'c--', 'g-'], 'Duct', 'time, s', %zoom)

    figs($+1) = figure("Figure_name", 'Heat Model', "Position", [%loc, %size]);
    subplot(221)
    overplot(['P.M.TaDot', 'P.M.TwDot', 'P.M.TaSdot'], ['k-', 'g-', 'r-'], 'Flow', 'time, s', %zoom)
    subplot(222)
    overplot(['P.M.Ta', 'P.B.Ta_Sense', 'P.M.Tw', 'P.M.Tass', 'P.M.Twss'], ['k-', 'c--', 'g-', 'r--', 'b--'], 'Duct', 'time, s', %zoom)
    subplot(223)
    overplot(['P.M.Qmatch', 'P.M.Qconv'], ['b-', 'g-'], 'Flow', 'time, s', %zoom)

endfunction

function plot_all_heat(%zoom, %loc)
    global figs D C P
    if ~exists('%zoom') then
        %zoom = '';
        %loc = [40, 30];
        else if ~exists('%loc') then
            %loc = default_loc;
        end
    end
    
    P.C.cmd = struct('time', C.time, 'values', C.cmd);
    P.C.hduct = struct('time', C.time, 'values', C.hduct);
    P.C.mdotd = struct('time', C.time, 'values', C.mdotd);
    P.C.cfm = struct('time', C.time, 'values', C.cfm);
    P.C.Tp = struct('time', C.time, 'values', C.Tp);
    P.C.Tbl = struct('time', C.time, 'values', C.Tbl);
    P.C.Tml = struct('time', C.time, 'values', C.Tml);
    P.C.Tbs = struct('time', C.time, 'values', C.Tbs);
    P.C.Tms = struct('time', C.time, 'values', C.Tms);
    P.C.Tdsi = struct('time', C.time, 'values', C.Tdsi);
    P.C.Tdso = struct('time', C.time, 'values', C.Tdso);
    P.C.Ta = struct('time', C.time, 'values', C.Ta);
    P.C.Tw = struct('time', C.time, 'values', C.Tw);
    P.C.OAT = struct('time', C.time, 'values', C.OAT);
    P.C.Qdli = struct('time', C.time, 'values', C.Qdli);
    P.C.Qdlo = struct('time', C.time, 'values', C.Qdlo);
    P.C.Qdsi = struct('time', C.time, 'values', C.Qdsi);
    P.C.Qdso = struct('time', C.time, 'values', C.Qdso);
    P.C.Qai = struct('time', C.time, 'values', C.Qai);
    P.C.Qao = struct('time', C.time, 'values', C.Qao);
    P.C.Qwi = struct('time', C.time, 'values', C.Qwi);
    P.C.Qwo = struct('time', C.time, 'values', C.Qwo);
    P.C.TmlDot = struct('time', C.time, 'values', C.TmlDot);
    P.C.TmsDot = struct('time', C.time, 'values', C.TmsDot);
    P.C.TaDot = struct('time', C.time, 'values', C.TaDot);
    P.C.TwDot = struct('time', C.time, 'values', C.TwDot);
    P.C.QaiMQao = struct('time', C.time, 'values', C.QaiMQao);
    P.C.TaSdot = struct('time', C.time, 'values', C.TaSdot);

    plot_heat(%zoom, %loc + [30 30]);
endfunction

function plot_all_model(%zoom, %loc)
    global figs D C P M
    if ~exists('%zoom') then
        %zoom = '';
        %loc = [40, 30];
        else if ~exists('%loc') then
            %loc = default_loc;
        end
    end
    
    P.M.cmd = struct('time', M.time, 'values', M.cmd);
    P.M.cfm = struct('time', M.time, 'values', M.cfm);
    P.M.OAT = struct('time', M.time, 'values', M.OAT);
    P.M.Qai = struct('time', M.time, 'values', M.Qai);
    P.M.Qao = struct('time', M.time, 'values', M.Qao);
    P.M.Qwi = struct('time', M.time, 'values', M.Qwi);
    P.M.Qwo = struct('time', M.time, 'values', M.Qwo);
    P.M.Ta = struct('time', M.time, 'values', M.Ta);
    P.M.TaDot = struct('time', M.time, 'values', M.TaDot);
    P.M.Tdso = struct('time', M.time, 'values', M.Tdso);
    P.M.Tp = struct('time', M.time, 'values', M.Tp);
    P.M.Tw = struct('time', M.time, 'values', M.Tw);
    P.M.TwDot = struct('time', M.time, 'values', M.TwDot);
    P.M.Tass = struct('time', M.time, 'values', M.Tass);
    P.M.Twss = struct('time', M.time, 'values', M.Twss);
    P.M.Qmatch = struct('time', M.time, 'values', M.Qmatch);
    P.M.Qconv = struct('time', M.time, 'values', M.Qconv);

    P.M.TaSdot = struct('time', M.time, 'values', M.TaSdot);
    P.B.Ta_Sense = struct('time', B.time, 'values', B.Ta_Sense);

    plot_model(%zoom, %loc + [30 30]);
endfunction

function zoom_model(%zoom, %loc)
    global figs
    if ~exists('%zoom') then
        %zoom = '';
        %loc = default_loc + [650, 0];
        else if ~exists('%loc') then
            %loc = default_loc + [650, 0];
        end
    end
    plot_all_model(%zoom, %loc);
endfunction
