#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "diehdral.h"
#include "messages.h"
#include "solution.h"
#include "futil.h"
#include "string_owner.h"
#include "protein.h"
#include "topology.h"
#include "pdbio.h"
#include "pdbatom.h"
#include "aminoacids.h"
#include "aminoacids_io.h"
#include "topologyio.h"
#include "math_owner.h"
#include "vector_math.h"


float compute_diehdral_angle(const own_vector_t *a1,
		const own_vector_t *a2,const own_vector_t *a3,	const own_vector_t *a4){
	/* Computes the dihedral angle	 */
	own_vector_t *P1, *P2, *M, *r1, *r2, *r3;
	double mod_P1, mod_P2;
	double W;

	P1 = Malloc(own_vector_t,1);
	P2 = Malloc(own_vector_t,1);
	M = Malloc(own_vector_t,1);
	r1 = Malloc(own_vector_t,1);
	r2 = Malloc(own_vector_t,1);
	r3 = Malloc(own_vector_t,1);

	//Computing distances
	sub_vector(r1,a1,a2);
	sub_vector(r2,a2,a3);
	sub_vector(r3,a3,a4);

	cross_product(P1,r1,r2);
	cross_product(P2,r2,r3);
	mod_P1 = mod_vector(P1);
	mod_P2 = mod_vector(P2);

	W = acos(scalar_prod(P1,P2)/(mod_P1*mod_P2));
	//Check if is necessary change the signal of W
	cross_product(M,P1,P2);
	if (scalar_prod(M,r2) < 0){
		W = W *(-1);
	}
	//Deallocating variables
	free(P1);
	free(P2);
	free(M);
	free(r1);
	free(r2);
	free(r3);

	return W;

}

/* compute phi of residue
*/
float compute_phi_residue(pdb_atom_t *prot, 
	const int *res_num, const top_global_t *top){	
	if (*res_num > 1){
		if (residue_is_caps_from_num(prot, res_num, top) == btrue){
			return 0;
		}
		char *atmC_ = NULL;
		char *atmN = NULL;
		char *atmCA = NULL;
		char *atmC = NULL;
		int res_num_ant;		
		float phi;
		const own_vector_t *a1,*a2,*a3,*a4;

		atmC_ = (char*)malloc(sizeof(char)*2);
		atmN = (char*)malloc(sizeof(char)*2);
		atmCA = (char*)malloc(sizeof(char)*3);
		atmC = (char*)malloc(sizeof(char)*2);
		strcpy(atmC_, "C");
		strcpy(atmN, "N");
		strcpy(atmCA, "CA");
		strcpy(atmC, "C");

		res_num_ant = *res_num - 1;
		a1 = get_pdb_atom_coordinates(prot,&res_num_ant, atmC_, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmN, &top->numatom);
		a3 = get_pdb_atom_coordinates(prot,res_num, atmCA, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmC, &top->numatom);

		phi = compute_diehdral_angle(a1, a2, a3, a4);

		free(atmC_);
		free(atmN);
		free(atmCA);
		free(atmC);

		return phi;
	}
	return 0;
}

/* compute psi of residue
*/
float compute_psi_residue(pdb_atom_t *prot, 
	const int *res_num, const top_global_t *top){	
	if (*res_num < top->numres ){
		if (residue_is_caps_from_num(prot, res_num, top) == btrue){
			return 0;
		}
		char *atmN = NULL;
		char *atmCA = NULL;
		char *atmC = NULL;
		char *atmN_plus = NULL;

		int res_num_plus;		
		float psi;
		const own_vector_t *a1,*a2,*a3,*a4;
		
		atmN = (char*)malloc(sizeof(char)*2);
		atmCA = (char*)malloc(sizeof(char)*3);
		atmC = (char*)malloc(sizeof(char)*2);
		atmN_plus = (char*)malloc(sizeof(char)*2);		
		strcpy(atmN, "N");
		strcpy(atmCA, "CA");
		strcpy(atmC, "C");
		strcpy(atmN_plus, "N");
		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmN, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmCA, &top->numatom);
		a3 = get_pdb_atom_coordinates(prot,res_num, atmC, &top->numatom);
		res_num_plus = *res_num + 1;
		a4 = get_pdb_atom_coordinates(prot,&res_num_plus, atmN_plus, &top->numatom);
		
		psi = compute_diehdral_angle(a1, a2, a3, a4);
		
		free(atmN);
		free(atmCA);
		free(atmC);
		free(atmN_plus);

		return psi;
	}
	return 0;
}

/* compute omega of residue
*/
float compute_omega_residue(pdb_atom_t *prot, 
	const int *res_num, const top_global_t *top){

	if (*res_num < top->numres ){	
		int res_num_plus;
		res_num_plus = *res_num + 1;

		char *atmCA = NULL;
		char *atmC = NULL;
		char *atmCA_plus = NULL;
		char *atmN_plus = NULL;
		
		float omega;
		const own_vector_t *a1,*a2,*a3,*a4;
				
		atmCA = (char*)malloc(sizeof(char)*4); //Remember CH3
		atmC = (char*)malloc(sizeof(char)*2);
		atmN_plus = (char*)malloc(sizeof(char)*2);		
		atmCA_plus = (char*)malloc(sizeof(char)*4); //Remember CH3
		if ( (residue_is_ACE_from_num(prot, res_num, top) == btrue) ) {
			strcpy(atmCA, "CH3");
		}else{
			strcpy(atmCA, "CA");
		}		
		strcpy(atmC, "C");		
		strcpy(atmN_plus, "N");
		if ( (residue_is_NME_from_num(prot, &res_num_plus, top) == btrue) ){
			strcpy(atmCA_plus, "CH3");
		}else{
			strcpy(atmCA_plus, "CA");
		}				
		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmCA, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmC, &top->numatom);		
		
		a3 = get_pdb_atom_coordinates(prot, &res_num_plus, atmN_plus, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,&res_num_plus, atmCA_plus, &top->numatom);
		
		omega = compute_diehdral_angle(a1, a2, a3, a4);
				
		free(atmCA);
		free(atmC);
		free(atmN_plus);
		free(atmCA_plus);

		return omega;
	}
	return 0;
}

/* compute chi of residue
* chi array that contains each chi angles of residues
* num_chi stores how many chi exists in residue
*/
void compute_chi_residue(float *chi, int *num_chi, pdb_atom_t *prot, 
	const int *res_num, const top_global_t *top){	
	
	char *atmA1 = NULL;
	char *atmA2 = NULL;
	char *atmA3 = NULL;
	char *atmA4 = NULL;
	char *res_name = NULL;
	const own_vector_t *a1,*a2,*a3,*a4;

	*num_chi = -1;
	//Obtaing residue name
	res_name = (char*)malloc(sizeof(char)*4);
	get_res_name_from_res_num(res_name,res_num, prot, &top->numatom);

	atmA1 = (char*)malloc(sizeof(char)*5);
	atmA2 = (char*)malloc(sizeof(char)*5);
	atmA3 = (char*)malloc(sizeof(char)*5);		
	atmA4 = (char*)malloc(sizeof(char)*5);

	if ( strcmp(res_name, "ARG") == 0 ){
		*num_chi = 4;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi3		
		strcpy(atmA1, "CB");
		strcpy(atmA2, "CG");		
		strcpy(atmA3, "CD");
		strcpy(atmA4, "NE");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[2] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi4			
		strcpy(atmA1, "CG");
		strcpy(atmA2, "CD");		
		strcpy(atmA3, "NE");
		strcpy(atmA4, "CZ");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[3] = compute_diehdral_angle(a1, a2, a3, a4);

	}else if ( strcmp(res_name, "LYS") == 0 ){
		*num_chi = 4;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi3		
		strcpy(atmA1, "CB");
		strcpy(atmA2, "CG");		
		strcpy(atmA3, "CD");
		strcpy(atmA4, "CE");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[2] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi4		
		strcpy(atmA1, "CG");
		strcpy(atmA2, "CD");		
		strcpy(atmA3, "CE");
		strcpy(atmA4, "NZ");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[3] = compute_diehdral_angle(a1, a2, a3, a4);

	}else if ( strcmp(res_name, "ASP") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "OD1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "GLU") == 0 ){
		*num_chi = 3;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi3				
		strcpy(atmA1, "CB");
		strcpy(atmA2, "CG");		
		strcpy(atmA3, "CD");
		strcpy(atmA4, "OE1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[2] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "GLN") == 0 ){
		*num_chi = 3;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi3		
		strcpy(atmA1, "CB");
		strcpy(atmA2, "CG");		
		strcpy(atmA3, "CD");
		strcpy(atmA4, "OE1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[2] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "ASN") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "OD1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "HIS") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "ND1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "SER") == 0 ){
		*num_chi = 1;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "OG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "THR") == 0 ){
		*num_chi = 1;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "OG1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "TYR") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "CYS") == 0 ){
		*num_chi = 1;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "SG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "MET") == 0 ){
		*num_chi = 3;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "SD");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi3		
		strcpy(atmA1, "CB");
		strcpy(atmA2, "CG");		
		strcpy(atmA3, "SD");
		strcpy(atmA4, "CE");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[2] = compute_diehdral_angle(a1, a2, a3, a4);

	}else if ( strcmp(res_name, "TRP") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "ILE") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG1");
		strcpy(atmA4, "CD");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "LEU") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "PHE") == 0 ){
		*num_chi = 2;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);

		//chi2		
		strcpy(atmA1, "CA");
		strcpy(atmA2, "CB");		
		strcpy(atmA3, "CG");
		strcpy(atmA4, "CD1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[1] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "VAL") == 0 ){
		*num_chi = 1;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG1");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);
	}else if ( strcmp(res_name, "PRO") == 0 ){
		*num_chi = 0;
/*It is considered without chi, because it will be not moved.
See get_number_chi function at topology.c 	
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);
*/		
	}else if ( strcmp(res_name, "ALA") == 0 ){
		*num_chi = 0;
/*	In Topology is considered ALA with out side chain. See 
set_fixed_moved_atoms_side_chains_chi function at topology.c */
/*		
		*num_chi = 1;
		//chi1		
		strcpy(atmA1, "N");
		strcpy(atmA2, "CA");		
		strcpy(atmA3, "CB");
		strcpy(atmA4, "CG");		
		a1 = get_pdb_atom_coordinates(prot,res_num, atmA1, &top->numatom);
		a2 = get_pdb_atom_coordinates(prot,res_num, atmA2, &top->numatom);			
		a3 = get_pdb_atom_coordinates(prot,res_num, atmA3, &top->numatom);
		a4 = get_pdb_atom_coordinates(prot,res_num, atmA4, &top->numatom);
		chi[0] = compute_diehdral_angle(a1, a2, a3, a4);
*/		
	}else if ( strcmp(res_name, "GLY") == 0 ){
		*num_chi = 0;
	}

	free(atmA1);
	free(atmA2);
	free(atmA3);
	free(atmA4);
	free(res_name);
}
