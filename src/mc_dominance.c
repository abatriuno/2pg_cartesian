#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "enums.h"
#include "protein.h"
#include "topology.h"
#include "pdbio.h"
#include "pdbatom.h"
#include "messages.h"
#include "algorithms.h"
#include "string_owner.h"
#include "futil.h"
#include "aminoacids.h"
#include "aminoacids_io.h"
#include "populationio.h"
#include "topology.h"
#include "topologyio.h"
#include "rotation.h"
#include "math_owner.h"
#include "solution.h"
#include "gromacs_objectives.h"
#include "algorithms.h"
#include "randomlib.h"
#include "objective.h"
#include "solutionio.h"
#include "dominance.h"


void save_values_to_analysis(const double *obj1_after, const double *obj2_after,
    const double *obj1_before, const double *obj2_before, const double *obj1_new, 
    const double *obj2_new, char *dominance_status, const input_parameters_t *in_para, 
    const int *num_sol){    
    
    FILE * energy_file; 
    char *file_name;
    file_name = Malloc(char, MAX_FILE_NAME);
    strcpy(file_name, "monte_carlo_dominance_objectives.fit");
    char *fname = path_join_file(in_para->path_local_execute, file_name);        
    if (*num_sol == 1){
        energy_file = open_file(fname, fWRITE);
        fprintf (energy_file,"#Index\t\tEnergy_After_Criteria\t\t\t\tEnergy_Before_Criteria\t\t\t\t\tNew_Solution\t\tDominou_solucao\n");
    }else{
        energy_file = open_file(fname, fAPPEND);
    }  
    fprintf (energy_file,"%i\t%.10e\t%.10e\t%.10e\t%.10e\t%.10e\t%.10e\t%s\n", *num_sol, *obj1_after, *obj2_after,
        *obj1_before, *obj2_before, *obj1_new, *obj2_new, dominance_status); 
    fclose(energy_file);    
    free(fname);
    free(file_name);    
}

void save_solution(const solution_t *solution_curr, 
    const input_parameters_t *in_para, const int *model){
    const protein_t* p_curr;
    FILE * pdbfile; 
    char *file_name;
    file_name = Malloc(char, MAX_FILE_NAME);
    strcpy(file_name, "monte_carlo_solutions.pdb");
    char *fname = path_join_file(in_para->path_local_execute,
        file_name);    
    p_curr = (protein_t*) solution_curr->representation;
    if (*model == 1){
        pdbfile = open_file(fname, fWRITE);
        writeHeader(pdbfile, 0.00, &p_curr->p_topol->numatom);
    }else{
        pdbfile = open_file(fname, fAPPEND);
    }
    writeModel(pdbfile, model);
    writeATOM(pdbfile, p_curr->p_atoms, &p_curr->p_topol->numatom);
    writeEndModel(pdbfile);
    fclose(pdbfile);

    free(fname);
    free(file_name);    
}

/** Accepts updating the current solution with new solution
* solution_curr is the current solution
* solution_new is the new solution
*/ 
void accept(solution_t *solution_curr, solution_t *solution_new){
	protein_t *p_curr;
	const protein_t *p_new;
	p_curr = (protein_t*) solution_curr[0].representation;
	p_new = (protein_t*) solution_new[0].representation;
	copy_protein(p_curr, p_new);
	solution_curr->obj_values[0] = solution_new->obj_values[0];
    solution_curr->obj_values[1] = solution_new->obj_values[1];
}

/** Updates new solution with current solution
* solution_new is the new solution
* solution_curr is the current solution
*/ 
void reject(solution_t *solution_new, const solution_t *solution_curr){
	//const protein_t *p_curr;
	solution_new->obj_values[0] = solution_curr->obj_values[0];
}


static int compare_solution(const void *x, const void *y){
    double fx, fy;
    fx = ((solution_t *)x)->obj_values[0];
    fy = ((solution_t *)y)->obj_values[0];
    if (fx > fy){
        return 1;
    }else if ( (fx -fy) == 0.0001){
        return 0;
    }else{
        return -1;
    }
}

/** Builds a new solution and computes the objective
* solution_new is new solution
* solution_curr is current solution
* in_para is the input parameter
*/
void mc_build_solution(solution_t *solution_new, const solution_t *solution_curr,
    const input_parameters_t *in_para){
    float angle_radians, angle_degree, rate;
    int what_rotation;
    int max_kind_of_rotation = 4;
    int num_residue_choose;

    //Copy current solution to new solution
    protein_t *ind_new;
    const protein_t *p_curr;    
    p_curr = (protein_t*) solution_curr[0].representation;
    ind_new = (protein_t*) solution_new[0].representation;
    copy_protein(ind_new, p_curr);
    
    //Choose a residue. It must be started 1 untill number of residue
    num_residue_choose = get_choose_residue(&ind_new->p_topol->numres);
    //Obtaing kind of rotation
    what_rotation = _get_int_random_number(&max_kind_of_rotation);            
    //Appling the rotation 
    if (what_rotation == 0){
        //Obtaing a random degree angule 
        angle_degree = _get_float_random_interval(&in_para->min_angle_mutation_psi, 
        &in_para->max_angle_mutation_psi);
        //Cast to radians
        angle_radians = degree2radians(&angle_degree);
        rotation_psi(ind_new, &num_residue_choose, &angle_radians);
    }else if (what_rotation == 1){
        //Obtaing a random degree angule 
        angle_degree = _get_float_random_interval(&in_para->min_angle_mutation_phi, 
        &in_para->max_angle_mutation_phi);
        //Cast to radians
        angle_radians = degree2radians(&angle_degree);
        rotation_phi(ind_new, &num_residue_choose, &angle_radians);            
    }else if (what_rotation == 2){
        //Obtaing a random degree angule 
        angle_degree = _get_float_random_interval(&in_para->min_angle_mutation_omega, 
        &in_para->max_angle_mutation_omega);
        //Cast to radians
        angle_radians = degree2radians(&angle_degree);
        rotation_omega(ind_new, &num_residue_choose, &angle_radians);            
    }else if (what_rotation == 3){
        int chi = 0;
        int max_chi;
        int num_chi_choose;
        char *res_name;
        res_name = Malloc(char, 4);
        get_res_name_from_res_num(res_name, &num_residue_choose, 
            ind_new->p_atoms, &ind_new->p_topol->numatom);
        max_chi = get_number_chi(res_name);
        if (max_chi > 0){
            //Choose a chi of residue. It must be started 1 untill number of residue
            if (max_chi == 1){
                chi = 1;
            }else{
                while (chi == 0){                
                    num_chi_choose = max_chi + 1;
                    chi = _get_int_random_number(&num_chi_choose);
                }                    
            }                
            //Obtaing a random degree angule 
            angle_degree = _get_float_random_interval(&in_para->min_angle_mutation_side_chain, 
            &in_para->max_angle_mutation_side_chain);
            //Cast to radians
            angle_radians = degree2radians(&angle_degree);
            //printf("what_rotation %i num_residue_choose %i chi %i \n", 3, num_residue_choose, chi);
            rotation_chi(ind_new, &num_residue_choose, &chi, &angle_radians);
        }
        free(res_name);
    }    
}

int mc_dominance(const input_parameters_t *in_para){

    char *prefix;
    int num_solution; // stores the number of solution    
    int num_solution_dominance; // number of solutions to verify dominance
    int model;
    char *dominance_status;
    double obj1_before, obj2_before, obj1_after, obj2_after, obj1_new, obj2_new;

    primary_seq_t *primary_sequence; // Primary Sequence of Protein
    
    protein_t *prot_new; // new protein structure. It is build by mc_build_solution function
    protein_t *prot_curr; // current protein structure

    solution_t *solution_new;  // new solution
    solution_t *solution_curr; // current solution
    solution_t *solutions;     // store the total of solutions to verify dominance
    dominance_t *dominance;    // apply dominance on the total of solutions

    //Starting values
    num_solution_dominance = 2;
    num_solution = 1;

    model = 0;

    //Loading Fasta file
    primary_sequence = _load_amino_seq(in_para->seq_protein_file_name);

    //Allocating PDB ATOMS
    prot_new = allocateProtein(&num_solution);
    prot_curr = allocateProtein(&num_solution);
    
    //Loading initial population
    load_initial_population_file(prot_curr, &num_solution, 
        in_para->path_local_execute,in_para->initial_pop_file_name,
        primary_sequence);    

    //Setting protein previous that is the same of prot_curr
    prot_new[0].p_atoms = allocate_pdbatom(&prot_curr[0].p_topol->numatom);    
    prot_new[0].p_topol = allocateTop_Global(&prot_curr[0].p_topol->numres, &prot_curr[0].p_topol->numatom);
    copy_protein(&prot_new[0], &prot_curr[0]);
    build_topology_individual(&prot_new[0]);

    //Saving topology of population 
    prefix = Malloc(char,10);
    strcpy(prefix, "prot");
    save_topology_population(prot_curr, &num_solution, 
    in_para->path_local_execute, prefix);    
    free(prefix);

/**************** STARTING Monte Carlo Dominance Algorithm *************************/
    display_msg("Starting Monte Carlo Dominance Algorithm \n");
    initialize_algorithm_execution(primary_sequence, in_para);
    init_gromacs_execution();    
    //Setting solutions
    solution_curr = allocate_solution(&num_solution, &in_para->number_fitness);
    solution_new = allocate_solution(&num_solution, &in_para->number_fitness);    
    solutions = allocate_solution(&num_solution_dominance, &in_para->number_fitness);

    //Setting reference of proteins to solution 
    set_proteins2solutions(solution_curr, prot_curr, &num_solution);
    set_proteins2solutions(solution_new, prot_new, &num_solution);
	//Computing solutions with GROMACS
    get_gromacs_objectives(solution_curr, in_para);    

    dominance = allocate_dominance(&num_solution_dominance);

    for (int s = 1; s <= in_para->MonteCarloSteps; s++){        

        //Building new Solution
    	mc_build_solution(&solution_new[0], &solution_curr[0], in_para);

        //Calculates energy of new Solution
    	get_gromacs_objectives(&solution_new[0], in_para);        

        // Store solution_curr and solution_new on one structure solutions
        solutions[0] = solution_curr[0];
        solutions[1] = solution_new[0];

        obj1_before = solution_curr[0].obj_values[0];
        obj2_before = solution_curr[0].obj_values[1];
        obj1_new = solution_new[0].obj_values[0];
        obj2_new = solution_new[0].obj_values[1];

        // Verifying dominance
        set_dominance(dominance,solutions,&num_solution_dominance);
        
        // printf("-----------------\n");
        // printf("Step: %d\n\n",s);


        // printf("[Potential/Solvatation] solucao corrente[0]: %f / %f\n",dominance[0].sol->obj_values[0],dominance[0].sol->obj_values[1]);
        // printf("[Potential/Solvatation] solucao nova[1]: %f / %f\n",dominance[1].sol->obj_values[0],dominance[1].sol->obj_values[1]);


        // printf("corrente[0] - how_many_solutions_dominate_it: %d\n", dominance[0].how_many_solutions_dominate_it );
        // printf("nova[1] - how_many_solutions_dominate_it: %d\n\n",dominance[1].how_many_solutions_dominate_it);   

        // If solution_new dominates solution_curr
		if( dominance[1].how_many_solutions_dominate_it == 0 && dominance[0].how_many_solutions_dominate_it == 1 ){

            accept(&solution_curr[0],&solution_new[0]);

            obj1_after = solution_new[0].obj_values[0];
            obj2_after = solution_new[0].obj_values[1];     
            dominance_status = (char *)"YES";          
 
            save_values_to_analysis(&obj1_after, &obj2_after, &obj1_before, &obj2_before, &obj1_new,
                 &obj2_new, dominance_status, in_para, &s);

		} else if (dominance[1].how_many_solutions_dominate_it == 1 && dominance[0].how_many_solutions_dominate_it == 0) {

            obj1_after = solution_curr[0].obj_values[0];
            obj2_after = solution_curr[0].obj_values[1];
            dominance_status = (char *)"NO";             
            
            save_values_to_analysis(&obj1_after, &obj2_after, &obj1_before, &obj2_before, &obj1_new,
                 &obj2_new, dominance_status, in_para, &s);          
        } else {

            obj1_after = 0;
            obj2_after = 0;
            dominance_status = (char *)"--";

            save_values_to_analysis(&obj1_after, &obj2_after, &obj1_before, &obj2_before, &obj1_new,
                 &obj2_new, dominance_status, in_para, &s);             
        }

        //Saving PDB of current solution
        if (s % in_para->freq_mc == 0){
            model  = model + 1;
            save_solution(&solution_curr[0], in_para, &model);
        }
    }    
    //save_final_solution(&solution_curr[0], in_para);
    finish_gromacs_execution();

/**************** FINISHED Multi-objective Evolutionary Algorithm *************************/     

    desallocate_dominance(dominance, &num_solution_dominance);
    desallocate_solution(solutions, &num_solution_dominance);
    desallocate_solution(solution_curr, &num_solution);
    desallocate_solution(solution_new, &num_solution);
    desallocateProtein(prot_curr, &num_solution);
    desallocateProtein(prot_new, &num_solution);
    desallocate_primary_seq(primary_sequence);

	return 0;
}
