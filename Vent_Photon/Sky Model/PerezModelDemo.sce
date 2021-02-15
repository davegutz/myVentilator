funcprot(0)

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


// Sun declination from day of year
// delta    Declination angle, sun's angle with equator at noon, deg.   +is N, -is S
// Gamma    Day angle, radians
function delta_deg = delta_gamma(n)
    Gamma = 2*%pi*(n-1)/365;  // radians
    delta_deg = (0.006918 - 0.399912*cos(Gamma) + 0.070257*sin(Gamma) - ...
        0.006758*cos(2*Gamma) + 0.000907*sin(2*Gamma) - 0.002697*cos(3*Gamma) ...
         + 0.00148*sin(3*Gamma)) * 180/%pi; // deg
endfunction
function delta_deg = delta_gamma_approx(n)
    delta_deg = -23.43*cos((n+9.5)*2*%pi/365);
endfunction

//  Sun angles
// Best nomenclature in energies-10-00134-v2.pdf:  Maleki, et.al. "Estimation of Hourly, Daily and Monthly Global Solar Radiation on Inclined Surfaces:  Models Re-Visited", 2016
// A            Apparent Extraterrestrial Solar Insolation, W/m^2
// ET       Equation of time, minutes
// B        Input to Equation of Time
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
// n        Day of year
// omega    Hour angle before noon, deg.  +is morning.  -is afternoon
// phi      Latitude, deg, +is North, -is South
// phiC     Collector aximuth angle, deg faces due south
// ref      reflectance of surface  (user input)
// sigma        Collector tilt angle, deg  vertical wall
// ST       Local solar time, decimal hours
// theta_r  Zenith angle, rad

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
n = get_yday(mm, dd, yy);

// Angles
delta = delta_gamma_approx(n); delta_r = delta*%pi/180;
B = 360*(n-81)/364; B_r = B*%pi/180;
ET = 9.87*sin(2*B_r) - 7.53*cos(B_r) - 1.5*cos(B_r);
ST = CT + ET/60 + 4/60*(Ls-Ll);
omega = 15*(12-ST); omega_r = omega*%pi/180;
%gamma = -omega;
E0 = 1 + 0.0033*cos(2*%pi*n/365);
//theta_z = acosd(sin(delta_r)*sin(phi_r) + cos(delta_r)*cos(phi_r)*cos(omega_r));  theta_z_r = theta_z*%pi/180;

// Example of sector
//omega1=0; omega2 = 15;omega1_r = omega1*%pi/180; omega2_r = omega2*%pi/180;
//I0 = 12*3.5/%pi*Isc*E0*((sin(phi_r)*cos(delta_r))*(sin(omega2_r)-sin(omega1_r)) + ...
//        (omega2_r-omega1_r)*(sin(phi_r)*sin(delta_r)));        

H = 15*(12-ST);  //  I think there is an error in this
H_r = %pi/180*H;  //  I think there is an error in this
BetaS = 180/%pi * asin( cos(phi_r)*cos(delta_r)*cos(H_r)  +  sin(phi_r)*sin(delta_r) ); BetaS_r = BetaS*%pi/180;
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
BetaN = 90 - %phi + delta;  BetaN_r = BetaN*%pi/180;
theta_r =  acos( cos(BetaN_r)*cos(0-phiC_r)*sin(sigma_r) + sin(BetaN_r)*cos(sigma_r) );
cosTheta = cos(theta_r);
A = 1160+75*sin(360/365*(n-275)*%pi/180);   
k = 0.174+0.035*sin(360/365*(n-100)*%pi/180);
C = 0.095+0.04*sin(360/365*(n-100)*%pi/180);
m = abs(1/sin(BetaN_r));   
IB = A*exp(-k*m);  
IBC = IB*cosTheta; 
IDC = C*IB*(1+cos(sigma_r))/2; 
IRC = ref*IB*(sin(BetaN_r)+C)*(1-cos(sigma_r))/2;  
IC = IBC + IDC + IRC;  
