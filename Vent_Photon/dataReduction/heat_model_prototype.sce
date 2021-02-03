// Heat model prototype
// Tdl temp of large duct mass Mdl, considered to be the muffler box, F
// Tds temp of small duct mass Mds, considered to be the lenght of 6" duct, F
// Rdl coefficient of thermal transfer, BTU/hr/F/ft^2
// Rds coefficient of thermal transfer, BTU/hr/F/ft^2
// Mdl, mass of muffler box, lbm
// Mds, mass of duct, lbm


clear
clear globals
clear C P D
mclose('all')
funcprot(0);
exec('overplot.sce');
exec('export_figs.sci');
global figs D C P
try close(figs); end
figs=[];
try
    xdel(winsid())
catch
end
default_loc = [40 30];


// Airflow model ECMF-150 6" duct at 50', bends, filters, muffler box
// see ../datasheets/airflow model.xlsx
function [Qduct, mdot, hf] = flow_model(cmd, rho, mu);
    // cmd      Fan speed, %
    // Pfan     Fan pressure, in H20 at 75F
    // mdotd    Duct = Fan airflow, lbm/hr at 75F
    // hf       Forced convection, BTU/hr/ft^2/F
    Qduct = -0.005153*cmd^2 + 2.621644*cmd;  // CFM
    mdot = Qduct * rho * 60;
    Pfan = 5.592E-05*cmd^2 + 3.401E-03*cmd - 2.102E-02;
    d = 0.5;   // duct diameter, ft
    Ax = %pi*d^2/4;             // duct cross section, ft^2
    V = Qduct / Ax * 60;        // ft/hr
    Red = rho * V * d / mu;
    log10hfi = 0.804*log10(Red) - 1.72; hfi = 1.1;
    log10hfo = 0.804*log10(Red) - 1.12; hfo = 4.4;
    log10hf = 0.804*log10(Red) - 1.72;   // Use smaller to start
    // For data fit, adjust the addend
    hf = max(10^(0.804*log10(Red) - 1.72), hfi);
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
    figs($+1) = figure("Figure_name", 'Heat Model', "Position", [%loc, %size]);
    subplot(221)
    overplot(['P.C.cmd', 'P.C.cfm'], ['k-', 'b-'], 'Flow', 'time, s', %zoom)
    subplot(222)
    overplot(['P.C.Tp', 'P.C.Tdl', 'P.C.Tds', 'P.C.Tdo', 'P.C.Ta', 'P.C.OAT'], ['r-', 'r--', 'g--', 'g-', 'c-', 'm-'], 'Data cmd', 'time, s', %zoom)
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

function plot_all(%zoom, %loc)
    global figs D C P
    if ~exists('%zoom') then
        %zoom = '';
        %loc = [40, 30];
        else if ~exists('%loc') then
            %loc = default_loc;
        end
    end
    
    P.C.cmd = struct('time', C.time, 'values', C.cmd);
    P.C.cfm = struct('time', C.time, 'values', C.cfm);
    P.C.Tp = struct('time', C.time, 'values', C.Tp);
    P.C.Tdl = struct('time', C.time, 'values', C.Tdl);
    P.C.Tds = struct('time', C.time, 'values', C.Tds);
    P.C.Tdo = struct('time', C.time, 'values', C.Tdo);
    P.C.Ta = struct('time', C.time, 'values', C.Ta);
    P.C.OAT = struct('time', C.time, 'values', C.OAT);

    plot_heat(%zoom, %loc + [30 30]);
endfunction


// This model will be tuned to match data.   Using physics-based form
// to get the structure correct.   Some numbers are approximate.
// Major assumptions:
//      Attic temperature same as OAT
//      Fan/duct airflow and pressure instantaneous response to cmd
//      Air at 80F.   Apply 75F models to 80F
//      Uniform mixing of duct air into room
//      Bulk temperatures of air in muffler and duct are the supply temps
//      Neglect radiation (we're only trying to get the shape right, then match)
Aw = 300;   // Surface area room ws and ceiling, ft^2
Adli = 9;   // Surface area inner muffler box, ft^ft
Adlo = 16;  // Surface area inner muffler box, ft^ft
Adsi = 40;  // Surface area inner duct, ft^ft
Adso = 150; // Surface area inner duct, ft^ft
Cpa = 0.23885; // Heat capacity of dry air at 80F, BTU/lbm/F (1.0035 J/g/K)
Cpl = 0.2;  // Heat capacity of muffler box, BTU/lbm/F
Cps = 0.4;  // Heat capacity of duct insulation, BTU/lbm/F
Cpw = 0.2;  // Heat capacity yof ws, BTU/lbm/F
Mw = 2000;  // Mass of room ws and ceiling, lbm
Mdl = 50;   // Mass of muffler box, lbm
Mds = 100;  // Mass of duct, lbm
hf = 1;     // TBD.  Rdf = hf*log10(mdot);    // Boundary layer heat resistance forced convection, BTU/hr/ft^2/F
            // NOTE:  probably need step data from two different flow conditions to triangulate this value.
hi = 1.4;   // Heat transfer resistance inside still air, BTU/hr/ft^2/F.  Approx industry avg
ho = 4.4;   // Heat transfer resistance outside still air, BTU/hr/ft^2/F.  Approx industry avg
rhoa = .0739;    // Density of dry air at 80F, lbm/ft^3
mua = 0.04379;  // Viscosity air, lbm/ft/hr
Mair = 8*12*12 * rhoa; // Mass of air in room, lbm

//mdotd     Mass flow rate of air through duct, lbm/sec
// Pfan     Fan pressure, in H20 at 75F, assume = 80F
// Qduct    Duct = Fan airflow, CFM at 75F, assume = 80F
// Tp       Plenum temperature, F
// Ta       Sunshine Room temperature, F
// Tdo      Duct discharge temp, F
// Tbs      Small duct bulk air temp, F
// Tbl      Muffler bulk air temp, F


// Rdf model:
//  R ~ 2 - 100  BTU/hr/ft^2/F

// Loop for time transient
dt = 2;   // sec time step
Tp = 80;  // Duct supply, plenum temperature, F
plotting = 1;
debug = 2;
run_name = 'heat_model';

//pause

// Initialize
OAT = 30;   // Outside air temperature, F
Tdl = 75;   // Muffler box temp, F
Tds = 75;   // Duct wall temp, F
Tw = 50;    // House wall temp F
Ta = 65;    // Air temp, F
cmdi = 1;
cmdf = 1;
//for time = 0:dt:3600
C = "";
for time = 0:dt:14
    if time<10 then, cmd = cmdi; else cmd = cmdf;  end
    [cfm, mdotd, hduct] = flow_model(cmd, rhoa, mua);

    // Flux
    Tbl = Tp;
    Qdli = (Tbl - Tdl) / hduct / Adli;
    Qdlo = (Tdl - OAT) / hi / Adlo;
    Tbs = Tp - Qdli / mdotd / Cpa;
    Qdsi = (Tbs - Tds) / hduct / Adsi;
    Qdso = (Tds - OAT) / hi / Adso;  // Attic is still air but cold
    Qwi = (Ta - Tw) / hi / Aw;
    Qwo = (Tw - OAT) / ho / Aw;
    Tdo = Tp - (Qdsi + Qdli) / mdotd / Cpa;
    Qai = Tdo * Cpa * mdotd;
    Qao = Ta * Cpa * mdotd;

    // Derivatives
    TdsDot = (Qdsi - Qdso) * Cps * Mds;
    TdlDot = (Qdli - Qdlo) * Cpl * Mdl;
    TwDot = (Qwi - Qwo) * Cpw * Mw;
    TaDot = (Qai - Qao) * Cpa * Mair;

    // Store results
    if time==0 then, printf('  time,      cmd,   hduct,   cfm,    mdotd,   Tp,      Tds,     Tdl,     Tdo,    Ta,        Tw,      OAT,\n'); end
    printf('%7.1f, %7.1f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f,\n',...
        time, cmd, hduct, cfm, mdotd, Tp, Tds, Tdl, Tdo, Ta, Tw, OAT);
    C.time($+1) = time;
    C.cmd($+1) = cmd;
    C.hduct($+1) = hduct;
    C.cfm($+1) = cfm;
    C.mdotd($+1) = mdotd;
    C.Tds($+1) = Tds;
    C.Tdl($+1) = Tdl;
    C.Tw($+1) = Tw;
    C.Ta($+1) = Ta;
    C.Tdo($+1) = Tdo;
    C.OAT($+1) = OAT;
    C.Tp($+1) = Tp;
    
    // Integrate
    Tds = Tds + dt*TdsDot;
    Tdl = Tdl + dt*TdlDot;
    Ta = min(max(Ta + dt*TaDot, Tw), Tp);
    Tw = min(max(Tw + dt*TwDot, OAT), Tp);


end

// Plots
// Zoom last buffer
C.N = size(C.time, 1);
last = C.N;
if plotting then
    zoom([0 C.time(last)])
    if debug>1 then
        plot_all()
    end
end

mclose('all')

export_figs(figs, run_name)



