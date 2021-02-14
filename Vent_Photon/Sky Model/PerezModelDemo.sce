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


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function phip = pixelAzimuthAngle(thetac, phic, fc, up, vp)
//%   Converts pixel coordinates to azimuth angles.
//% 
//% Input parameters:
//%  - thetac: camera zenith angle (in radians)
//%  - phic: camera azimuth angle (in radians)
//%  - fc: camera focal length
//%  - up: x-coordinates of pixels in image
//%  - vp: y-coordinates of pixels in image
//%
//% Output parameters:
//%  - phip: pixel azimuth angle
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function phip = pixelAzimuthAngle(thetac, phic, fc, up, vp)
    phip = atan((sin(phic).*sin(thetac).*fc-cos(phic).*up-sin(phic).*cos(thetac).*vp),(cos(phic).*sin(thetac).*fc+sin(phic).*up-cos(phic).*cos(thetac).*vp));
end
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function thetap = pixelZenithAngle(thetac, fc, up, vp)
//%   Converts pixel coordinates to zenith angles.
//% 
//% Input parameters:
//%  - thetac: camera zenith angle (in radians)
//%  - fc: camera focal length
//%  - up: x-coordinates of pixels in image
//%  - vp: y-coordinates of pixels in image
//%
//% Output parameters:
//%  - thetap: pixel zenith angle
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function thetap = pixelZenithAngle(thetac, fc, up, vp)
    thetap = acos((sin(thetac).*vp+cos(thetac).*fc)./(fc.^2+up.^2+vp.^2).^(1/2));
end

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function M = getTurbidityMapping(channelId)
//%  Returns the turbidity mapping between turbidity and the constants
//%  (a,b,c,d,e) from the Perez sky model. Taken from [Preetham et al., SIGGRAPH '99].
//% 
//% Input parameters:
//%  - channelId: [1,3] channel in the xyY space
//%
//% Output parameters:
//%  - M: turbidity mapping. [a b c d e] = M*[T 1]'
//%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function M = getTurbidityMapping(channelId)
    switch channelId
       case 1 //% x
            M = [-0.0193 -0.2592; -0.0665 0.0008; -0.0004 0.2125; -0.0641 -0.8989; -0.0033 0.0452];
            
        case 2 //% y     
            M = [-0.0167 -0.2608; -0.0950 0.0092; -0.0079 0.2102; -0.0441 -1.6537; -0.0109 0.0529];
            
        case 3 //% Y
            M = [0.1787 -1.4630; -0.3554 0.4275; -0.0227 5.3251; 0.1206 -2.5771; -0.0670 0.3703];
            
        otherwise
            error('input channelID must be between 1 and 3!');
    end
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function skyParams = convertTurbidityToSkyParams(turbidity, k)
//%  Converts turbidity to sky parameters. Also appends
//%  the scale factors at the end, if given. 
//% 
//% Input parameters:
//%  - turbidity
//%  - [k]: scale factors for each channels
//%
//% Output parameters:
//%  - skyParams: parameters (a,b,c,d,e), 6x3 (if k specified), 5x3 otherwise
//%   
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function skyParams = convertTurbidityToSkyParams(turbidity, k)
    if nargin > 1, then nbParams = 6; else nbParams = 5; end

    skyParams = zeros(nbParams, 3);
    for ch=1:3
        skyParams(1:5,ch) = getTurbidityMapping(ch)*[turbidity 1]';
    end

    if nargin > 1, then
        skyParams(6,:) = k(:)';
    end
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function lum = perezGradientModel(a, b, theta)
//%  Synthesizes the sky according to the Perez sky model, the
//%  gradient-dependent part only.
//% 
//% Input parameters:
//%  - a, b: the 2 gradient-related weather coefficients
//%  - theta: zenith angle of sky element
//%
//% Output parameters:
//%  - lum: luminance map (same dimensions as theta)
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function lum = perezGradientModel(a, b, theta)
lum = 1 + a.*exp(b./cos(theta));
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function lum = perezSunModel(c, d, e, theta, gamma)
//%  Synthesizes the sun contribution to the sky model
//% 
//% Input parameters:
//%  - c, d, e: the 3 sun-related weather coefficients
//%  - theta: zenith angle of sky element
//%  - gamma: angular difference between sky element and sun
//%
//% Output parameters:
//%  - lum: luminance map (same dimensions as theta, gamma)
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function lum = perezSunModel(c, d, e, gamma)
    lum = 1 + c.*exp(d.*gamma) + e.*cos(gamma).^2;
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function lum = perezSkyModel(a, b, c, d, e, theta, gamma)
//%  Synthesizes the sky according to the Perez sky model.
//% 
//% Input parameters:
//%  - a, b, c, d, e: the 5 weather coefficients
//%  - theta: zenith angle of sky element
//%  - gamma: angular difference between sky element and sun
//%
//% Output parameters:
//%  - lum: luminance map (same dimensions as theta, gamma)
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function lum = perezSkyModel(a, b, c, d, e, theta, gamma)
    lum = perezGradientModel(a, b, theta) .* perezSunModel(c, d, e, gamma);
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function lum = exactSunModel(c, d, e, f, up, vp, vh, phi, phiSun, thetaSun)
//%  Synthesizes the sun contribution to the sky model
//% 
//% Input parameters:
//%  - c, d, e: Perez sky model parameters
//%  - f: camera focal length (in pixels)
//%  - up: x-coordinates of pixels in image
//%  - vp: y-coordinates of pixels in image
//%  - theta: camera zenith angle
//%  - phi: camera azimuth angle (in radians)
//%  - phiSun: sun azimuth angle (in radians)
//%  - thetaSun: sun zenith angle (in radians)
//%
//% Output parameters:
//%  - lum: luminance map in image coordinates (same dimensions as up and up)
//%   
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function lum = exactSunModel(c, d, e, f, up, vp, theta, phi, phiSun, thetaSun)
    phic = phi;
    thetap = pixelZenithAngle(theta, f, up, vp);
    phip = pixelAzimuthAngle(theta, phic, f, up, vp);
    deltaPhi = abs(phiSun - phip);
    gamma = acos(cos(thetaSun) .* cos(thetap) + sin(thetaSun) .* sin(thetap) .* cos(deltaPhi));
    
    //% plug in Perez sky model
    lum = perezSunModel(c, d, e, gamma);
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function lum = exactGradientModel(a, b, f, up, vp, vh)
//%  Synthesizes the sky gradient part only.
//% 
//% Input parameters:
//%  - a, b: Perez sky model parameters
//%  - f: camera focal length (in pixels)
//%  - up: x-coordinates of pixels in image
//%  - vp: y-coordinates of pixels in image
//%  - theta: camera zenith angle
//%
//% Output parameters:
//%  - lum: luminance map in image coordinates (same dimensions as up and vp)
//% 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function lum = exactGradientModel(a, b, f, up, vp, theta)
    //% convert to camera parameters
    //% thetac = %pi/2+atan2(vh, f);
    thetap = pixelZenithAngle(theta, f, up, vp);
    
    //% plug in Perez sky model
    lum = perezGradientModel(a, b, thetap);
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function lum = exactSkyModel(a, b, c, d, e, f, up, vp, vh, phi, phiSun, thetaSun)
//%  Synthesizes the full sky model.
//% 
//% Input parameters:
//%  - a, b, c, d, e: Perez sky model parameters
//%  - f: camera focal length (in pixels)
//%  - up: x-coordinates of pixels in image
//%  - vp: y-coordinates of pixels in image
//%  - theta: camera zenith angle
//%  - phi: camera azimuth angle (in radians)
//%  - phiSun: sun azimuth angle (in radians)
//%  - thetaSun: sun zenith angle (in radians)
//%
//% Output parameters:
//%  - lum: luminance map in image coordinates (same dimensions as up and up)
//%   
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function lum = exactSkyModel(a, b, c, d, e, f, up, vp, theta, phi, phiSun, thetaSun)
    //% product of sun and gradient luminance
    //% vh = tan(theta-%pi/2).*f;
    lum = exactGradientModel(a, b, f, up, vp, theta) .* exactSunModel(c, d, e, f, up, vp, theta, phi, phiSun, thetaSun);
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%  function ratio = exactSkyModelRatio(a, b, c, d, e, f, up, vp, vh, lz, phi, phiSun, thetaSun)
//%    Synthesizes the full sky model, based on ratios of luminances
//% 
//% Input parameters:
//%  - a, b, c, d, e: Perez sky model parameters
//%  - f: camera focal length (in pixels)
//%  - up: x-coordinates of pixels in image
//%  - vp: y-coordinates of pixels in image
//%  - theta: camera zenith angle
//%  - lz: zenith luminance
//%  - phi: camera azimuth angle (in radians)
//%  - phiSun: sun azimuth angle (in radians)
//%  - thetaSun: sun zenith angle (in radians)
//%
//% Output parameters:
//%  - ratio: ratio of luminances (with respect to zenith)
//%   
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function ratio = exactSkyModelRatio(a, b, c, d, e, f, up, vp, lz, theta, phi, phiSun, thetaSun)
    ratio = lz .* exactSkyModel(a, b, c, d, e, f, up, vp, theta, phi, phiSun, thetaSun) ./ perezSkyModel(a, b, c, d, e, 0, thetaSun);
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% function reconstructedImg = reconstructSkyFromFullModel(imgDims, params, vh, f, phiCam, thetaSun, phiSun)
//%  Synthesizes the sky according to the input parameters.
//% 
//% Input parameters:
//%  - imgDims: dimensions of the output image [height width nbChannels]
//%  - params: the weather coefficients (6xnbChannels, rows = a,b,c,d,e,k)
//%  - vh: horizon line (where 0 = center of the image, pointing up)
//%  - f: focal length of the camera
//%  - phiCam: camera azimuth angle
//%  - thetaSun: sun zenith angle
//%  - phiSun: sun azimuth angle
//%
//% Output parameters:
//%  - reconstructedImg: rendered sky (in the xyY color space)
//%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function reconstructedImg = reconstructSkyFromFullModel(imgDims, params, f, thetaCam, phiCam, thetaSun, phiSun)
    imgHeight = imgDims(1);
    imgWidth = imgDims(2);
    nbChannels = imgDims(3);
    [uRange, vRange] = meshgrid(1:imgWidth, 1:imgHeight);
    upVec = (uRange - imgWidth/2) - 0.5;
    vpVec = (imgHeight/2 - vRange) + 0.5;
    //% vhImg = floor(imgHeight/2 - vh + 0.5);
    //% synthesize each channel
    reconstructedImg = zeros(imgHeight, imgWidth, nbChannels);
    for ch = 1:nbChannels
        reconstructedImg(:,:,ch) = exactSkyModelRatio(params(1,ch), params(2,ch), params(3,ch), params(4,ch), params(5,ch), ...
            f, upVec, vpVec, params(6,ch), thetaCam, phiCam, phiSun, thetaSun);
    end
    vhImg = floor(imgHeight/2 - tan(thetaSun-%pi/2).*f+0.5);
    
    //% black out what's below the horizon
    reconstructedImg(vhImg:$,:,:) = 0;
end


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%  Example code which demonstrates the use of the sky model.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if %t then
        imgDims = [480 640 3]; //% 640x480 color image
        k = [0.2 0.2 0.05];
        skyParams = convertTurbidityToSkyParams(2.2, k); //% t=2.2, clear sky
        //% vh = -100; % looking up
        f = 800; //% focal length in pixels
        phiCam = 0; //% facing north
        
        thetaSun = %pi/2-(10*%pi/180); //% 10 degrees to horizon
        thetaCam = thetaSun; //% straight at the sun
        
        //% synthesize the sky with the sun behind the camera
        phiSun = %pi; 
        skySunBehind = reconstructSkyFromFullModel(imgDims, skyParams, f, thetaCam, phiCam, thetaSun, phiSun);
        
        //% synthesize the sky with the sun in front of the camera
        phiSun = 0;
        skySunFront = reconstructSkyFromFullModel(imgDims, skyParams, f, thetaCam, phiCam, thetaSun, phiSun);
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
// n        Day of year
// ET       Equation of time, minutes
// B        Input to Equation of Time
// delta    Declination angle, sun's angle with equator at noon, deg.   +is N, -is S
// E0       Eccentricity correction factor
// gamma    Solar azimuth angle, displacement from south, deg, -is E, +is W
// I0       Hourly extraterrestrial Radiation during hours time interval, W/m^2
// Ib       Direct beam solar radiation, W/m^2
// IbN      Normal beam solar radiation, W/m^2
// Id       Diffuse solar radiation, W/m^2
// IH       Total horizontal surface, W/m^2
// Isc      Solar constant 1367 W/m^2
// IV       Total vertical surface, W/m^2
// Ls       Standard meridian for the local zone, through middle of zone, deg = -longitude, +is W, -is E
// Ll       Meridian at location, deg = -longitude, +is W, -is E
// LT       Local standard time, decimal hours
// phi      Latitude, deg, +is North, -is South
// ST       Local solar time, decimal hours
// omega    Hour angle before noon, deg.  +is morning.  -is afternoon
// theta_z  Zenith angle, deg
right_now = getdate();
yy = right_now(1); mm = right_now(2); dd = right_now(6);
hr = right_now(7);mn = right_now(8);
LT = hr + mn/60;
GMT = -5;
// 10 Howard St, Wenham, MA 01984
Ll = 70.88361;
%phi = 42.61172;
phi_r = %phi*%pi/180;
Ls = 75;
n = get_yday(mm, dd, yy);
delta = delta_gamma(n);
delta_r = delta*%pi/180;
B = 360*(n-81)/364;
B_r = B*%pi/180;
ET = 9.87*sin(2*B_r) - 7.53*cos(B_r) - 1.5*cos(B_r);
ST = LT + ET/60 + 4/60*(Ls-Ll);
omega = 15*(12-ST);
omega_r = omega*%pi/180;
%gamma = -omega;
Isc = 1367;
E0 = 1 + 0.0033*cos(2*%pi*n/365);  // Beckman simple approximation

omega1=0; omega2 = 15;omega1_r = omega1*%pi/180; omega2_r = omega2*%pi/180;
I0 = 12*3.5/%pi*Isc*E0*((sin(phi_r)*cos(delta_r))*(sin(omega2_r)-sin(omega1_r)) + ...
        (omega2_r-omega1_r)*(sin(phi_r)*sin(delta_r)));        

theta_z = acosd(sin(delta_r)*sin(phi_r) + cos(delta_r)*cos(phi_r)*cos(omega_r));
theta_z_r = theta_z*%pi/180;
omega_w_r = 0; // South facing wall

// Show that delta approx may be ok for you, look at plot and decide
del = zeros(1, 365);
del_x = zeros(1, 365);
for i = 1:365,
    del(i) =delta_gamma(i);
    del_x(i) = delta_gamma_approx(i);
end
//figure;plot([del' del_x'])

H = 15*(12-ST);  //  I think there is an error in this
H_r = %pi/180*H;  //  I think there is an error in this
BetaS = 180/%pi * asin( cos(phi_r)*cos(delta_r)*cos(H_r)  +  sin(phi_r)*sin(delta_r) );
BetaS_r = BetaS*%pi/180;
Azimuth = 180/%pi * asin(cos(delta_r)*sin(H_r)/cos(BetaS_r));
if cos(H_r)>=tan(delta_r)/tan(phi_r) then
    phiS = Azimuth;
else
    if LT<12 then,
        phiS = Azimuth;
    else
        phiS = 180 - Azimuth;
    end
end
phiC = 0; // Collector aximuth angle, deg faces due south
phiC_r = phiC*%pi/180;
sigma = 90; // Collector tilt angle, deg  vertical wall
sigma_r = sigma*%pi/180;
phiS_r = phiS*%pi/180;
BetaN = 90 - %phi + delta;
BetaN_r = BetaN*%pi/180;
//=COS(RADIANS(Angles!F11))*COS(RADIANS(0-Insolation!L6))*SIN(RADIANS(Insolation!L7))+SIN(RADIANS(Angles!F11))*COS(RADIANS(Insolation!L7))
theta_r =  acos( cos(BetaN_r)*cos(0-phiC_r)*sin(sigma_r) + sin(BetaN_r)*cos(sigma_r) );
cosTheta = cos(theta_r);
Ibc = cosTheta*Ib;
A = 1160+75*sin(360/365*(n-275)*%pi/180);
k = 0.174+0.035*sin(360/365*(n-100)*%pi/180);
C = 0.095+0.04*sin(360/365*(n-100)*%pi/180);
m = abs(1/sin(BetaN_r));
IB = A*exp(-k*m);
IBC = IB*cosTheta;

//Id = C*IbN; // ???????
//IH = IbN * cos(theta_z_r) + Id;
//IV = IbN * cos(%pi/2-theta_z_r) * cos(omega_w_r - omega_r) + Id;


