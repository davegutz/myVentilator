// Serial print functions
function serial_print_inputs(k)

    printf('%s,%18.3f,   %4.1f,%7.3f,%7.3f,', TD.hmString(k+1), D.time(k+1), D.set(k+1), D.Tp_Sense(k+1), D.Ta_Sense(k+1));

    mfprintf(doubtfd, '%s,%18.3f,   %4.1f,%7.3f,%7.3f,', TD.hmString(k+1), D.time(k+1), D.set(k+1), D.Tp_Sense(k+1), D.Ta_Sense(k+1));

endfunction


function serial_print(k)

    printf('%d,   %5.2f,%4.1f,%7.3f,  %7.3f,%7.3f,%7.3f,%7.3f,', D.duty(k+1), D.T(k+1), D.OAT(k+1), D.Ta_Obs(k+1), D.err(k+1), D.prop(k+1), D.integ(k+1), D.cont(k+1));

    mfprintf(doubtfd, '%d,   %5.2f,%4.1f,%7.3f,  %7.3f,%7.3f,%7.3f,%7.3f,', D.duty(k+1), D.T(k+1), D.OAT(k+1), D.Ta_Obs(k+1), D.err(k+1), D.prop(k+1), D.integ(k+1), D.cont(k+1));

endfunction


function serial_print_inputs_1()
    for k=0:size(D.set,1)-2
        serial_print_inputs(k);
        serial_print(k);
        printf('\n');
        mfprintf(doubtfd, '\n');
    end
end
