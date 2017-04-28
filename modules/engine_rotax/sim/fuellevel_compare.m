% inputs
%  Rfuel = 0 ohm for empty, 190 ohm for 100% full (==50L)
%  Vrd = Rfuel*3.3/(120+Rfuel)
%  ADCfuel = Vrd * 2^12/3.3 = Rfuel*4096/(120+Rfuel)
%  ADCfuel_min = 0
%  ADCfuel_max = 2510
%
% solve for Rfuel:
%  Rfuel = 120*ADCfuel/(4096-ADCfuel)

ADCb = 12; % 12-bit adc values
xmax = 2600;

% x == input vector == ADC value between 0 and ADCfuel_max
x = 0:xmax;

% adc value
%x = y.*(2^ADCb)./(120+y); 
y_fuel_ml = ((120.*x) ./ (4096-x)) * 50000 / 190;



% PIC look up table based calculation
LUTx = [0 360  700  975  1203  1400  1590  1750  1900  2030  2150  2250  2350  2435  2510  2600 ];
LUTy = [0 3000 6470 9820 13100 16370 20000 23533 27300 30990 34870 38445 42490 46260 49950 54880];
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
plot(x,y_fuel_ml,'r',x,outp,'g')

figure(2)
plot(x,y_fuel_ml-outp)