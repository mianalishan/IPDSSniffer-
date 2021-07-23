#pragma once
#include<iostream>
#include<string>
#include<map>
#include<fstream>
struct ipds_cmd_ref
{
	std::string cmd;
	std::string cmdHex;
	std::string cmdd;
	std::string subset;
};
class MappingCommand
{
private:
	std::ifstream in;
	std::map<std::string,ipds_cmd_ref>  refCmd;
public:
	bool fillRefCmd()
	{
		in.open("cmdfile.txt");
		while (!in.eof())
		{
			ipds_cmd_ref icr;
			in >> icr.cmd;
			in >> icr.cmdHex;
			std::string temp;
			while( (temp.find("page")==-1))
			{
				
				in >> temp;
				icr.cmdd = icr.cmdd + temp;
				
			}
			in >> icr.subset;
			in >> icr.subset;
		refCmd.insert(std::pair<std::string,ipds_cmd_ref>(icr.cmdHex,icr));
			//return true;
		}
		return true;
	}
	
   ipds_cmd_ref getCmdAssoc(std::string  key )
	{
		
		  auto it=refCmd.find(key);
		  if (it != refCmd.end())
		  {
			  return it->second;
		  }
		  else
		  {
			  ipds_cmd_ref icm;
			  icm.cmd = key;
			  return icm;
		  }
   }
};
