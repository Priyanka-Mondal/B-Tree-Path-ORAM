#include "packorion/Oriel.h"
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
bool batch = true;//false;//true; // true makes the program crash
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
    string content((std::istreambuf_iterator<char>(file)), 
		                             std::istreambuf_iterator<char>());
    return content;
}


static void list_dir ( const char * dir_name, Oriel& oriel)
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
	          cout <<endl<<"=====================================" << endl;
                  cout <<file<< " " << fileid <<endl;
	     
	     	  oriel.insertWrap(cont,fileid,batch);
             
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
                         list_dir (path, oriel);
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
	if(argc<6)
	{
		cout <<"incorrect number of arguments"<<endl;
		return 0;
	}
	int size = to_int(argv[1]);
	int fsize = to_int(argv[2]);
	Oriel oriel(usehdd, size, fsize, local);  
	list_dir(argv[3],oriel);
	oriel.endSetup();
        ifstream kw;
	kw.open(argv[4]);
	string line;
        ofstream sres;
	sres.open(argv[5]);//,ios::app);	
	int l = 1;
	if(local)
	{
		while(getline(kw,line))
		{
        		auto start = high_resolution_clock::now();
			auto s = oriel.simplebatchSearch(line);
        		auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop-start);
			sres <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			cout <<l<<" "<< duration.count()<<" "<< s.size()<<endl;
			l++;
		}
	}
	else
	{
		while(getline(kw,line))
		{
        		auto start = high_resolution_clock::now();
			auto s = oriel.simplebatchSearch(line);//////
        		auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop-start);
			sres <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
			cout <<l<<" "<< duration.count()<<" "<< s.size()<<endl;
			l++;
		}
	}
	return 0;


/*
	list_dir("allen-p/deleted_items",oriel);
        auto start = high_resolution_clock::now();
	auto s = oriel.search("borion");
        auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<<" "<<s.size()<<endl;

	return 0;

	list_dir("allen-p/small_deleted_items",oriel);
        start = high_resolution_clock::now();
	s = oriel.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " "<<s.size()<<endl;

	list_dir("allen-p/sent",oriel);
        start = high_resolution_clock::now();
	s = oriel.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " " <<s.size()<<endl;

	list_dir("allen-p/all_documents",oriel);
        start = high_resolution_clock::now();
	s = oriel.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " "<<s.size()<<endl;

	list_dir("allen-p/discussion_threads",oriel);
        start = high_resolution_clock::now();
	s = oriel.search("oriel");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<< " "<<s.size()<<endl;


	return 0;


	
	cout <<"== TOTAL files inserted :"<<fileid-1<<" =="<<endl;
	cout <<"Time taken for setup(oriel):"<<duration.count()<<endl;
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
		cout <<"---------------Search result----------------"<<endl;
			start = high_resolution_clock::now();
	    	map<int,string> results=oriel.search(keyword);
			stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
		cout <<endl<< "search TIME: "<< duration.count()<<endl;  
		cout<<endl<<"RESULT SIZE:"<<results.size()<<endl<<endl;
		for(auto con: results)
		  cout<<"["<<con.first<<"]";
		cout<<endl;//":["<<con.second<<"]"<<endl;

		//cout <<endl<< "RESULT srchSIZE: " << files2.size() << endl;
			//sres<< duration.count() <<" "<<files.size()<<endl;
		}
		else if(c=='d'|| c=='D')
		{
			cout <<"Enter file id to be deleted: ";
			int fid;
			cin>>fid;
			start = high_resolution_clock::now();
			oriel.remove(fid);
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
			oriel.insertWrap(cont,fileid,false);
			stop = high_resolution_clock::now();
			duration = duration_cast<microseconds>(stop-start);
			cout << "Insertion time: "<< duration.count()<<endl;  
		cout <<"--TOTAL files inserted so far:"<<fileid<<endl;
			fileid++;
			cout <<endl;
		}
		else if(c=='p' || c=='P')
		{
			//oriel.print();
		}
		else if(c=='q'||c=='Q')
		{
			cout <<"QUITTING..."<<endl<<endl;
			break;
		}
		else
			cout <<"invalid choice!"<<endl;
	}    
*/
        return 0;
}
