#include "borion/BOrion.h"
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
bool usehdd = false;
bool batch = true; // true makes the program crash

int stoint(string updt_cnt)
{
        int updc;
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
        return updc;
}
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
    string content((std::istreambuf_iterator<char>(file)), 
		                             std::istreambuf_iterator<char>());
    return content;
}


static void list_dir ( const char * dir_name, BOrion& borion)
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
                  cout << endl <<file<< " " << fileid <<endl;
	     
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
                  	path_length=snprintf(path,PATH_MAX,"%s/%s", 
					                   dir_name, d_name);
                         if (path_length >= PATH_MAX) 
			 {
                                 fprintf (stderr, "Path length too long.\n");
                                 exit (EXIT_FAILURE);
                         }
                         list_dir (path, borion);
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
	int size = stoint(argv[1]);
	BOrion borion(usehdd, size);  
        ofstream sres;
	sres.open("vardbsearchborion.txt");//,ios::app);	
//	/*
        auto start = high_resolution_clock::now();
	list_dir(argv[2],borion);
        auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
//*/

/*
	list_dir("allen-p/deleted_items",borion);
        auto start = high_resolution_clock::now();
	auto s = borion.search("borion");
        auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<<" "<<s.size()<<endl;
	
	list_dir("allen-p/small_deleted_items",borion);
        start = high_resolution_clock::now();
	s = borion.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " "<<s.size()<<endl;

	list_dir("allen-p/sent",borion);
        start = high_resolution_clock::now();
	s = borion.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " " <<s.size()<<endl;

	list_dir("allen-p/all_documents",borion);
        start = high_resolution_clock::now();
	s = borion.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " "<<s.size()<<endl;

	list_dir("allen-p/discussion_threads",borion);
        start = high_resolution_clock::now();
	s = borion.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " "<<s.size()<<endl;

	return 0;
*/
	
	cout <<"== TOTAL files inserted :"<<fileid-1<<" =="<<endl;
	cout <<"Time taken for setup(borion):"<<duration.count()<<endl;
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
			map<string,string> files, files2;
		cout <<"---------------Search result----------------"<<endl;
			start = high_resolution_clock::now();
	    	vector<pair<string,string>> results=borion.setupsearch(keyword);
			stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
			cout << "setupsearch TIME: "<< duration.count()<<endl;  
			start = high_resolution_clock::now();
	    	vector<pair<string,string>> res2=borion.search(keyword);
			stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
			cout << "search TIME: "<< duration.count()<<endl;  
		for(auto file:results)
		{
			string id = file.second.substr(0,FID_SIZE);
			int sz = file.second.size();
			string cont = file.second.substr(FID_SIZE,sz);
			if(files.find(id)!=files.end())
			{
				string con = files.at(id);
				con.append(cont);
				files.erase(id);
				files.insert(pair<string,string>(id,con));
			}
			else
			{
				files.insert(pair<string,string>(id,cont));
			}
		}
		for(auto file : files)
		{
			cout <<"["<<file.first<<"]";
		}

		for(auto file:res2)
		{
			string id = file.second.substr(0,FID_SIZE);
			int sz = file.second.size();
			string cont = file.second.substr(FID_SIZE,sz);
			if(files2.find(id)!=files2.end())
			{
				string con = files2.at(id);
				con.append(cont);
				files2.erase(id);
				files2.insert(pair<string,string>(id,con));
			}
			else
			{
				files2.insert(pair<string,string>(id,cont));
			}
		}
	    		cout <<endl<<endl;
			cout << "RESULT SetupsearchSIZE: " << files.size() << endl<<endl;
			cout << "RESULT srchSIZE: " << files2.size() << endl<<endl;
			sres<< duration.count() <<" "<<files.size()<<endl;
		}
		else if(c=='d'|| c=='D')
		{
			cout <<"Enter file id to be deleted: ";
			int fid;
			cin>>fid;
			start = high_resolution_clock::now();
			borion.remove(toS(fid));
			stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
			cout << "Deletion time: "<< duration.count()<<endl;  
		}
		else if(c=='i' || c=='I')
		{
			cout << "Enter file name to be inserted:";
			string file;
			cin>> file;
	          	string cont = getFileContent(file);
			start = high_resolution_clock::now();
			borion.insertWrap(cont,toS(fileid),true);
			stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
			cout << "Insertion time: "<< duration.count()<<endl;  
		cout <<"--TOTAL files inserted so far:"<<fileid<<endl;
			fileid++;
			cout <<endl;
		}
		else if(c=='p' || c=='P')
		{
			//borion.print();
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
