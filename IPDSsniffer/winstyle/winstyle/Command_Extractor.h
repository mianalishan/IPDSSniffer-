#pragma once

#include<iostream>
#include<fstream>
#include<omp.h>
#include<sstream>
#include<list>
#include<queue>
#include<thread>
using namespace std;
struct ipds_command
{
	unsigned short length;
	unsigned short command;
	size_t pos_in_file = 0;
	void * pData;
};


struct tcpMessage
{
	unsigned long messageLength;
	unsigned long ID;
	unsigned char *pData;
};

#define SWAP_WORD(x)   x = x << 8 | x >> 8 
#define SWAP_DWORD(x)  x = x << 24 | ((x & 0x0000FF00) << 8 )  | ((x & 0x00FF0000) >> 8) | x >> 24


class  StreamReader
{

	size_t offset;
	std::string fileNmae;
	bool bInitialized;
	size_t  fileLength;

	FILE *fptr;
public:
	int count;
	std::vector<ipds_command *> cmdVect;
	bool init(std::string fNmae)
	{
		count = 0;
		bInitialized = false;
		fptr = NULL;
		offset = 0;
		fileLength = 0;
		fopen_s(&fptr, fNmae.c_str(), "r+b");
		if (fptr)
		{
			fseek(fptr, 0, SEEK_SET);//starting point
			size_t stP = ftell(fptr);
			fseek(fptr, 0, SEEK_END);//ending point
			fileLength = ftell(fptr) - stP;
			fseek(fptr, 0, SEEK_SET);
			bInitialized = true;
		}
		return bInitialized;
	}

	bool GetNextField(std::vector<ipds_command *> & commandVect)
	{

		if (ReadTCPMessage2())
		{
			//commandVect = cmdVect;
			return true;
		}
		fclose(fptr);
		return false;
		//if (fptr)
		//{
		//	fseek(fptr, offset, SEEK_SET);
		//	fread(&ipdsField.length, 2, 1, fptr);
		//	offset += 2;
		//	fread()
		//}
	}
	 void getVector(std::vector<ipds_command *> & commandVect)
	{

	}
	bool ReadTCPMessage()
	{
		int count = 0;
		tcpMessage tcp;
		if (fptr)
		{
			//if (feof(fptr) == 0)
			//{
			while (1)
			{
				if ((feof(fptr) == 0) && (offset < fileLength))
				{
					fseek(fptr, offset, SEEK_SET);
					if (fread(&tcp.messageLength, 4, 1, fptr) != 1)
					{
						return false;
					}
					SWAP_DWORD(tcp.messageLength);
					offset += 4;
					fseek(fptr, offset, SEEK_SET);
					if (tcp.messageLength < 4)
					{
						offset -= 3;
						continue;
					}
					tcp.pData = new unsigned char[tcp.messageLength - 4];
					if (fread(tcp.pData, 1, tcp.messageLength - 4, fptr) != tcp.messageLength - 4)
					{
						cout << "count " << count << endl;
						return false;
					}
					offset += (tcp.messageLength - 4);
					tcp.ID = *(unsigned long*)tcp.pData;
					SWAP_DWORD(tcp.ID);
					if (tcp.ID == 0x0000000E)
					{
						count++;
						cout << "count " << count << endl;
						//ProcessIPDS(tcp.pData + 8);
						//delete[] tcp.pData;
						//return true;
					}

					printf("Length = %X , ID = %X  , FOff = %X\n", tcp.messageLength, tcp.ID, offset);
					delete tcp.pData;
				}
				else
				{
					cout << "count " << count << endl;
					return false;
				}
			}
		}
		cout << "count " << count << endl;
		return false;
	}
	bool ReadTCPMessage2()
	{

		int temp = 0;
		tcpMessage tcp;
		if (fptr)
		{
			//if (feof(fptr) == 0)
			//{

			while ((1) && (feof(fptr) == 0))
			{
				fread(&temp, 1, 1, fptr);
				while ((temp != 0x0e) && (feof(fptr) == 0)) {
					fread(&temp, 1, 1, fptr);
					

				}

				unsigned long ARQFlag;
				size_t pos = ftell(fptr) - 4;
				fseek(fptr, pos, SEEK_SET);
				fread(&tcp.ID, 1, 4, fptr);
				//tcp.ID = *(unsigned long*)tcp.pData;
				SWAP_DWORD(tcp.ID);
				fread(&ARQFlag, 1, 4, fptr);
				SWAP_DWORD(ARQFlag);
				if ((tcp.ID == 0x0000000E) && ((ARQFlag == 0x00000000) || (ARQFlag == 0x00000001)))
				{
					size_t pos = ftell(fptr);
					fseek(fptr, pos, SEEK_SET);
					fread(&tcp.messageLength, 1, 4, fptr);
					SWAP_DWORD(tcp.messageLength);
					if (tcp.messageLength >= fileLength)
					{
						//int a;
						
						continue;
					}
					tcp.pData = new unsigned char[tcp.messageLength - 4];
					fread(tcp.pData, 1, tcp.messageLength, fptr);
					count++;
					//cout << "count2 " << count << endl;
					ProcessIPDS(tcp.pData, tcp.messageLength);
					//delete[] tcp.pData;
					//printf("Length = %X , ID = %X  , FOff = %X\n", tcp.messageLength, tcp.ID, pos);
					//return true;

				

				}
				else
				{
					continue;
				}
				//delete tcp.pData;


			}

		}
		
		fclose(fptr);
		std::remove("TempPacPayload.txt");
		ClearVector();
		return false;
	}

	void ClearVector()
	{
		for (size_t s = 0; s < cmdVect.size(); s++)
		{
			if (cmdVect[s])
			{
				delete cmdVect[s];
			}
		}
		cmdVect.clear();
	}

	void ProcessIPDS(unsigned char *pBuffer, long len)
	{

		unsigned char *pTemp = pBuffer;
		int templen = len;
		unsigned long bufLght = *(unsigned long *)pBuffer;
		SWAP_DWORD(bufLght);
		size_t   curOffset = 0;
		ipds_command *pIPDSC;
		unsigned short temp;
		//ClearVector();
		while (curOffset < len)
		{
			/*unsigned short temp;
			pIPDSC = new ipds_command;
			//pIPDSC->pos_in_file = ftell(fptr)-pIPDSC->length;
			pIPDSC->length = *(unsigned short *)pTemp;
			SWAP_WORD(pIPDSC->length);
			curOffset += 2;
			pTemp = pTemp + 2;
			pIPDSC->command = *(unsigned short *)pTemp;
			SWAP_WORD(pIPDSC->command);

			*/
			if (curOffset < len) {
				curOffset += 1;
				pTemp = pTemp = pTemp + 1;
				temp = *(unsigned char *)pTemp;
			}
			while ((temp != 0xd6) && (curOffset<len)) {
				try {
					pTemp = pTemp + 1;
					curOffset += 1;
					temp = *(unsigned char *)pTemp;
				}
				catch (exception e)
				{
					return;
				}

			}
			pIPDSC = new ipds_command;
			pTemp = pTemp - 2;
			curOffset -= 1;
			pIPDSC->length = *(unsigned short *)pTemp;
			SWAP_WORD(pIPDSC->length);
			curOffset += 2;
			pTemp = pTemp + 2;
			//command read
			pIPDSC->command = *(unsigned short *)pTemp;
			SWAP_WORD(pIPDSC->command);
			curOffset += 2;
			pTemp = pTemp + 2;
			if (pIPDSC->length > templen)
			{
				return;

			}


			//pIPDSC->pData= new unsigned char[pIPDSC->length - 4];
			//pIPDSC->pData = (void *)pTemp;
			curOffset += pIPDSC->length - 4;
			pTemp = pTemp + pIPDSC->length - 4;

			cmdVect.push_back(pIPDSC);
			
		}
	}
	void show_command()
	{
		
	}
};

/*class ReadFromFile
{
public:
queue<int> command_address;
queue<ipds_command> commands;
ifstream in;

unsigned char *buffer;
ReadFromFile()
{
buffer = new unsigned char[1024 * 1024];
}
queue<ipds_command> get_commands_queue()
{
cout << "commandsc  " << commands.size();
return commands;
}
void read()
{

//int *length;
char temp;
in.open("E:\\pacPayload2.txt", ios::binary);

//fread(length,);
//in.read(buffer, length - 4);

while (1) {
in >> temp;



if (temp == 'Ö')
{

//cout<<"pos  "<<in.tellg()<<endl<<endl;
fetching_commands_info(in.tellg());
//command_address.push(in.tellg());
//cout << hex << (signed int)temp;
//in >> temp;
//cout <<hex<< (signed int)temp<<endl;
//in >> temp;
//cout << temp;


}
if (in.eof())
{
break;
cout << "end of file break";

}

}
}
void fetching_commands_info(int pos)
{
//char len[2];
unsigned short len;
ipds_command obj;
obj.pos_in_file = pos;
in.seekg(pos - 3);
in.read((char *)&len, 2);

stringstream ss;

ss << hex << (unsigned int)len[0] << hex << (unsigned int)len[1];
ss >> obj.length;
//cout <<"len    "<<obj.length <<endl;
in.seekg(pos - 1);
in.read(len, 2);
obj.command[0] = len[0];
obj.command[1] = len[1];
//cout << hex << (unsigned int)obj.command[0] << (unsigned int)obj.command[1]<<endl;

commands.push(obj);



}
int fetch_no_of_pages()
{
queue<ipds_command> temp;
ipds_command temp2;
int page_count = 0;
temp = commands;
int k;
cout << "size " << temp.size() << endl;
cin >> k;
while (!temp.empty())
{
temp2 = temp.front();

if (temp2.command[1] == 175)
{
cout << "if";
page_count++;
}
temp.pop();
}
return page_count;
}
char* fetch_command_data(int pos, int len)
{
char *data = new char[len];
in.seekg(pos - 2);
in.read(data, len);

return data;
}
char* chartohex(unsigned char *cmd)
{
char* temp = new char[2];
stringstream ss;

ss << hex << (unsigned int)cmd[0] << hex << (unsigned int)cmd[1];
ss >> temp;
return temp;
}

};
/*int main()
{
ReadFromFile rf;
rf.read();
queue<ipds_command> temp;
temp = rf.get_commands_queue();
cout << temp.size();
bool pg = false;
for (double i = 0; i <2523; i++)
{
ofstream out;
out.open("E:\\newtext.txt", ios::app);
//cout<<temp.front().command[1];
//out << temp.front().command[1]<<endl;
if (temp.front().command[1] == '¯')
{
cout << "page begin" << endl;
char *data = rf.fetch_command_data(temp.front().pos_in_file, temp.front().length);
cout << temp.front().length;
for (int i = 0; i < temp.front().length; i++)
{
cout << hex << (unsigned int)data[i];
}
cout << endl << "***************************" << endl;

}
//cout << i << endl;
temp.pop();

}
cout << endl << temp.size();
}*/
