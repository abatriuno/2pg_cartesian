#include <LoadConfig.h>
#include <string.h>

#define MAXLINE  200
#include "defines.h"

//Equivale from sdt import *
using namespace std;

LoadConfig::LoadConfig(const string &pathFileName)

   //cast string to *char
   :fileConf(pathFileName.c_str())
{
	file2Map();
}


LoadConfig::~LoadConfig(){
   fileConf.close();
}

void LoadConfig::file2Map(){
  char *l;
  l = Malloc(char, MAXLINE);
  strcpy(l, "\0");
  while (fileConf.eof() == false){
#ifdef _WIN32
       fileConf.getline(l,MAXLINE, '\n');
#endif
#ifdef linux
 	  fileConf.getline(l,MAXLINE);
#endif
	   //printf("%s\n", l);
       string s(l);
       int ind_token = s.find("=");
       if (ind_token != string::npos){
           string key = trim(s.substr(0,ind_token-1));
           string value = trim( s.substr(ind_token+2,s.length()) );
           dicParameters[key] = value;
       }
	}
  free(l);
}

std::string LoadConfig::getParameter(const std::string &Parameter){
	return dicParameters[Parameter];
}

std::string LoadConfig::trim(std::string s){
    char l[MAXLINE] = "";
    char l2[MAXLINE]= "";
    strcpy(l,s.c_str());    
    int i = 0;
    int j = 0;
    while (l[i] != '\0'){
        if (l[i] != ' '){
        	l2[j++] = l[i];
        };
        i++;
    }
    l2[j] = '\0';
	return string(l2);
}


char* LoadConfig::getParameterChar(const std::string &Parameter){  
#ifdef _WIN32
	char * c = strdup(getParameter(Parameter).c_str());
	return c;
#endif

#ifdef linux
	return const_cast< char* > ( getParameter(Parameter).c_str() );
#endif
	
}
