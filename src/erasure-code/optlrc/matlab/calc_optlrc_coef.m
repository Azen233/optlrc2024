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

function [coef] = calc_optlrc_coef (A,r,field,n,locations)
j=0;
groups=floor(n/(r+1));
rows=size(A,2);
%for r+1 groups guess the coefficiens are existing locations
if (rows == r+1)
    for j=[0:groups-1]
        coef=[];
        tot=0;
        for i=[1:r+1]
            index=j*(r+1)+i;
            tot=tot + locations(index) * A(:,i);
            coef = [coef locations(index)];
        end
        if (tot == 0)
            return;
        end
    end
else
    coef=[];
    tot=0;
    rold=r;
    r=size(A,2)-1;
    for i=[1:(mod(n,rold+1))]
        j=groups;
        index=j*(rold+1)+i;
        tot=tot + locations(index) * A(:,i);
        coef = [coef locations(index)];
    end
    if (tot == 0)
        coef
        return;
    end
end
fprintf("Coef calc\n");
coef=0;



if (r==7)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
		for i3=1:(2^field-2)
			primeli3=gf(i3,field);
                for i4=1:(2^field-2)
                    primeli4=gf(i4,field);
					for i5=1:(2^field-2)
						primeli5=gf(i5,field);
						for i6=1:(2^field-2)
							primeli6=gf(i6,field);
                            for i7=1:(2^field-2)
                                primeli7=gf(i7,field);
                                for i8=1:(2^field-2)
                                    primeli8=gf(i8,field);
                                    a = primeli1*A(:,[1])+primeli2*A(:,[2])+primeli3*A(:,[3])+primeli4*A(:,[4])+primeli5*A(:,[5])+primeli6*A(:,[6])+primeli7*A(:,[7])+primeli8*A(:,[8]);
                                    if (a==0)
                                        coef = [i1 i2 i3 i4 i5 i6 i7 i8];
                                        return
                                    end
                                end
                            end    
                        end
                    end
                end
        end
    end
end
end

if (r==6)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
		for i3=1:(2^field-2)
			primeli3=gf(i3,field);
                for i4=1:(2^field-2)
                    primeli4=gf(i4,field);
					for i5=1:(2^field-2)
						primeli5=gf(i5,field);
						for i6=1:(2^field-2)
							primeli6=gf(i6,field);
                            for i7=1:(2^field-2)
                                primeli7=gf(i7,field);
                                a = primeli1*A(:,[1])+primeli2*A(:,[2])+primeli3*A(:,[3])+primeli4*A(:,[4])+primeli5*A(:,[5])+primeli6*A(:,[6])+primeli7*A(:,[7]);
                                if (a==0)
                                    coef = [i1 i2 i3 i4 i5 i6 i7];
                                    return
                                end
                            end    
                        end
                    end
                end
        end
    end
end
end



if (r==5)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
		for i3=1:(2^field-2)
			primeli3=gf(i3,field);
                for i4=1:(2^field-2)
                    primeli4=gf(i4,field);
					for i5=1:(2^field-2)
						primeli5=gf(i5,field);
						for i6=1:(2^field-2)
							primeli6=gf(i6,field);
							a = primeli1*A(:,[1])+primeli2*A(:,[2])+primeli3*A(:,[3])+primeli4*A(:,[4])+primeli5*A(:,[5])+primeli6*A(:,[6]);
							if (a==0)
								coef = [i1 i2 i3 i4 i5 i6];
								return
							end
                        end
                    end
                end
        end
    end
end
end

if (r==4)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
		for i3=1:(2^field-2)
			primeli3=gf(i3,field);
                for i4=1:(2^field-2)
                    primeli4=gf(i4,field);
					for i5=1:(2^field-2)
						primeli5=gf(i5,field);
						a = primeli1*A(:,[1])+primeli2*A(:,[2])+primeli3*A(:,[3])+primeli4*A(:,[4])+primeli5*A(:,[5]);
						if (a==0)
							coef = [i1 i2 i3 i4 i5];
                            return;
                        end
                     end
                end
         end
    end
end
end

if (r==3)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
		for i3=1:(2^field-2)
			primeli3=gf(i3,field);
			for i4=1:(2^field-2)
				primeli4=gf(i4,field);
				a = primeli1*A(:,[1])+primeli2*A(:,[2])+primeli3*A(:,[3])+primeli4*A(:,[4]);
				if (a==0)
					coef = [i1 i2 i3 i4];
					return;
				end
            end
        end
    end
end
end

if (r==2)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
		for i3=1:(2^field-2)
			primeli3=gf(i3,field);
			a = primeli1*A(:,[1])+primeli2*A(:,[2])+primeli3*A(:,[3]);
			if (a==0)
				coef = [i1 i2 i3];
				return;
			end
        end
    end
end
end

if (r==1)
for i1=1:(2^field-2)
	primeli1=gf(i1,field);
	for i2=1:(2^field-2)
		primeli2=gf(i2,field);
			a = primeli1*A(:,[1])+primeli2*A(:,[2]);
			if (a==0)
				coef = [i1 i2];
				return;
			end
    end
end
end

end
