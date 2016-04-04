#include <stdlib.h>
#include <string.h>

#include "mc_dominance.h"
#include "dominance.h"
#include "load_parameters.h"
#include "messages.h"

int main(int argc, char *argv[]){
	input_parameters_t in_param;
	display_msg("Reading the configure file \n");
	load_parameters_from_file(&in_param,argv[1]);

	mc_dominance(&in_param);

	deAllocateload_parameters(&in_param);

	display_msg("Done MC Dominance !!! \n");
	return 0;
}