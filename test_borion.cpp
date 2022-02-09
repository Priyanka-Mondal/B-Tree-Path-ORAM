#include "borion/BOrion.h"
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>

using namespace std;

int fileid = 1;
bool usehdd = true;
BOrion borion(usehdd, 6000);  
bool batch = true;

string toS(int id)
{
	string s = to_string(id);
	string front ="";
	if (id < 10)
		front = "0000000";
	else if(id < 100)
		front = "000000";
	else if(id < 1000)
		front = "00000";
	else if(id < 10000)
		front = "0000";
	else if(id < 100000)
		front = "000";
	else if(id < 1000000)
		front = "00";
	else if(id < 10000000)
		front = "0";
	s=front.append(s);
	return s;
}

string getFileContent(string path)
{
    ifstream file(path);
    string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}


static void list_dir (const char * dir_name)
{
    DIR * d;
    d = opendir (dir_name);

   if (! d) 
   {
       fprintf (stderr, "Cant open '%s': %s\n", dir_name, strerror (errno));
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
          //printf ("%s/%s\n", dir_name, d_name);
            if (! (entry->d_type & DT_DIR)) 
	    {
	          string file = dir_name;
	          file = file.append("/");
	          file = file.append(d_name);
	          vector<string> kws1, kws;
	          string cont = getFileContent(file);
	          cout <<"=====================================" << endl;
                  cout << endl <<file<< " " << fileid <<endl << endl;
	     
	     	  borion.insertWrap(cont,toS(fileid),batch);
             
	     	  fileid++;
             }
             if (entry->d_type & DT_DIR) 
	     {
                if (strcmp (d_name, "..") != 0 &&   strcmp (d_name, ".") != 0) 
		{
			cout <<"HERE root 1: " << d_name<< endl ;
                        int path_length;
                        char path[PATH_MAX];
                  path_length=snprintf(path,PATH_MAX,"%s/%s", dir_name, d_name);
                         if (path_length >= PATH_MAX) 
			 {
                                 fprintf (stderr, "Path length too long.\n");
                                 exit (EXIT_FAILURE);
                         }
                         list_dir (path);
                 }
              }
     }
    if (closedir (d)) 
    {
    	fprintf (stderr, "Couldnt close '%s': %s\n",dir_name, strerror(errno));
        exit (EXIT_FAILURE);
    }
}


int main(int, char**) 
{

	//list_dir("allen-p/deleted_items");
	//list_dir("allen-p");
	//list_dir("may-l");
	list_dir("tiny");
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
	    		map<string,string> files= borion.searchWrapper(keyword);
			cout <<"--------Search result---------"<<endl;
	    		for(auto file:files)
			{
	    			cout << "["<<file.first<<"] ";
				//cout << file.second<< endl<<endl;
			}
	    		cout <<endl<<endl;
			cout << "RESULT size: " << files.size() << endl<<endl;
		}
		else if(c=='d'|| c=='D')
		{
			cout <<"Enter file id to be deleted: ";
			int fid;
			cin>>fid;
			borion.remove(toS(fid));
		}
		else if(c=='i' || c=='I')
		{
			cout << "Enter file name to be inserted:";
			string file;
			cin>> file;
	          	string cont = getFileContent(file);
			borion.insertWrap(cont,toS(fileid),batch);
			fileid++;
			cout <<endl;
		}
		else if(c=='p' || c=='P')
		{
			//borion.print();
		}
		else //if(c=='q'||c=='Q')
		{
			cout <<"QUITTING..."<<endl<<endl;
			break;
		}
	}    
        return 0;
}
