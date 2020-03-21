function [Y_fi, Y_fe, Y_ri, Y_re] = reactions2 (x, R_turn, xr, xf, w, m, Tf, Tr, h, Cz, rho, S, Cz_rep, p_Cy1, p_Dy1, p_Ky1, q)
%REACTIONS-2: utilise la fonction CHARGES et calcule la r�acion laterale
%des pneus en utilisant le mod�le de pneu
% x =  [a_y, SA_f, SA_r]

% calcul des charges
[Z_fi, Z_fe, Z_ri, Z_re] = charges(x(1), R_turn,  xr, xf, w, m, Tf, Tr, h, Cz, rho, S, Cz_rep);

% calcul des r�actions lat�rales des pneus 
Y_fi = simplifiedMFy(Z_fi, x(2), p_Cy1, p_Dy1, p_Ky1, q);
Y_fe = simplifiedMFy(Z_fe, x(2), p_Cy1, p_Dy1, p_Ky1, q);
Y_ri = simplifiedMFy(Z_ri, x(3), p_Cy1, p_Dy1, p_Ky1, q);
Y_re = simplifiedMFy(Z_re, x(3), p_Cy1, p_Dy1, p_Ky1, q);
end
