#include <string>
#include <fstream>
#include <map>

class LoadConfig{
   private:
       std::ifstream fileConf;
       std::map<std::string,std::string> dicParameters;
       std::string trim(std::string s);

   public:
	   //Com o const nao é permitido chamar metodos q nao sejam const
	   LoadConfig(const std::string &pathFileName);
	   ~LoadConfig();
	   void file2Map();
	   std::string getParameter(const std::string &Parameter);
	   char* getParameterChar(const std::string &Parameter);
};
