% ***   Implementation of Optimal-LRC generator matrices for Ceph plugin   ***
% 
% Written by: Oleg Kolosov, Tel Aviv University
% Date: 27/06/2018
% 
% The code for the implementation of USENIX ATC 2018 paper 
% ## On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes ###
% https://www.usenix.org/conference/atc18/presentation/kolosov
% 
% Authors: 
% Oleg Kolosov, School of Electrical Engineering, Tel Aviv University; 
% Gala Yadgar, Computer Science Department, Technion and School of Electrical Engineering, Tel Aviv University; 
% Matan Liram, Computer Science Department, Technion; 
% Itzhak Tamo, School of Electrical Engineering, Tel Aviv University; 
% Alexander Barg, Department of ECE/ISR, University of Maryland

function [new_pol] = ppower (pol, p)
	i=0;
	new_pol = [1];
	while i<p
		new_pol = conv(new_pol,pol);
		i=i+1;
	end
	if p==0
		new_pol=[1];
	end
end
