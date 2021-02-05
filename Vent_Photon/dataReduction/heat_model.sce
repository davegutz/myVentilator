// Heat model
// Tdl temp of large duct mass Mdl, considered to be the muffler box, F
// Tds temp of small duct mass Mds, considered to be the lenght of 6" duct, F
// Rdl coefficient of thermal transfer, BTU/hr/F/ft^2
// Rds coefficient of thermal transfer, BTU/hr/F/ft^2
// Mdl, mass of muffler box, lbm
// Mds, mass of duct, lbm


global figs D C P

// Airflow model ECMF-150 6" duct at 50', bends, filters, muffler box
// see ../datasheets/airflow model.xlsx
function [Qduct, mdot, hf] = flow_model(cmd, rho, mu);
    // cmd      Fan speed, %
    // Pfan     Fan pressure, in H20 at 75F
    // mdotd    Duct = Fan airflow, lbm/hr at 75F
    // hf       Forced convection, BTU/hr/ft^2/F
    // rho      Density, lbm/ft^3
    // mu       Viscosity air, lbm/ft/hr
    Qduct = -0.005153*cmd^2 + 2.621644*cmd;  // CFM
    mdot = Qduct * rho * 60;   // lbm/hr
    Pfan = 5.592E-05*cmd^2 + 3.401E-03*cmd - 2.102E-02;  // in H2O
    d = 0.5;   // duct diameter, ft
    Ax = %pi*d^2/4;             // duct cross section, ft^2
    V = Qduct / Ax * 60;        // ft/hr
    Red = rho * V * d / mu;

    hfi = 1.1;
//    log10hfi = 0.804*log10(Red) - 1.72; 

    hfo = 4.4;
//    log10hfo = 0.804*log10(Red) - 1.12;

    log10hf = 0.804*log10(Red) - 1.72;   // Use smaller to start
    // For data fit, adjust the addend
    hf = max(10^(0.804*log10(Red) - 1.72), hfi);
endfunction

// Will be tuned to match data.   Using physics-based form
// to get the structure correct.   Some numbers are approximate.
function C = heat_proto(debug)
// Major assumptions:
//      Attic temperature same as OAT
//      Fan/duct airflow and pressure instantaneous response to cmd
//      Air at 80F.   Apply 75F models to 80F
//      Uniform mixing of duct air into room
//      Bulk temperatures of air in muffler and duct are the supply temps
//      Neglect radiation (we're only trying to get the shape right, then match)
//      Insulation mass Tb splits the R-value between inside and outside transfer
//      Neglect convection draft from Kitchen that occurs at very low mdot
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
    dt = 10;   // sec time step
    Tp = 80;  // Duct supply, plenum temperature, F
    plotting = 1;
    run_name = 'heat_model';
    
    //pause
    
    // Initialize
    OAT = 30;   // Outside air temperature, F
    cmdi = 90;
    cmdf = 100;
    
    Rsl = 1/hi/Adli + R22/2/Adli + R22/2/Adlo + 1/ho/Adlo;
    Rsli = 1/hi/Adli + R22/2/Adli;
    Rslo = R22/2/Adlo + 1/ho/Adlo;
    Hdso = ho*Adso;
    
    Rsa = 1/hi/Aw + R22/Aw + 1/ho/Aw;
    Rsai = 1/hi/Aw;
    Rsao = R22/Aw + 1/ho/Aw;
    Hai = hi*Aw;
    Hao = ho*Aw;
    
    C = "";
    for time = 0:dt:3600*16
        
        if time<1000 then, cmd = cmdi; else cmd = cmdf;  end
        [cfm, mdotd, hduct] = flow_model(cmd, rhoa, mua);
        Tdli = Tp;
        Tbl = Tp;
        Hdli = hduct*Adli;
        Hdsi = hduct*Adsi;
        Rss = 1/hduct/Adsi + R8/2/Adsi +R8/2/Adso + 1/ho/Adso;
        Rssi = 1/hduct/Adsi + R8/2/Adsi;
        Rsso = R8/2/Adso + 1/ho/Adso;
        Hdlo = ho*Adlo;
    
        // Init logic
        if time<1e-12, then
    
            // Exact solution muffler box
            Tdli = Tp;
            Tbl = (2*mdotd*Cpa*Rsl*Tdli + OAT) / (2*mdotd*Cpa*Rsl + 1);
            Qdl = (Tbl - OAT) / Rsl;
            Tml = Tbl - Qdl * Rsli;
            Tmli = Tbl - Qdl / Hdli;
            Tmlo = OAT + Qdl / Hdlo;
            Tdlo = 2*Tbl - Tdli;
            Qdla = (Tdli - Tdlo)*mdotd*Cpa;  // BTU/hr
                    
            // Exact solution duct
            Tdsi = Tdlo;
            Tbs = (2*mdotd*Cpa*Rss*Tdsi + OAT) / (2*mdotd*Cpa*Rss + 1);
            Qds = (Tbs - OAT) / Rss;
            Tms = Tbs - Qds * Rssi;
    //        Tmsi = Tbs - Qds / Hdsi;
    //        Tmso = OAT + Qds / Hdso;
            Tdso = 2*Tbs - Tdsi;
            Qdsa = (Tdsi - Tdso)*mdotd*Cpa;  // BTU/hr
    
            // Exact solution room
            Ta = (mdotd*Cpa*Rsa*Tdso + OAT) / (mdotd*Cpa*Rsa + 1);
            Qa = (Ta - OAT) / Rsa;
            Tw = Ta - Qa * Rsai;
            Tmai = Ta - Qa / Hai;
            Tmao = OAT + Qa / Hao;
            Qai = Tdso*mdotd*Cpa;
            Qao = Ta*mdotd*Cpa;
    
        end
    
        // Flux
        Tbl = (2*mdotd*Cpa*Rsli*Tdli + Tml) / (2*mdotd*Cpa*Rsli + 1);
        Qdli = (Tbl - Tml) / Rsli;
        Qdlo = (Tml - OAT) / Rslo;
        Tdlo = 2*Tbl - Tdli;
    
        Tdsi = Tdlo;
        Tbs = (2*mdotd*Cpa*Rssi*Tdsi + Tms) / (2*mdotd*Cpa*Rssi + 1);
        Qdsi = (Tbs - Tms) / Rssi;
        Qdso = (Tms - OAT) / Rsso;
        Tdso = 2*Tbs - Tdsi;
    
        Qai = Tdso * Cpa * mdotd;
        Qao = Ta * Cpa * mdotd;
    
        Qwi = (Ta - Tw) / Rsai;
        Qwo = (Tw - OAT) / Rsao;
    
        // Derivatives
        TmlDot = (Qdli - Qdlo)/3600 / (Cpl * Mdl);
        TmsDot = (Qdsi - Qdso)/3600 / (Cps * Mds);
        TaDot = (Qai - Qao - Qwi)/3600 / (Cpa * Mair);
        TwDot = (Qwi - Qwo)/3600 / (Cpw * Mw);
    
        // Store results
        if debug>2 then
            if time==0 then, printf('  time,      cmd,   hduct,   cfm,    mdotd,   Tp,      Tbl,     Tml,     Tbs,     Tms,     Tdso,    Ta,        Tw,      OAT,\n'); end
            printf('%7.1f, %7.1f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f, %7.3f,\n',...
                time, cmd, hduct, cfm, mdotd, Tp, Tbl, Tml, Tbs, Tms, Tdso, Ta, Tw, OAT);
        end
        C.time($+1) = time;
        C.cmd($+1) = cmd;
        C.hduct($+1) = hduct;
        C.cfm($+1) = cfm;
        C.mdotd($+1) = mdotd;
        C.Tp($+1) = Tp;
        C.Tbl($+1) = Tbl;
        C.Tml($+1) = Tml;
        C.Tbs($+1) = Tbs;
        C.Tms($+1) = Tms;
        C.Tdso($+1) = Tdso;
        C.Tdsi($+1) = Tdsi;
        C.Ta($+1) = Ta;
        C.Tw($+1) = Tw;
        C.OAT($+1) = OAT;
        C.Qdli($+1) = Qdli;
        C.Qdlo($+1) = Qdlo;
        C.Qdsi($+1) = Qdsi;
        C.Qdso($+1) = Qdso;
        C.Qai($+1) = Qai;
        C.Qao($+1) = Qao;
        C.Qwi($+1) = Qwi;
        C.Qwo($+1) = Qwo;
        C.TmlDot($+1) = TmlDot;
        C.TmsDot($+1) = TmsDot;
        C.TaDot($+1) = TaDot;
        C.TwDot($+1) = TwDot;
        C.QaiMQao($+1) = Qai-Qao;
        
        // Integrate
        Tml = min(max(Tml + dt*TmlDot, OAT), Tbl);
        Tms = min(max(Tms + dt*TmsDot, OAT), Tbs);
        Ta = min(max(Ta + dt*TaDot, Tw), Tdso);
        Tw = min(max(Tw + dt*TwDot, OAT), Ta);
    
    
    end
endfunction


// Calculate all aspects of heat model
function [a, b, c, e, M] = total_model(time, dt, Tp, OAT, cmd, reset, M, i, B);
// Neglect duct heat effects
// reset    Flag to indicate initialization

    // Inputs
    [cfm, mdot_raw, hduct] = flow_model(cmd, M.rhoa, M.mua);
    
    // Flow filter
    if reset then,
        mdot = mdot_raw;
    else
        delta = mdot_raw - M.mdot(i-1);
        if delta > 0 then d_mdot_dt = (delta)/4500; else d_mdot_dt = delta/4500; end  //(4500 & 4500)
        mdot = M.mdot(i-1) + dt*d_mdot_dt;
    end
    
    // Duct loss
    Tdso = Tp - M.Duct_temp_drop;

    // Initialize
    a = []; b = []; c = []; e = [];
//    tran = 60;
//    Qconv = max(min( (tran - cmd)/tran, 1), 0) * M.Qcon;
    tran = 600; // (360)
    Qconv = max(min( (tran - mdot)/tran, 1), 0) * M.Qcon;
    Qmatch = (B.Ta_Sense(i)*(mdot*M.Cpa*M.Rsa + 1) - (mdot*M.Cpa*M.Rsa*Tdso + OAT - M.Qlk*M.Rsa) ) / M.Rsa;  // Qconv needed to match Ta_Sense to Ta
    Tass = max((mdot*M.Cpa*M.Rsa*Tdso + OAT - M.Qlk*M.Rsa + Qconv*M.Rsa) / (mdot*M.Cpa*M.Rsa + 1), OAT);
    Qaiss = (Tass - OAT) / M.Rsa;
    Twss = max(Tass - Qaiss * M.Rsai, OAT);
    if reset then,
//    if 1 then,
        Ta = Tass;
        Qai = (Tass - OAT) / M.Rsa;
        Qao = Qai;
        Tw = Twss;
    else
        Ta = M.Ta(i-1);
        Tw = M.Tw(i-1);
        Qai = Tdso * M.Cpa * mdot;
        Qao = Ta * M.Cpa * mdot;
    end
    Tmai = Ta - Qai / M.Hai;
    Tmao = OAT + Qao / M.Hao;
    Qwi = (Ta - Tw) / M.Rsai;
    Qwo = (Tw - OAT) / M.Rsao;

    // Derivatives
    TaDot = (Qai - Qao - Qwi - M.Qlk + Qconv)/3600 / (M.Cpa * M.Mair);
    TwDot = (Qwi - Qwo)/3600 / (M.Cpw * M.Mw);

//    
//            if time > -18500 then
//pause
//end
    
    // Integrate
    Ta = min(max(Ta + dt*TaDot, OAT), Tdso);
    Tw = min(max(Tw + dt*TwDot, OAT), Tdso);
    
    // Save / store
    M.time(i) = time;
    M.cmd(i) = cmd;
    M.cfm(i) = cfm;
    M.OAT(i) = OAT;
    M.Qai(i) = Qai;
    M.Qao(i) = Qao;
    M.Qwi(i) = Qwi;
    M.Qwo(i) = Qwo;
    M.Ta(i) = Ta;
    M.TaDot(i) = TaDot;
    M.Tdso(i) = Tdso;
    M.Tp(i) = Tp;
    M.Tw(i) = Tw;
    M.TwDot(i) = TwDot;
    M.Qmatch(i) = Qmatch;
    M.Qconv(i) = Qconv;
    M.mdot(i) = mdot;
    M.Tass(i) = Tass;
    M.Twss(i) = Twss;

end
