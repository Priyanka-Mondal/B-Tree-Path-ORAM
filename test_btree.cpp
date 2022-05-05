#include "btree/Orion.h"
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>
#include<chrono>

using namespace std;
using namespace std::chrono;

int fileid = 1;
bool usehdd = true;
bool batch = false;
bool local = true;

int to_int(string updt_cnt)
{
        int updc;
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
        return updc;
}

string getFileContent(string path)
{
    ifstream file(path);
    string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}


static void list_dir (const char * dir_name, Orion& orion)
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
	          string cont = getFileContent(file);
	          cout <<"============================================================" << endl;
                  cout << file<< " " << fileid <<endl ;
	     
	     	  orion.insertWrap(cont,fileid, batch);
	     	  fileid++;
             }
             if (entry->d_type & DT_DIR) 
	     {
                if (strcmp (d_name, "..") != 0 &&   strcmp (d_name, ".") != 0) 
		{
                        int path_length;
                        char path[PATH_MAX];
                  path_length=snprintf(path,PATH_MAX,"%s/%s", dir_name, d_name);
                         if (path_length >= PATH_MAX) 
			 {
                                 fprintf (stderr, "Path length too long.\n");
                                 exit (EXIT_FAILURE);
                         }
                         list_dir (path, orion);
                 }
              }
     }
    if (closedir (d)) 
    {
    	fprintf (stderr, "Couldnt close '%s': %s\n",dir_name, strerror(errno));
        exit (EXIT_FAILURE);
    }
}


int main(int argc, char**argv) 
{
	int sizekw = to_int(argv[1]);
	Orion orion(usehdd, sizekw, local);  

        ifstream kw;
	ofstream sres;
	list_dir(argv[2],orion);
	orion.endSetup();
	//return 0;
	//cout <<"------------------"<<endl;
	
	kw.open(argv[3]);
	sres.open(argv[4]);	
	string line;
	int l = 1;
	if(local)
	{
		while(getline(kw,line))
		{
	        	auto start = high_resolution_clock::now();
			cout <<"before-----------------"<<endl;
			auto s = orion.batchSearch(line);
	        	auto stop = high_resolution_clock::now();
			auto ss = orion.search(line);
			//assert(s.size() == ss.size());
			auto duration = duration_cast<microseconds>(stop-start);
			cout <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			sres <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			cout<<"..........................................."<<endl;
			//orion.remove(line);
			cout <<endl;
			cout <<"after-----------------"<<endl;
	        	start = high_resolution_clock::now();
			s = orion.search(line);
	        	stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
			sres <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			cout <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			cout <<"====================================="<<endl;
			cout <<"====================================="<<endl;
			l++;
		}
	}
	
	/*
	else
	{
		while(getline(kw,line))
		{
	        	auto start = high_resolution_clock::now();
			auto s = orion.batchSearch(line);
	        	auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop-start);
			sres <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			cout <<l<<" "<< duration.count()<<" "<< s.size()<<endl;
			l++;
		}
	}
	return 0;
	
*/
return 0;
	
	cout <<"== TOTAL files inserted :"<<fileid-1<<" =="<<endl;
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
			cout <<"SEARCHING ..."<<endl;
			//start = high_resolution_clock::now();
			auto s = orion.search(keyword);
			cout <<"search size:"<<s.size()<<endl;
			//stop = high_resolution_clock::now();
			//duration = duration_cast<microseconds>(stop-start);
			//sres<<fileid-1<<" "<<duration.count()<<" "<<files.size();
	    		//for(auto file:files)
			//{
	    		//	cout << "["<<file.first<<"] ";
				//cout << file.second<< endl<<endl;
			//}
			//sres<<duration.count()<<" "<< files.size()<<endl;
		}
		else if(c=='i' || c=='I')
		{
			cout << "Enter file name to be inserted:";
			string file;
			cin>> file;
	          	string cont = getFileContent(file);
			//start = high_resolution_clock::now();
			orion.insertWrap(cont,fileid, false);
			//stop = high_resolution_clock::now();
			//duration = duration_cast<microseconds>(stop-start);
		cout <<"--TOTAL files inserted so far:"<<fileid<<endl;
			//cout << "Insertion time: "<< duration.count()<<endl;  
			fileid++;
			cout <<endl;
		}
		
		else if(c=='d' || c=='D')
		{
			cout <<"insert KEYWORD and FILEID"<<endl;
			string kw;
			//int id;
			cin>>kw;
			//cin>>id;
			//vector<string> kws;
			//kws.push_back(kw);
			orion.remove(kw);
		}
		else if(c=='p' || c=='P')
		{
			//orion.print();
		}
		else if(c=='q'||c=='Q')
		{
			cout <<"QUITTING..."<<endl<<endl;
			break;
		}
		else
			cout <<"invalid choice!"<<endl;
	}   
        return 0;
}
