// heat_model_constants.sce

Aw = 12*12 + 7*12*3;   // Surface area room walls and ceiling, ft^2
Adli = 9;   // Surface area inner muffler box, ft^ft
Adlo = 16;  // Surface area inner muffler box, ft^ft
Adsi = 40;  // Surface area inner duct, ft^ft
Adso = 150/2; // Surface area inner duct, ft^ft  (half buried)
Cpa = 0.23885; // Heat capacity of dry air at 80F, BTU/lbm/F (1.0035 J/g/K)
Cpl = 0.2;  // Heat capacity of muffler box, BTU/lbm/F
Cps = 0.4;  // Heat capacity of duct insulation, BTU/lbm/F
Cpw = 0.2;  // Heat capacity of walls, BTU/lbm/F
Mw = 1000;  // Mass of room ws and ceiling, lbm (1000)
Mdl = 50;   // Mass of muffler box, lbm (50)
Mds = 20;  // Mass of duct, lbm (100)
hf = 1;     // TBD.  Rdf = hf*log10(mdot);    // Boundary layer heat resistance forced convection, BTU/hr/ft^2/F
            // NOTE:  probably need step data from two different flow conditions to triangulate this value.
hi = 1.4;   // Heat transfer resistance inside still air, BTU/hr/ft^2/F.  Approx industry avg
ho = 4.4;   // Heat transfer resistance outside still air, BTU/hr/ft^2/F.  Approx industry avg
rhoa = .0739;    // Density of dry air at 80F, lbm/ft^3
mua = 0.04379;  // Viscosity air, lbm/ft/hr
Mair = 8*12*12 * rhoa; // Mass of air in room, lbm

R8 = 45;  // Resistance of R8 duct insulation, F-ft^2/BTU
// 5.68 F-ft^2/(BTU/hr).   R8 = 8*5.68 = 45 F-ft^2/(BTU/hr)
R16 = 90;  // Resistance of R8 duct insulation, F-ft^2/(BTU/hr)
R22 = 125;  // Resistance of R22 wall insulation, F-ft^2/(BTU/hr)
R32 = 180;  // Resistance of R22 wall insulation, F-ft^2/(BTU/hr)
R64 = 360;  // Resistance of R22 wall insulation, F-ft^2/(BTU/hr)
