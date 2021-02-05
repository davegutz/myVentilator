


// Inline Exponential rate-lag
function [rate, lstate, rstate] = my_exp_rate_lag_inline(in, tau, T, ...
                                     rstate, lstate, MIN, MAX, RESET)
    eTt = exp(-T/tau);
    a = (tau/T) - eTt/(1-eTt);
    b = (1/(1-eTt)) - (tau/T);
    c = (1-eTt)/T;
    if RESET then
        rstate = in;
        lstate = in;
        rate = 0;
    else
        rate = c*( a*rstate + b*in - lstate);
        rate = min(max(rate, MIN), MAX);
        rstate = in;
        lstate = rate*T + lstate;
    end
endfunction

mdrate = zeros(100,1);
lstate = zeros(100,1);
rstate = zeros(100,1);
mdot = zeros(100,1);
dt = 0.01;
time = zeros(100,1);
for j = 1:100,
    time(j) = (j-1)*dt;
    if time(j) <0.05 then mdot(j) = .9; else  mdot(j) = mdot(j-1)+.01; end
    if j==1, then reset = %t; else reset = %f; end
    if reset then,
        [mdrate(j), lstate(j), rstate(j)] = my_exp_rate_lag_inline(mdot(j), 0.1, dt, ...
                                     0, 0, -%inf, %inf, reset)
    else
        [mdrate(j), lstate(j), rstate(j)] = my_exp_rate_lag_inline(mdot(j), 0.1, dt, ...
                                     rstate(j-1), lstate(j-1), -10, 10, reset)
    end
end
figure; plot(time, [mdot mdrate lstate, rstate]);
    
