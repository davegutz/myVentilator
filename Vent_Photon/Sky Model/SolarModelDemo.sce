clear
clear globals
mclose('all')
funcprot(0);
try close(figs); end
figs=[];
try
    xdel(winsid())
catch
end

// Is leap year
function is_leap_yr = yisleap(year)
    is_leap_yr = (modulo(year,4)==0 & modulo(year,100)~=0) | (modulo(year,400)==0);
end


// Get day of year
function yday = get_yday(mon, day, year)
    days = [ ...
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334;
        0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335;
    ];
    leap = yisleap(year)+1;
    yday = days(leap, mon) + day;
end


function [In, I] = sky(Ll, Ls, %phi_r, phiC_r, sigma_r, ref, yy, mm, dd, hr, mn)
//  Sun angles
// Best nomenclature in energies-10-00134-v2.pdf:  Maleki, et.al. "Estimation of Hourly, Daily and Monthly Global Solar Radiation on Inclined Surfaces:  Models Re-Visited", 2016
// A            Apparent Extraterrestrial Solar Insolation, W/m^2
// ET       Equation of time, minutes
// B_r      Longitude correction, input to Equation of Time, rad
// CT       Local standard time, decimal hours
// delta    Declination angle, sun's angle with equator at noon, deg.   +is N, -is S
// E0       Eccentricity correction factor.  Beckman simple approximation
// gamma    Solar azimuth angle, displacement from south, deg, -is E, +is W
// I0       Hourly extraterrestrial Radiation during hours time interval, W/m^2
// IB       Clear sky beam radiation at Earth, W/m^2
// IBC      Beam insolation on surface or collector, W/m^2
// IC       Total insolation o surface or collector, W/m^2
// Id       Diffuse solar radiation, W/m^2
// IDC      Diffuse radiation, W/m^2
// IH       Total horizontal surface, W/m^2
// IRC      Reflected radiation, W/m^2
// Isc      Solar constant 1367 W/m^2
// IV       Total vertical surface, W/m^2
// Ls       Standard meridian for the local zone, through middle of zone, deg = -longitude, +is W, -is E
// Ll       Meridian at location, deg = -longitude, +is W, -is E
// m        Air mass ratio
// julian   Day of year
// omega    Hour angle before noon, deg.  +is morning.  -is afternoon
// phi      Latitude, deg, +is North, -is South
// phiC     Collector aximuth angle, deg faces due south
// ref      reflectance of surface  (user input)
// sigma        Collector tilt angle, deg  vertical wall
// ST       Local solar time, decimal hours
// theta_r  Zenith angle, rad
// Angles
    julian = get_yday(mm, dd, yy);
    //    delta = 23.45*sin((360/365*(julian-81))*%pi/180);
    //    delta_r = delta*%pi/180;
    delta_r = 0.40928*sin(0.0172142*(julian-81));
    delta = delta_r*180/%pi;
    //    B_r = (julian-81)/364*2*%pi;
    B_r = 0.0172615*(julian-81);
    I.ET = 9.87*sin(2*B_r) - 7.53*cos(B_r) - 1.5*sin(B_r);
    In.ET = I.ET;
    In.DT = hr + mn/60;
    I.DT = In.DT;
    In.ST = (In.DT*60 + I.ET + 4*(Ls-Ll))/60;
    I.ST = In.ST;
    //    omega = 15*(12-I.ST); omega_r = omega*%pi/180;
    //    %gamma = -omega;
    //    E0 = 1 + 0.0033*cos(2*%pi*julian/365);    
    //    H = 15*(12-I.ST);
    //    H_r = %pi/180*H;
    H_r = 0.2617994*(12-I.ST);
    //    BetaS = 180/%pi * asin( cos(phi_r)*cos(delta_r)*cos(H_r)  +  sin(phi_r)*sin(delta_r) );
    //    BetaS_r = BetaS*%pi/180;
    BetaS_r = asin( cos(phi_r)*cos(delta_r)*cos(H_r)  +  sin(phi_r)*sin(delta_r) );
    Azimuth = 180/%pi * asin(cos(delta_r)*sin(H_r)/cos(BetaS_r));
    if cos(H_r)>=tan(delta_r)/tan(phi_r) then
        phiS = Azimuth;
    else
        if CT<12 then,
            phiS = Azimuth;
        else
            phiS = 180 - Azimuth;
        end
    end
    phiS_r = phiS*%pi/180;
    BetaN = 90 - %phi + delta;
    BetaN_r = BetaN*%pi/180;
    A = 1160+75*sin(360/365*(julian-275)*%pi/180);   
    k = 0.174+0.035*sin(360/365*(julian-100)*%pi/180);
    C = 0.095+0.04*sin(360/365*(julian-100)*%pi/180);
    In.m = abs(1/sin(BetaN_r));
    I.m = abs(1/sin(BetaS_r));
    
    // At noon
    In.thetan_r =  acos( cos(BetaN_r)*cos(0-phiC_r)*sin(sigma_r) + sin(BetaN_r)*cos(sigma_r) );
    In.cosThetan = cos(In.thetan_r);
    In.IB = A*exp(-k*In.m);  
    In.IBC = In.IB*In.cosThetan; 
    In.IDC = C*In.IB*(1+cos(sigma_r))/2;
    In.IRC = ref*In.IB*(sin(BetaN_r)+C)*(1-cos(sigma_r))/2;  
    In.IC = In.IBC + In.IDC + In.IRC;  
    
//    printf("ST = %7.3f\ndelta=%7.3f\nH = %7.4f\nBetaS = %7.3f\nAzimuth = %7.3f\nm = %7.3f\nA = %7.3f\nk = %7.3f\nC = %7.3f\nm(noon) = %7.3f\nIB = %7.3f\nm = %7.3f\n", I.ST, delta, H, BetaS, Azimuth, m, A, k, C, In.m, In.IB, I.m);

    // At time
    I.theta_r =  acos( cos(BetaS_r)*cos(phiS_r-phiC_r)*sin(sigma_r) + sin(BetaS_r)*cos(sigma_r) );
    I.cosTheta = cos(I.theta_r);
    if BetaS_r>0 then
        I.IB = A*exp(-k*I.m);  
    else
        I.IB = 0;
    end
    if I.cosTheta<0 then,
        I.IBC = 0;
    else
        I.IBC = I.IB*I.cosTheta;
    end
    I.IDC = C*I.IB*(1+cos(sigma_r))/2;
    I.IRC = ref*I.IB*(sin(BetaS_r)+C)*(1-cos(sigma_r))/2;  
    I.IC = I.IBC + I.IDC + I.IRC;  
endfunction



// User inputs
// 10 Howard St, Wenham, MA 01984
Ll = 70.88361;
Ls = 75;
%phi = 42.61172; phi_r = %phi*%pi/180;
// Surface
phiC = 0;  phiC_r = phiC*%pi/180;
sigma = 90; sigma_r = sigma*%pi/180;
omega_w_r = 0; // South facing wall
ref = 0.2;  

// Sun
Isc = 1367;

// Date and time
right_now = getdate();
yy = right_now(1); mm = right_now(2); dd = right_now(6);
hr = right_now(7);mn = right_now(8);
CT = hr + mn/60;
//[In, I] = sky(Ll, Ls, phi_r, phiC_r, sigma_r, ref, yy, mm, dd, hr, mn);
[In, I] = sky(Ll, Ls, phi_r, phiC_r, sigma_r, ref, 2021, 2, 15, 9, 22);

T = [];
IC = [];
for mm = 1:12
    for hr = 1:24,
        for mn = 1:5:60;
            [In, I] = sky(Ll, Ls, phi_r, phiC_r, sigma_r, ref, 2021, mm, 15, hr, mn);
            T($+1) = I.DT;
            IC($+1) = I.IC;
        end
    end
end
figs($+1) = figure("Figure_name", 'Sol');plot(T, IC)




