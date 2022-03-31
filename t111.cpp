#include<iostream>
#include<vector>
#include<map>
#include<unordered_map>
#include<chrono>

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
	*/
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
	//reserve is faster
	return 0;
}
