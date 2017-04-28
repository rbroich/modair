% data from NTC table (temp in degrees C)
x = [42 46 50 55 61 67 75 83 92 103 115 128 144 163 185 210 239 273 314 360 413 476 550 638 742 861 998 1154 1331 1527 1742 1970 2207 2448 2686 2916 3129 3320 3486 3625 3740 3824 3892 3945 3986];
y_h2o_temp = [180 175 170 165 160 155 150 145 140 135 130 125 120 115 110 105 100 95 90 85 80 75 70 65 60 55 50 45 40 35 30 25 20 15 10 5 0 -5 -10 -15 -20 -25 -30 -35 -40];


% PIC look up table based calculation
LUTx = [0   41  72  126 239 360 638 1154 3320 3740 3892 3986 4095 4095 4095 4095];
LUTy = [200 180 150 124 100 84  65  44   -4   -19  -30  -40  -50  -50  -50  -50 ];
NR = length(LUTx);

outp = zeros(1,length(x));
for i=0:length(x)-1
    INVAL = x(1+i);
    OUTVAL = 0;
    if (INVAL <= 0)
        OUTVAL = 0;
    else
        for j=1:NR-1
            if (INVAL <= LUTx(1+j))
                OUTVAL = linear_interpolate(INVAL, LUTx(1+j-1), LUTx(1+j), LUTy(1+j-1), LUTy(1+j));
                break;
            end
        end
    end
    outp(1+i) = OUTVAL;
end

% plot
figure(1)
plot(x,y_h2o_temp,'r',x,outp,'g')
title('Water Temperature: Theoretical (red) vs Interpolation (green)')
xlabel('ADC Value (12-bit)')
ylabel('NTC Temperature (deg C)')

figure(2)
plot(x,y_h2o_temp-outp)