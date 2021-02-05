// heat_model_constants.sce
function M = heat_model_define()
    M.Aw = 12*12 + 7*12*3;   // Surface area room walls and ceiling, ft^2
    M.Adli = 9;   // Surface area inner muffler box, ft^ft
    M.Adlo = 16;  // Surface area inner muffler box, ft^ft
    M.Adsi = 40;  // Surface area inner duct, ft^ft
    M.Adso = 150/2; // Surface area inner duct, ft^ft  (half buried)
    M.Cpa = 0.23885; // Heat capacity of dry air at 80F, BTU/lbm/F (1.0035 J/g/K)
    M.Cpl = 0.2;  // Heat capacity of muffler box, BTU/lbm/F
    M.Cps = 0.4;  // Heat capacity of duct insulation, BTU/lbm/F
    M.Cpw = 0.2;  // Heat capacity of walls, BTU/lbm/F
    M.Mw = 1000;  // Mass of room ws and ceiling, lbm (1000)
    M.Mdl = 50;   // Mass of muffler box, lbm (50)
    M.Mds = 20;  // Mass of duct, lbm (100)
    M.hf = 1;     // TBD.  Rdf = hf*log10(mdot);    // Boundary layer heat resistance forced convection, BTU/hr/ft^2/F
                // NOTE:  probably need step data from two different flow conditions to triangulate this value.
    M.hi = 1.4;   // Heat transfer resistance inside still air, BTU/hr/ft^2/F.  Approx industry avg
    M.ho = 4.4;   // Heat transfer resistance outside still air, BTU/hr/ft^2/F.  Approx industry avg
    M.rhoa = .0739;    // Density of dry air at 80F, lbm/ft^3
    M.mua = 0.04379;  // Viscosity air, lbm/ft/hr
    M.Mair = 8*12*12 * M.rhoa; // Mass of air in room, lbm
    
    M.R8 = 45;  // Resistance of R8 duct insulation, F-ft^2/BTU
    // 5.68 F-ft^2/(BTU/hr).   R8 = 8*5.68 = 45 F-ft^2/(BTU/hr)
    M.R16 = 90;  // Resistance of R8 duct insulation, F-ft^2/(BTU/hr)
    M.R22 = 125;  // Resistance of R22 wall insulation, F-ft^2/(BTU/hr)
    M.R32 = 180;  // Resistance of R22 wall insulation, F-ft^2/(BTU/hr)
    M.R64 = 360;  // Resistance of R22 wall insulation, F-ft^2/(BTU/hr)
    M.Duct_temp_drop = 7;  // Observed using infrared thermometer, F (7)
    M.Qlk = 800;    // Model alignment heat loss, BTU/hr
    M.Qcon = (M.Qlk + 104) *.9;  // Model alignment heat gain when cmd = 0, BTU/hr.   ***Don't know why factor of 2 needed.
//    M.Qcon = (M.Qlk + 104);  // Model alignment heat gain when cmd = 0, BTU/hr.   ***Don't know why factor of 2 was needed.

    M.Rsa = 1/M.hi/M.Aw + M.R22/M.Aw + 1/M.ho/M.Aw;
    M.Rsai = 1/M.hi/M.Aw;
    M.Rsao = M.R22/M.Aw + 1/M.ho/M.Aw;
    M.Hai = M.hi*M.Aw;
    M.Hao = M.ho*M.Aw;
endfunction

function M = heat_model_init(M)
    M.time = zeros(B.N, 1);
    M.cmd = zeros(B.N, 1);
    M.cfm = zeros(B.N, 1);
    M.OAT = zeros(B.N, 1);
    M.Qai = zeros(B.N, 1);
    M.Qao = zeros(B.N, 1);
    M.Qwi = zeros(B.N, 1);
    M.Qwo = zeros(B.N, 1);
    M.Ta = zeros(B.N, 1);
    M.TaDot = zeros(B.N, 1);
    M.Tdso = zeros(B.N, 1);
    M.Tp = zeros(B.N, 1);
    M.Tw = zeros(B.N, 1);
    M.TwDot = zeros(B.N, 1);
    M.Qmatch = zeros(B.N, 1);
    M.Qconv = zeros(B.N, 1);
    M.mdot = zeros(B.N, 1);
    M.Tass = zeros(B.N, 1);
    M.Twss = zeros(B.N, 1);
endfunction
