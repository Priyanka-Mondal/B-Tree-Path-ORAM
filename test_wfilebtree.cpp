#include "wfilebtree/Orion.h"
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
	          //cout <<"============================================================" << endl;
                  //cout << file<< " " << fileid <<endl ;
	     
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
	if(argc <8)
	{
		cout <<"incorrect arguments! EXITING..."<<endl;
		cout <<"<execFile> <KwOramSize> <FileOramSize> " <<
		"<fileDir> <keywordFile> <outputFile> <speed> <latency-micro>"<<endl;
		return 0;
	}

	int sizekw = to_int(argv[1]);
	int sizefile = to_int(argv[2]);
	Orion orion(usehdd, sizekw,sizefile, local);  

        ifstream kw;
	ofstream sres;
	//cout <<"------------------"<<endl;
	
	kw.open(argv[4]);
	sres.open(argv[5]);	
	string speed1 = argv[6];
	string speed2="";
	speed2.assign(speed1.begin(),speed1.end()-2);
	string unit = "";
	unit.assign(speed1.end()-2,speed1.end());
	if(unit != "Mb")
	{
		cout <<"Give speed in MBits"<<endl;
		return 0;
	}
	list_dir(argv[3],orion);
	orion.endSetup();
	cout <<"FILES PARSED!"<<endl;
	double speed = double(to_int(speed2));
	speed = speed*1024*128; //MBits/sec to Bytes/sec
	double latency = double(to_int(argv[7]))/double(1000);
	cout <<"latency:"<<latency;
///////////////////////////////////////////////////////////////////////////
 	string first=argv[0];
	int b =1 ;
	while(b<argc)
	{
		first.append(" ");
		first.append(argv[b]);
		b++;
	}
	sres<<first<<endl;
///////////////////////////////////////////////////////////////////////////

cout <<endl<<"keyword||SearchTime||TotalTime||resultSize||indexBytes||fileBytes||rtt"<<endl;
sres <<"keyword||SearchTime||TotalTime||resultSize||indexBytes||fileBytes||rtt"<<endl;
	string line;
	if(local)
	{
		while(getline(kw,line))
		{
			auto s = orion.search(line,sres,speed,latency);
		}
	}
	//orion.remove("bbbb");
	cout <<"-------------------------------"<<endl;
/*	else
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
*/
	return 0;
	
	/*
	
	list_dir("allen-p/deleted_items",orion);
	list_dir("allen-p/sent_items",orion);
	//list_dir("allen-p/sent",orion);
	//list_dir("allen-p/all_documents",orion);
	//list_dir("allen-p/discussion_threads",orion);
	cout <<"SEARCHING..."<<endl;
        auto start = high_resolution_clock::now();
	auto s = orion.search("borion");
        auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<< duration.count()<<" "<< s.size()<<endl;
return 0;
	list_dir("allen-p/sent_items",orion);
        start = high_resolution_clock::now();
	s = orion.search("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<<duration.count() <<" "<< s.size()<<endl;

	list_dir("allen-p/sent",orion);
        start = high_resolution_clock::now();
	s = orion.searchsimple("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<<duration.count()<<" "<< s.size()<<endl;

	list_dir("allen-p/all_documents",orion);
        start = high_resolution_clock::now();
	s = orion.searchsimple("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<<duration.count()<<" "<< s.size()<<endl;

	list_dir("allen-p/discussion_threads",orion);
        start = high_resolution_clock::now();
	s = orion.searchsimple("borion");
        stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	sres <<(fileid-1)<<" "<<duration.count()<<" "<< s.size()<<endl;

	return 0;

*/
/*	
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
			cout <<"search size:"<<s<<endl;
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
			int id;
			//cin>>kw;
			cin>>id;
			//vector<string> kws;
			//kws.push_back(kw);
			orion.remove(id);
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
	*/
}
