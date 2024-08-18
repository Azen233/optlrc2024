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


function [g] = g_for_r_5 (n,field,r)

for i=[0:63]
	found=0;
	groups=0;
	base = [];
	val = [];
	bin=de2bi(i);
	bin=fliplr(bin);
	while (length(bin)<= r)
		bin=[0 bin];
    end
	g = [ 1 bin];
	for i=[1:(2^field-1)]
		e= gf (i,field);
		base = [base i];
		val= [val polyval(gf(g,field),e)];
	end
	j=1;
	while(1)
		%count groups of size r+1
		while (sum(val(:) == val(j)) < r+1)
			j=j+1;
			if (j>=length(val))
				break;
			end
		end
		if j<length(val)
			groups = groups+1;
			% remove all found values from array
			val = val(find(val~=val(j)));
		else
			break;
		end
		if (groups == ceil (n/(r+1)))
			return;
		end
	end
end
end
