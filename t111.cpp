#include<iostream>
#include<string.h>
#include<vector>
#include<map>
#include<unordered_map>
#include<chrono>
#include<algorithm>
#include <boost/algorithm/string.hpp>
#include "src/utils/Utilities.h"

using namespace std::chrono;
using namespace std;
using pr = pair<int,int>;
int main(int argc, char** argv) 
{
/*
	vector<int> vec;
	for(int k=1; k<=25; k++)
	{
	auto start = high_resolution_clock::now();
	for(int i =0; i<=1000; i++)
	{
		vec.clear();
		for(int j =0 ; j<=10000;j++)
		{
			vec.push_back(j);
		}
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	cout <<"clear:"<< duration.count()<<endl;


	start = high_resolution_clock::now();
	for(int i =0; i<=1000; i++)
	{
		vector<int> vec;
		for(int j =0 ; j<=10000;j++)
		{
			vec.push_back(j);
		}
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	cout <<"new:"<< duration.count()<<endl<<endl<<endl;

	}
	// vec.clear is faster than new vector allocation.************************************
*/
	/*
	vector<int> vec;
	for(int k=1; k<=25; k++)
	{
	auto start = high_resolution_clock::now();
	for(int i =0; i<=1000; i++)
	{
		for(int j =0 ; j<=10000;j++)
		{
			vec.push_back(j);
		}
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	cout <<"push:"<< duration.count()<<endl;


	vector<int> vec2;
	start = high_resolution_clock::now();
	for(int i =0; i<=1000; i++)
	{
		for(int j =0 ; j<=10000;j++)
		{
			vec2.emplace_back(j);
		}
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	cout <<"empl:"<< duration.count()<<endl<<endl<<endl;

	}
	//push is faster than emplace***************
	*/
	/*
	vector<int> vec;
	for(int k=1; k<=25; k++)
	{
	for(int i =0; i<=1000; i++)
	{
		for(int j =0 ; j<=10000;j++)
		{
			vec.push_back(j);
		}
	}

	auto start = high_resolution_clock::now();
	for(auto it=vec.begin();it!=vec.end();it++)
	{
		cout<<"";
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	cout <<"pointer:"<< duration.count()<<endl;

	 start = high_resolution_clock::now();
	for(auto it:vec)
	{
		cout<<"";
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	cout <<"element:"<< duration.count()<<endl;
	}
	//element is faster
	vector<int> vec;
	vec.reserve(10000000);
	for(int k=1; k<=25; k++)
	{
	auto start = high_resolution_clock::now();
	for(int i =0; i<=1000; i++)
	{
		for(int j =0 ; j<=10000;j++)
		{
			vec.push_back(j);
		}
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	cout <<"reserve:"<< duration.count()<<endl;


	vector<int> vec2;
	start = high_resolution_clock::now();
	for(int i =0; i<=1000; i++)
	{
		for(int j =0 ; j<=10000;j++)
		{
			vec2.push_back(j);
		}
	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	cout <<"notrese:"<< duration.count()<<endl<<endl<<endl;

	}
	*/
	//reserve is faster
	//
	//
	/*
	for(int k=1;k<=25;k++)
	{
	auto start = high_resolution_clock::now();
	for(int i=0; i<=100000000;i++)
	{
		int y = i*1000;

	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	cout <<"repeat:"<< duration.count()<<endl;
	int y;
	start = high_resolution_clock::now();
	for(int i=0; i<=100000000;i++)
	{
		y = i*1000;

	}
	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop-start);
	cout <<"single:"<< duration.count()<<endl<<endl<<endl;
	}
	vector<int> vec1;
	vector<int> vec2;
	vector<string> vec3;
	for(int i =0; i<=1000; i++)
	{
		for(int j =0 ; j<=10000;j++)
		{
			vec1.push_back(j+i);
			vec2.push_back(j+i);
			vec3.push_back(to_string(j)+"-"+to_string(i));

		}
	}
	for(int k1 =1;k1<=20;k1++)
	{
		string k = vec3[k1+100];
		auto start = high_resolution_clock::now();
		auto parts = Utilities::splitData(k, "-");
                int ac = stoi(parts[0]);
                int bc = stoi(parts[1]);
                string newstr = to_string(ac+1)+"-"+to_string(bc);
		vec3[k1+100]=newstr;
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop-start);
		cout <<"str:"<< duration.count()<<endl;

 		start = high_resolution_clock::now();
		ac = vec1[k1+3000];
		bc = vec2[k1+200];
		vec1[k1+3000]= ac+1;
		stop = high_resolution_clock::now();
		duration = duration_cast<microseconds>(stop-start);
		cout <<"vec:"<< duration.count()<<endl;
	}*/

	std::array<std::string,1> t;
	t[0]="qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq";
	t[0].c_str();
	cout <<sizeof(t[0]);
	return 0;
}
