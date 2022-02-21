#include "mitra/Client.h"
#include<iostream>
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>
using namespace std;

#define REAL true
#define FAKE false

int fileid = 1;
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");
set<string> neg = {"\n","\0", " ", "-","?","from","to", "in"};

int stoI(string updt_cnt)
{
        int updc;
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
        return updc;
}
vector<string> getUniquedWords(vector<string> kws, int fileid)
{
    vector<string> kw;
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
        if (!mp.count(word))
            mp.insert(make_pair(word, 1));
        else
            mp[word]++;
    }
    for (map<string, int> :: iterator p = mp.begin();p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
    return kw;
}

vector<string> divideString(string filename, int sz, int id)
{
	fstream fs(filename); 
	string str((istreambuf_iterator<char>(fs)),
	               (istreambuf_iterator<char>()));
	int str_size = str.length();
	if (str_size% sz !=0)
	{
		int pad = ceil(str_size/sz)+1;
		pad = pad*sz-str_size;
	        str.insert(str.size(), pad, '#');
	}
	str_size = str.length();
	int i;
	vector<string> result;
	string temp="";
	for (i = 0; i < str_size; i++) 
	{
	    if (i % sz == 0) 
	    {
		  if(i!=0)
	          {
		     string ttemp ="";//id;
		     ttemp.append(temp); 
	             result.push_back(ttemp);    
	          }
	          temp="";
	    }
	    temp +=str[i];
	}
	string ttemp = "";//id;
	ttemp.append(temp);	
	result.push_back(ttemp); 
	return result;
}

string getFileContent(string path)
{
	  ifstream file(path);
	  string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	      return content;
}

static void list_dir (const char * dir_name, Client& client, bool real)
{
    string delimiters("|?@,:!\">; -./  \n");
    DIR * d;
    d = opendir (dir_name);

   if (! d) 
   {
      fprintf (stderr, "Cannot open directory '%s': %s\n",
         dir_name, strerror (errno));
       exit (EXIT_FAILURE);
   }
   while (1) 
   {
      struct dirent * entry;
      const char * d_name;
      entry = readdir (d);
      if (! entry) 
      {
            break;
      }
      d_name = entry->d_name;
            if (! (entry->d_type & DT_DIR)) 
	    {
              printf ("%s/%s\n", dir_name, d_name);
	      string file = dir_name;
	      file = file.append("/");
	      file = file.append(d_name);
	      vector<string> kws1, kws;
	      string cont = getFileContent(file);
	      cout <<"=====================================" << endl;

	      boost::split(kws1, cont, boost::is_any_of(delimiters));
	      kws =  getUniquedWords(kws1, fileid);
	      for (auto it = kws.begin(); it != kws.end(); it++)
	      {
		      if(neg.find(*it)!=neg.end())
		      {
			 kws.erase(it--);
		      }
	      }
	      cout << endl <<file<< " " << fileid <<endl << endl;
	      cout << "============================" << endl;
    		//vector<string> blocks;
		//blocks = divideString(file,BLOCK,id);
		if(real)
		{
        		client.updateFile(OP::INS,fileid, cont, false);
			for(auto k: kws)
			{
				int lenn = k.length();
				if(lenn <= 12)
				{
					client.update(OP::INS,k, fileid,false);
				}
				else
				{
					cout << "GREATER THAN 12"<< endl;
				}
			}
		}
		else
		{
			client.updateFile(OP::INS,fileid,cont,false);
			client.addfakefileid(fileid);
			//no keywords to insert for fake files
		}
                fileid++;
              }
             if (entry->d_type & DT_DIR) {
                if (strcmp (d_name, "..") != 0 &&
                  strcmp (d_name, ".") != 0) {
                 int path_length;
                   char path[PATH_MAX];
                     
                     path_length = snprintf (path, PATH_MAX,
                         "%s/%s", dir_name, d_name);
                         if (path_length >= PATH_MAX) {
                             fprintf (stderr, "Path length has got too long.\n");
                               exit (EXIT_FAILURE);
                                 }
                           list_dir (path, client, real);
                            }
                }
             }
    if (closedir (d)) {
       fprintf (stderr, "Could not close '%s': %s\n",
          dir_name, strerror (errno));
        exit (EXIT_FAILURE);
        }
    //return client;
}


void insertSingleFile(Client &client,string file)
{

      string cont = getFileContent(file);
      vector<string> kws1, kws;
      boost::split(kws1, cont, boost::is_any_of(delimiters));
      kws =  getUniquedWords(kws1, fileid);
      for (auto it = kws.begin(); it != kws.end(); it++)
      {
	      if(neg.find(*it)!=neg.end())
	      {
		 kws.erase(it--);
	      }
      }
      cout << endl <<file<< " " << fileid <<endl << endl;
      client.updateFile(OP::INS,fileid, cont, false);
      for(auto k: kws)
      {
      	int lenn = k.length();
      	if(lenn <= 12)
      	{
      		client.update(OP::INS,k, fileid,false);
      	}
      	else
      	{
      		cout << "GREATER THAN 12"<< endl;
      	}
      }
      fileid++;
}

void deleteSinglefile(Client &client, int fileid)
{
	vector <int> singlefile;
	singlefile.push_back(fileid);
	vector<string> file = client.searchfile(singlefile);
	for(auto cont : file)// there is one file though
	{
      		vector<string> kws1, kws;
      		boost::split(kws1, cont, boost::is_any_of(delimiters));
      		kws =  getUniquedWords(kws1, fileid);
     		for (auto it = kws.begin(); it != kws.end(); it++)
      		{
	      		if(neg.find(*it)!=neg.end())
	      		{
		 		kws.erase(it--);
	      		}
      		}
      		client.updateFile(OP::DEL,fileid, cont, false);
        	 for(auto k: kws)
        	 {
      			int lenn = k.length();
      			if(lenn <= 12)
      			{
      				client.update(OP::DEL,k, fileid,false);
      			}
        	 }
	 }
}

int main(int argc, char** argv) 
{
    bool usehdd = false, cleaningMode = false;
    Server server(usehdd, cleaningMode);
    Client client(&server, cleaningMode, 100, 300);
    //client.endSetup();    
	list_dir("tiny",client, FAKE);
	cout <<"==================fake files added=================="<<endl;
	list_dir("tiny3",client, REAL);
	cout << endl<<" SETUP INSERT DONE!"<< endl;
	cout <<"=================================="<< endl;
	cout <<"READY TO PERFORM QUERIES!" << endl;
	
	
	while(1)
	{
		char c;
		cout <<endl<<endl<<endl;
		cout <<"Enter your choice (s/i/d/p/q): "<<endl;
		cout <<"s/S: Search"<<endl;
		cout <<"i/I: Insert"<<endl;
		cout <<"d/D: Delete"<<endl;
		cout <<"p/P: Print"<<endl;
		cout <<"q/Q: Quit"<<endl;
		cout <<"------------------"<<endl;
		cout <<"_";
		cin >> c;
		if(c=='s' || c=='S')
		{
			cout << "Enter the keyword to be searched: ";
			string keyword;
			cin>> keyword;
   	pair<vector<int>,vector<string>>filespair = client.search(keyword);
			vector<int> files = filespair.first;
			vector<string> fakes = filespair.second;
			cout <<"--------Search result---------"<<endl;
	    		for(auto file:files)
			{
	    			cout << "["<<file<<"] ";
				//cout << file.second<< endl<<endl;
			}
	    		for(auto file:fakes)
			{
	    			cout << "["<<file<<"] ";
				//cout << file.second<< endl<<endl;
			}
	    		cout <<endl<<endl;
			cout << "RESULT size: " << files.size() << endl<<endl;
			vector<string> content = client.searchfile(files);
		}
		else if(c=='d'|| c=='D')
		{
			cout <<"Enter file id to be deleted: ";
			string idstr;
			cin>>idstr;
			bool flag = true;
			int len = idstr.length();
			for(int i=0;i<len;i++)
			{
				if(isdigit(idstr[i])== true)
				{
					continue;
				}
				else 
				{
					flag = false;
					break;
				}
			}
			if(flag == true)
			{
				int id = stoI(idstr);
				deleteSinglefile(client,id);
			}
			else
				cout <<"***invalid id (enter integer)***"<<endl;
		}
		else if(c=='i' || c=='I')
		{
			string file;
			cout << "Enter file path to be inserted:";
			cin>> file;
			insertSingleFile(client,file);
			cout <<endl;
		}
		else if(c=='p' || c=='P')
		{
			//mitra.print();
		}
		else //if(c=='q'||c=='Q')
		{
			cout <<"QUITTING..."<<endl<<endl;
			break;
		}
	}    
    return 0;
}
