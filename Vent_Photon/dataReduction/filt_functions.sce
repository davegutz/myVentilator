// Filtering functions

// Sort x(size) in ascending order
// i, j 0-based, x is 1-based
function x = my_sort_ascend(x, %size) 
// Sort array in asclast order (insertion sort algorithm)
  i = int32(0);  j = int32(0);  n = double(0);
  for i = 1:%size-1
    n = x(i+1);
    j = i;
    while j>0 & n<x(j-1+1)
        x(j+1) = x(j);
        j = j-1;
    end
    x(j+1) = n;
  end
endfunction

    
// Vectorial Exponential rate-lag
function [rate, lstate, rstate] = my_exp_rate_lag(in, tau, T, ...
                                            MIN, MAX, RESET)
    N = size(in, 1);
    eTt = exp(-T/tau);
    a = (tau./T) - eTt./(1 - eTt);
    b = ((1)./(1 - eTt)) - (tau./T);
    c = (1 - eTt)./T;
    rate = zeros(N, 1);
    lstate = zeros(N, 1);
    rstate = zeros(N, 1);
    for i=2:N,
        if RESET(i-1) then
            lstate(i-1) = in(i-1);
            rstate(i-1) = in(i-1);
            rate(i-1) = 0;
        end
        rate(i) = c(i)*( a(i)*rstate(i-1) + b(i)*in(i) - lstate(i-1));
        rate(i) = min(max(rate(i), MIN), MAX);
        rstate(i) = in(i);
        lstate(i) = rate(i)*T(i) + lstate(i-1);
    end
endfunction

// Vectorial Tustin rate-lag
function [rate, state] = my_tustin_rate_lag(in, tau, T, ...
                            MIN, MAX, MIN_STATE, MAX_STATE, RESET)
    N = size(in, 1);
    a = (2*ones(N,1) ./ (2*tau + T));
    b = (2*tau - T) ./ (2*tau + T);
    rate = zeros(N, 1);
    for i=2:N,
        if RESET(i-1) then
            state(i-1) = min(max(in(i-1), MIN_STATE), MAX_STATE);
            rate(i-1) = 0;
        end
        rate(i) = a(i)*(in(i) - state(i-1));
        state(i) = in(i)*(1-b(i)) + state(i-1)*b(i);
        state(i) = min(max(state(i), MIN_STATE), MAX_STATE);
        rate(i) = min(max(rate(i), MIN), MAX);
    end
endfunction

// Vectorial Tustin lag
function [state, state] = my_tustin_lag(in, tau, T, ...
                                                MIN, MAX, RESET)
    N = size(in, 1);
    a = (2*ones(N,1) ./ (2*tau + T));
    b = (2*tau - T) ./ (2*tau + T);
    for i=2:N,
        if RESET(i-1) then
            state(i-1) = in(i-1);
        end
        state(i) = in(i)*(1-b(i)) + state(i-1)*b(i);
        state(i) = min(max(state(i), MIN), MAX);
    end
endfunction

// Inline Tustin lag
function [filt, state] = my_tustin_lag_inline(in, tau, T, state, ...
                                                MIN, MAX, RESET)
    a =  2 / (2*tau + T);
    b = (2*tau - T) / (2*tau + T);
    if RESET then
        state = in;
    else
        state = in*(1-b) + state*b;
        state = min(max(state, MIN), MAX);
    end
    filt = state;
endfunction

function [res, hr_sd_ratio] = my_hr_and_o2_sat_filt(%time_us, ...
            %ir, %samp_size, %red, %ir_filt)
// Calculate the heart rate and SpO2 level
// By detecting  peaks of PPG cycle and corresponding AC/DC of
// red/infra-red signal, the ratio for the SPO2 is computed.
// Since this algorithm is aiming for Arm M0/M3. formaula for SPO2 did
// not achieve the accuracy due to register overflow.  Thus, accurate 
// SPO2 is precalculated and save longo uch_spo2_table[] per each
// ratio.
//
// Inputs
// *ir      - IR sensor data buffer
// size_buf - IR sensor data buffer length
// *red     - Red sensor data buffer
// *ir_filt - IR sensor data buffer filtered through bandpass
//
// Outputs
// *spo2    - Calculated SpO2 value
// *spo2_valid         - 1 if the calculated SpO2 value is valid
// *hr      - Calculated heart rate value
// *hr_valid           - 1 if the calculated heart rate value is valid
//
    // Emulate MEGA2560 and Photon
    ir_buf = int32(zeros(%samp_size,1));
    ir = uint32(%ir);
    size_buf = int32(%samp_size);
    red = uint32(%red);  
    k = int32(0);
    ratio_num = int32(0);
    i = int32(0);
    n_exact_ir_valley_locs_c = int32(0);
    n_middle_idx = int32(0);
    thresh = int32(0);
    num_peaks = int32(0); 
    locs_valley = int32(zeros(MAX_NUM_PEAKS, 1));
    sum_peak_intervals = double(0);
    red_ac = int32(0);
    ir_ac = int32(0);
    spo2_calc  = int32(0);
    red_dc_max  = int32(0);
    ir_dc_max = int32(0);
    red_dc_max_idx = int32(0);
    ir_dc_max_idx = int32(0); 
    ratio = double(zeros(MAX_NUM_PEAKS-1,1));
    pi = double(zeros(MAX_NUM_PEAKS-1,1));
    ratio_median = double(0); 
    pi_median = double(0);
    hr = double(0);
    hr_sd_ratio = double(0);
    time_calc_us = %time_us(size_buf);
    time_ref_us = %time_us(1);
    samples_per_sec = double(size_buf-1) / max(double(time_calc_us-time_ref_us)*1e-6, 1e-6);

    // Calculate threshold
    for k = 1:%samp_size
     thresh = thresh + abs(%ir_filt(k));
    end
    thresh = thresh / max(%samp_size, 1);
    thresh = min(max(thresh, MIN_THRESHOLD_FILT), MAX_THRESHOLD_FILT);

    // since we flipped signal, we use peak detector as valley detector
    for k = 0:MAX_NUM_PEAKS-1; locs_valley(k+1) = 0; end
    [locs_valley, num_peaks] = my_find_peaks(locs_valley, num_peaks, ...
           %ir_filt, %samp_size, thresh, PEAK_DISTANCE, MAX_NUM_PEAKS,time_calc_us);
    sum_peak_intervals = 0;
    if debug>=3 then
        printf('3,0,%ld, 0, %d, %5.2f,  %d, ',...
            time_calc_us, thresh, samples_per_sec, num_peaks);
        mfprintf(doubtfd, '3,0,%ld, 0, %d, %5.2f,  %d, ',...
            time_calc_us, thresh, samples_per_sec, num_peaks);
    end
    if num_peaks>=MIN_NUM_PEAKS;
        for k=1:num_peaks-1
            sum_peak_intervals = sum_peak_intervals + ...
                double(locs_valley(k+1) -locs_valley(k-1+1));
        end
        sum_peak_intervals = sum_peak_intervals / double(num_peaks-1);
        sd_peak_intervals = 0;
        for k=1:num_peaks-1
            sd_peak_intervals = sd_peak_intervals + ...
                (double(locs_valley(k+1) - locs_valley(k-1+1)) - ...
                sum_peak_intervals)^2;
        end
        sd_peak_intervals = sqrt( double(sd_peak_intervals)/double(num_peaks-2) );
        if debug>=3 then
            printf('%5.2f, %6.3f, [', sum_peak_intervals, sd_peak_intervals);
            mfprintf(doubtfd, '%5.2f, %6.3f, [', sum_peak_intervals, sd_peak_intervals);
            for k = 0:num_peaks-1
                printf('  [%2d,', locs_valley(k+1));             
                printf('%d],', -%ir_filt(locs_valley(k+1)+1));
                mfprintf(doubtfd, '  [%2d,', locs_valley(k+1));             
                mfprintf(doubtfd, '%d],', -%ir_filt(locs_valley(k+1)+1));
            end
        end
        hr = double( (samples_per_sec*60)/ sum_peak_intervals );
        hr_valid = 1;
        hr_sd_ratio = min( (double(sum_peak_intervals) / ...
                                max(sd_peak_intervals, 1e-3)), 100);
    else
        hr = -999; // unable to calculate because # of peaks too small
        hr_valid = 0;
    end
    if debug>=3 then
        if num_peaks>=MIN_NUM_PEAKS then
            printf(' ], %d,%d,%d,\n', hr, hr_valid, hr_sd_ratio);
            mfprintf(doubtfd, ' ], %d,%d,%d,\n', hr, hr_valid, hr_sd_ratio);
        else
            printf(' %d,%d,%d,\n', hr, hr_valid, hr_sd_ratio);
            mfprintf(doubtfd, ' %d,%d,%d,\n', hr, hr_valid, hr_sd_ratio);
        end
    end
    
    /////////////////////////////////////////////spo2//////////////////
    spo2 = double(100); spo2_valid = int8(1);
    // Load buffer again for spo2
    ir_buf = int32(%ir);
    red_buf = int32(%red);


    ////////////////////////////////////////////************************
    // calculate DC mean and subtract DC from ir
    measize_buf = 0; 
    for k=0:size_buf-1; measize_buf = measize_buf + ir_buf(k+1) ; end
    measize_buf =measize_buf/size_buf ;
    ir_buf_inv = -1*(ir_buf - measize_buf);
   // Calculate threshold
    for k = 0:%samp_size-1
     thresh = thresh + abs(ir_buf_inv(k+1));
    end
    thresh = thresh / %samp_size;
    thresh = min(max(thresh, MIN_THRESHOLD), MAX_THRESHOLD);
    // Find precise min near locs_valley
    // since we flip signal (-ir_buf), we use peak detector as valley detector
    for k = 0:MAX_NUM_PEAKS-1; locs_valley(k+1) = 0; end
    [locs_valley, count_ir_valleys_exact] = my_find_peaks(locs_valley, num_peaks, ...
           ir_buf_inv, %samp_size, thresh, PEAK_DISTANCE, MAX_NUM_PEAKS, time_calc_us);
           
    printf("1,1,%d,0, %d, %4.2f, %d, [", int(time_calc_us), thresh, samples_per_sec, count_ir_valleys_exact);
    if (count_ir_valleys_exact>0) then
        for k=0:count_ir_valleys_exact-1,
            printf("  [ %d,%d ] ,", locs_valley(k+1), -ir_buf_inv(locs_valley(k+1)+1));
        end
    end
    printf("  ]\n");

    ////////////////////////////////////////////************************


    // using exact_ir_valley_locs , find ir-red DC and ir-red AC for 
    // SPO2 calibration ratio
    // finding AC/DC maximum of raw
    // find max between two valley locations 
    // and use ratio betwen AC compoent of Ir & Red and DC compoent 
    // of Ir & Red for SPO2 
    if spo2_valid==1 then
        if debug>=3 then
            printf('3,1,%d,%d,----, [, ', time_calc_us, hr);
            mfprintf(doubtfd, '3,1,%d,%d,----, [, ', time_calc_us, hr);
        end
        for k=0:count_ir_valleys_exact-1-1
            red_dc_max= -16777216; 
            ir_dc_max= -16777216;
            if locs_valley(k+1+1)-locs_valley(k+1)>=PEAK_DISTANCE then
                for i=locs_valley(k+1):locs_valley(k+1+1)-1
                    if ir_buf(i+1)>ir_dc_max then
                        ir_dc_max =ir_buf(i+1);
                        ir_dc_max_idx = i;
                    end
                    if red_buf(i+1)>red_dc_max then
                        red_dc_max = red_buf(i+1);
                        red_dc_max_idx = i;
                    end
                end
                
                // red ac
                red_ac = ( red_buf(locs_valley(k+1+1)+1) - ...
                           red_buf(locs_valley(k+1)+1)  ) * ...
                         ( red_dc_max_idx - locs_valley(k+1) ); //red
                red_ac = red_buf(locs_valley(k+1)+1) + ...
                            red_ac / (locs_valley(k+1+1) - ...
                            locs_valley(k+1)); 
                // subracting linear DC compoenents from
                red_ac = red_buf(red_dc_max_idx+1) - red_ac;
            
                // ir ac
                ir_ac = ( ir_buf(locs_valley(k+1+1)+1) - ...
                          ir_buf(locs_valley(k+1)+1)  ) * ...
                        ( ir_dc_max_idx - locs_valley(k+1) ); // ir
                ir_ac = ir_buf(locs_valley(k+1)+1) + ...
                            ir_ac / (locs_valley(k+1+1) - ...
                            locs_valley(k+1)); 
                // subracting linear DC compoenents from
                ir_ac = ir_buf(ir_dc_max_idx+1) - ir_ac;
                red_dc = red_dc_max - red_ac/2;
                ir_dc = ir_dc_max - ir_ac/2;
                numel = int32(int64(red_ac)* int64(ir_dc)) / 128;
                denom = int32(int64(ir_ac) * int64(red_dc))/ 128;
                
                //formula is (red_ac *ir_dc) / (ir_ac *red_dc);

//         if time_calc_us==71512729 then
//             disp (locs_valley(k+1+1), locs_valley(k+1), numel, denom)
//             pause
//         end
                if denom>0  && ratio_num<MAX_NUM_PEAKS-1 &&  numel>0 then  // dag 12/21/2020 ~=0 to >0
                    ratio(ratio_num+1) = double(numel) / double(denom);
                    pi(ratio_num+1) = double(ir_ac) / double(ir_dc) * 100; // Perfusion Index
                    ratio_num = ratio_num + 1;
                    ratio_ = ratio(ratio_num);
                    float_SPO2_ = -45.060*ratio_* ratio_ + 30.354 *ratio_ + 94.845;
                    if debug==2 then
                        printf('\nred_ac=%6.0f, red_dc=%9.0f, ir_ac=%6.0f, ir_dc=%9.0f, ratio=%6.2f,float_SPO2=%6.2f, ratio_=%6.2f, float_SPO2_=%6.2f\n',...
                        red_ac, red_dc, ir_ac, ir_dc, ratio(ratio_num), float_SPO2_, ratio_, float_SPO2_)
                    end
                    if debug>=3 then
                        printf('[,%d,%d, %d, %d, %d,%d, %d,%d, [%d,%d], [%d,%d], [%d,%d], [%d,%d], %d,%d, %6.3f,%6.3f,], ', ...
                        ratio_num-1, locs_valley(k+1), red_dc_max_idx, ir_dc_max_idx, ...
                        red_ac, ir_dc, ...
                        ir_ac, red_dc, ...
                        locs_valley(k+1),   red_buf(locs_valley(k+1)+1), ...
                        locs_valley(k+1+1), red_buf(locs_valley(k+1+1)+1), ...
                        locs_valley(k+1),   ir_buf(locs_valley(k+1)+1), ...
                        locs_valley(k+1+1), ir_buf(locs_valley(k+1+1)+1), ...
                        numel, denom, ...
                        ratio(ratio_num-1+1), pi(ratio_num-1+1));
                        mfprintf(doubtfd, '[,%d,%d, %d, %d, %d,%d, %d,%d, [%d,%d], [%d,%d], [%d,%d], [%d,%d], %d,%d, %6.3f,%6.3f,], ', ...
                        ratio_num-1, locs_valley(k+1), red_dc_max_idx, ir_dc_max_idx, ...
                        red_ac, ir_dc, ...
                        ir_ac, red_dc, ...
                        locs_valley(k+1),   red_buf(locs_valley(k+1)+1), ...
                        locs_valley(k+1+1), red_buf(locs_valley(k+1+1)+1), ...
                        locs_valley(k+1),   ir_buf(locs_valley(k+1)+1), ...
                        locs_valley(k+1+1), ir_buf(locs_valley(k+1+1)+1), ...
                        numel, denom, ...
                        ratio(ratio_num-1+1), pi(ratio_num-1+1));
                    end
                end
            end  // if > 3
        end // for count_ir_valleys_exact-1
    end // spo2_valid
    if debug>=3 then
        printf(' ]\n');
        mfprintf(doubtfd, ' ]\n');
    end
    // choose median value since PPG signal varies from beat to beat
    ratio_sort = my_sort_ascend(ratio, ratio_num);
    index_mid= int(ratio_num/2);
    if index_mid>1 then   // use median
        ratio_median =(ratio_sort(index_mid-1+1) + ratio_sort(index_mid+1)) / 2;
    else
        ratio_median = ratio_sort(index_mid+1);
    end
    pi_sort = my_sort_ascend(pi, ratio_num);
    index_mid= int(ratio_num/2);
    if index_mid>1 then   // use median
        pi_median =(pi_sort(index_mid-1+1) + pi_sort(index_mid+1)) / 2;
    else
        pi_median = pi_sort(index_mid+1);
    end

    // Very simple fault screening
    if ratio_median>RATIO_MIN && ratio_median <RATIO_MAX && hr_valid==1 && ratio_num>=MIN_NUM_PEAKS then
        ratio_lim = max(ratio_median, BEER_BREAK);
        // from MAXIM:  spo2 = -45.060*ratio_lim*ratio_lim + 30.354*ratio_lim + 94.845;
        spo2 = min(BEER_GN*ratio_lim + BEER_BIAS, 100);
        spo2_valid = 1;
    else
        spo2_valid = 0; 
        spo2 = 0; // do not use SPO2 since signal ratio is out of range
        pi_median = 0;
        ratio_median = 0;
    end
    if debug>=3 then
        printf('3,2,%d,%6.3f, %d, %6.3f, %6.3f, %1d,%1d, [, [,', time_calc_us, ...
            pi_median, hr, ratio_median, spo2, count_ir_valleys_exact, ratio_num);
        mfprintf(doubtfd, '3,2,%d,%6.3f, %d, %6.3f, %6.3f, %1d,%1d, [, [,', time_calc_us, ...
            pi_median, hr, ratio_median, spo2, count_ir_valleys_exact, ratio_num);
        for k = 0:ratio_num-1
            printf('%5.3f, ', ratio_sort(k+1));             
            mfprintf(doubtfd, '%5.3f, ', ratio_sort(k+1));             
        end
        printf('], [,');
        mfprintf(doubtfd, '], [,');
        for k = 0:ratio_num-1
            printf('%5.3f, ', pi_sort(k+1));             
            mfprintf(doubtfd, '%5.3f, ', pi_sort(k+1));             
        end
        printf('], ], %6.3f,%6.3f\n', spo2, pi_median);
        mfprintf(doubtfd, '], ], %6.3f,%6.3f\n', spo2, pi_median);
    end


//         if time_calc_us==28604042 then
//             pause
//         end
    

    // Return
    res = list(spo2, spo2_valid, hr, hr_valid, pi_median, ratio_median);
endfunction  // my_hr_and_o2_sat_filt

