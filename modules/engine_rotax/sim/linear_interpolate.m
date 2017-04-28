function [y] = linear_interpolate(x, x0, x1, y0, y1)

y = round(y0 + (y1-y0)*(x-x0)/(x1-x0));

end