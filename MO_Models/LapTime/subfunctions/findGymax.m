%% Constant turn Model
    % Ce programme donne une estimation de l'acc�leration lat�rale maximale
    % lors d'un virage de rayon constant en fonction des param�tres de la voiture et des pneus
    % Input :   - global : Vehicle_file step
    %           - Turn radius (R_turn)
    %           - Turn angle  (A_turn)

    %Output :   - vectors of speed, time and distance
    
    %Hypothesis :   - tire always at the slip limit
    %               - no suspensions (wtf ?)
 
%% Algo

% Le but du programme est de calculer l'acc�l�ration lat�rale amax � laquelle
% l'�quilibre de la voiture est rompu, c'est-�-dire l'�quilibre en force et en moment.
% La fonction FORCE donne le bilan des forces selon la direction parall�le au rayon du
% virage, amax_force est l'accel�ration a telle que FORCE(a)=0, ce qui correspond au cas
% ou les 2 trains de la voiture d�rapent simultan�ment.
% La fonction torque donne le bilan des moments autour de l'axe vertical,
% amax_force est l'accel�ration a telle que TORQUE(a)=0, ce qui correspond au
% cas ou un des trains de la voiture d�rape (train avant ou arri�re).
% On choisit le minimum de ces deux valeurs et on obtient l'acc�l�ration maximale telle
% que la voiture ne d�rape pas

function [amax,V_turn] = findGymax(R_turn,param_file)
%import parameters :
load(param_file,'xr','W','xf','m_t','g','Tf','Tr','h','Cz','rho','S','FZ','FY','Cz_rep')

% Searching for the maximal lateral acceleration for this turn : 
options = optimset('Algorithm','Levenberg-Marquardt','Display','off');

f1 = @(x) force(x, xr, W, xf, m_t, g, Tf, Tr, h, Cz, rho, S, R_turn, FZ,FY,Cz_rep);
f2 = @(x) force_f(x, xr, W, m_t, g, Tf, h, Cz, rho, S, R_turn, FZ,FY,Cz_rep);
f3 = @(x) force_r(x, W, xf, m_t, g, Tr, h, Cz, rho, S, R_turn, FZ,FY,Cz_rep);


x1 = fsolve(f1,2000,options) ;
x2 = fsolve(f2,2000,options) ;
x3 = fsolve(f3,2000,options) ;
amax = min([abs(x1) abs(x2) abs(x3)]);

%Calcul of the speed associated with this lateral acceleration
V_turn = sqrt(amax*abs(R_turn));
%V_turn = V_turn*3.6; % speed conversion 
end
%% Functions


function [y] = Y(z,FZ,FY)
% force laterale y en fonction de la charge z interpol� par rapport au
% vecteurs FZ et FY qui viennet du mod�le de pneu TTC
    y = interp1(FZ,FY,z,'linear','extrap');
end

% === NOMENCLATURE ===
% xr : distance CG train arri�re
% xf : distance CG train avant
% w : empattement (wheelbase)
% m : masse totale suspendue
% Cz : costante a�rodynamique
% rho : masse volumique air
% S : surface �quivalente v�hicule (plan normale x)
% a_y ; acc�leration lat�rale
% R_turn : rayon nominale du virage
% Cz_rep : r�partition a�ro avant
% Tf : voie (track) avant (front)
% Tr : voie arri�re

function f = Z_0f(xr, w, m, g)
% charge statique sur un pneu du train avant (front)
    f = (xr/w)*m*g/2;
end

function f = Z_0r(xf, w, m, g)
% charge statique sur un pneu du train arri�re (rear)
    f = (xf/w)*m*g/2;
end

function f = Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep)
% aero sur un pneu (V^2 = a* R_turn),
    f = 1/4*Cz*rho*S* abs(a_y)*abs(R_turn) *(1-Cz_rep); 
end

function f = dZ_f(xr, w, m, a_y, h, Tf)
% transfert de charge train avant (front)
    f = (xr/w)*m * a_y*h/Tf; 
end

function f = dZ_r(xf, w, m, a_y, h, Tr)
% transfert de charge train arri�re (rear)
    f = (xf/w)*m * a_y*h/Tr; 
end


function [F] = force(a_y, xr, w, xf, m, g, Tf, Tr, h, Cz, rho, S, R_turn, FZ,FY,Cz_rep)
% force lat�rale r�sultante ? quelle convention pour le rep�re ??
    
    % charge pneu avant ext�rieur
    Z_fe = Z_0f(xr, w, m, g) + dZ_f(xr, w, m, a_y, h, Tf) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep); 
    
    % charge pneu avant int�rieur
    Z_fi = Z_0f(xr, w, m, g) - dZ_f(xr, w, m, a_y, h, Tf) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep); 
    
    % charge pneu arri�re ext�rieur
    Z_re = Z_0r(xf, w, m, g) + dZ_r(xf, w, m, a_y, h, Tr) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep);
    
    % charge pneu arri�re int�rieur
    Z_ri = Z_0r(xf, w, m, g) - dZ_r(xf, w, m, a_y, h, Tr) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep);
    
    % force lat�rale r�sultante
    F = Y(Z_fe,FZ,FY)+Y(Z_fi,FZ,FY)+Y(Z_re,FZ,FY)+Y(Z_ri,FZ,FY) - m*a_y;
end


function [F_f] = force_f(a_y,xr, w, m, g, Tf, h, Cz, rho, S, R_turn, FZ,FY,Cz_rep)
% force r�sultatnte train avant

    % charge pneu avant ext�rieur
    Z_fe = Z_0f(xr, w, m, g) + dZ_f(xr, w, m, a_y, h, Tf) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep); 
    
    % charge pneu avant int�rieur
    Z_fi = Z_0f(xr, w, m, g) - dZ_f(xr, w, m, a_y, h, Tf) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep);
    
    % contribution CG sur le train avant
    Y_f = (xr/w)*m*a_y;
 
    F_f = Y(Z_fe,FZ,FY)+Y(Z_fi,FZ,FY) - Y_f;
end


function [F_r] = force_r(a_y, w, xf, m, g, Tr, h, Cz, rho, S, R_turn, FZ,FY,Cz_rep)
% force r�sultante train arri�re
   
    % charge pneu arri�re ext�rieur
    Z_re = Z_0r(xf, w, m, g) + dZ_r(xf, w, m, a_y, h, Tr) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep);
    
    % charge pneu arri�re int�rieur
    Z_ri = Z_0r(xf, w, m, g) - dZ_r(xf, w, m, a_y, h, Tr) + Z_aero(Cz, rho, S, a_y, R_turn, Cz_rep);
    
    % contribution CG sur train arri�re
    Y_r = (xf/w)*m*a_y;
    
    F_r = Y(Z_re,FZ,FY)+Y(Z_ri,FZ,FY) -Y_r  ;
end 