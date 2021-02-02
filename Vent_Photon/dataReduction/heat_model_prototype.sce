// Heat model prototype
// Tdl temp of large duct mass Mdl, considered to be the muffler box, F
// Tds temp of small duct mass Mds, considered to be the lenght of 6" duct, F
// Rdl coefficient of thermal transfer, BTU/hr/F/ft^2
// Rds coefficient of thermal transfer, BTU/hr/F/ft^2
// Mdl, mass of muffler box, lbm
// Mds, mass of duct, lbm


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

// Airflow model ECMF-150 6" duct at 50', bends, filters, muffler box
// see ../datasheets/airflow model.xlsx
function [Pfan, Qduct] = flow_model(cmd);
    // cmd      Fan speed, %
    // Pfan     Fan pressure, in H20 at 75F
    // Qduct    Duct = Fan airflow, CFM at 75F
    Qduct = -0.005153*cmd^2 + 2.621644*cmd;
    Pfan = y = 5.592E-05*cmd^2 + 3.401E-03*cmd - 2.102E-02;
endfunction

global figs D C P
try close(figs); end
figs=[];
try
    xdel(winsid())
catch
end

// This model will be tuned to match data.   Using physics-based form
// to get the structure correct.   Some numbers are approximate.
// Major assumptions:
//      Attic temperature same as OAT
//      Fan/duct airflow and pressure instantaneous response to cmd
//      Air at 80F.   Apply 75F models to 80F
Mdl = 50;   // Mass of muffler box, lbm
Mds = 100;  // Mass of duct, lbm
Ma = 2000;  // Mass of room walls and ceiling, lbm
Aa = 300;   // Surface area room walls and ceiling, ft^2
Cpl = 0.2;  // Heat capacity of muffler box, BTU/lbm/F
Cps = 0.4;  // Heat capacity of duct insulation, BTU/lbm/F
Adli = 9;   // Surface area inner muffler box, ft^ft
Adsi = 40;  // Surface area inner duct, ft^ft
Adlo = 16;  // Surface area inner muffler box, ft^ft
Adso = 150; // Surface area inner duct, ft^ft
Rdf = hf*log10(mdot);    // Boundary layer heat resistance forced convection, BTU/hr/ft^2/F
ho = 4.4;   // Heat transfer resistance outside still air, BTU/hr/ft^2/F.  Approx industry avg
hi = 1.4;   // Heat transfer resistance inside still air, BTU/hr/ft^2/F.  Approx industry avg
OAT = 30;   // Outside air temperature, F
Cpa = ;     // Heat capacity of dry air at 80F, BTU/lbm/F
Rhoa = .0739;    // Density of dry air at 80F, lbm/ft^3
mdotd = ;    // Mass flow rate of air through duct, lbm/sec
// Pfan     Fan pressure, in H20 at 75F, assume = 80F
// Qduct    Duct = Fan airflow, CFM at 75F, assume = 80F
// Tp       Plenum temperature, F
// Ta       Sunshine Room temperature, F

// Loop for time transient
dt = 2;   // sec time step
for time = 0:dt:3600
    if time<10 then, cmd = 50; else cmd = 100; end
    [Pfan, Qduct] = flow_model(cmd);
    mdotd = Qduct * Rhoa;
    
    
    
    qd = mdotd * Cpa * (Tp - Ta);
end

