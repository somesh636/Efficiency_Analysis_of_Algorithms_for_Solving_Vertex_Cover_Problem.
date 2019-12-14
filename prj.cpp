#include <memory>//defined std::unique_ptr
#include "minisat/core/SolverTypes.h"// defines Var and Lit
#include "minisat/core/Solver.h"// defines Solver
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <vector>
#include <time.h>
#include <map>
#include <algorithm>

using namespace std;
using namespace Minisat;

sem_t semaphoreIO;
sem_t semaphore;

int n;
int edges;
int** E;

int indexSAT = 0;
int indexVC1 = 0;
int indexVC2 = 0;
int* vertex_coverSAT;
int* vertex_coverVC1;
int* vertex_coverVC2;
std::vector<int> VC1_Vect;

bool flag = false;


void *CNF_SAT_VC(void*)
{
	bool res;
	if (edges == 0)
	{
		return 0;
	}

	for (int k = 1;k <= n;k++)
	{
		unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
		Lit** X;
		X = new Lit*[n];
		for (int i = 0;i < n;i++)
		{
			X[i] = new Lit[k];
		}

		//initialize
		for (int i = 0;i < n;i++)
		{
			for (int j = 0;j <k;j++)//k=[0,n]
			{
				Lit l;
				l = Minisat::mkLit(solver->newVar());
				X[i][j] = l;
			}
		}

		//step1
		for (int j = 0;j <k;j++)
		{
			Minisat::vec<Minisat::Lit> constr1;
			for (int i = 0;i <n;i++)
			{
				constr1.push(X[i][j]);
			}
			solver->addClause(constr1);
		}


		//step2
		for (int m = 0;m <n;m++)
		{
			for (int p = 0;p <(k - 1);p++)
			{
				for (int q = p + 1;q <k;q++)
				{
					Minisat::vec<Minisat::Lit> constr2;
					constr2.push(~X[m][p]);
					constr2.push(~X[m][q]);
					solver->addClause(constr2);
				}
			}
		}

		//step3
		for (int m = 0;m <k;m++)
		{
			for (int p = 0;p <(n - 1);p++)
			{
				for (int q = p + 1;q <n;q++)
				{
					Minisat::vec<Minisat::Lit> constr3;
					constr3.push(~X[p][m]);
					constr3.push(~X[q][m]);
					solver->addClause(constr3);
				}
			}
		}

		//step4
		for (int e = 0;e <edges;e++)
		{
			int v1 = E[e][0];
			int v2 = E[e][1];
			Minisat::vec<Minisat::Lit> constr4;
			for (int j = 0;j <k;j++)
			{
				constr4.push(X[v1][j]);
				constr4.push(X[v2][j]);
			}
			solver->addClause(constr4);
		}

		res = solver->solve();//1-success   0-fail

		if (res == 1)
		{
			//int* vertex_cover;
			indexSAT = 0;
			vertex_coverSAT = new int[n];

			for (int i = 0;i < n;i++)
			{
				for (int j = 0;j < k;j++)
				{
					if (Minisat::toInt(solver->modelValue(X[i][j])) == 0)
					{
						vertex_coverSAT[indexSAT] = i;
						indexSAT++;
					}
				}
			}
			break;
		}
	}
}


bool no_edges(int** mMatrix, int n1)
{
	for (int i = 0; i < n1; i++)
	{
		for (int j = 0; j < n1; j++)
		{
			if (mMatrix[i][j] == 1)
			{
				return false;
			}
		}
	}
	return true;
}


void *APPROX_VC_1(void*)
{
	//int* vertex_cover;
	vertex_coverVC1 = new int[n];
	indexVC1 = 0;

	//initialize
	int** mMatrix;
	mMatrix = new int*[n];
	for (int i = 0; i < n; i++)
	{
		mMatrix[i] = new int[n];
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			mMatrix[i][j] = 0;
		}
	}

	for (int i = 0; i < edges; i++)
	{
		int p1 = E[i][0];
		int p2 = E[i][1];
		mMatrix[p1][p2] = 1;
		mMatrix[p2][p1] = 1;
	}

	while (no_edges(mMatrix, n) == false)
	{
		//record how many vertices are connected to this vertex
		int* count;
		count = new int[n];
		for (int i = 0;i < n;i++)
		{
			count[i] = 0;
		}

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (mMatrix[i][j] == 1)
				{
					count[i] += 1;
				}
			}
		}

		//find the index of the vertex with highest degree
		int max = count[0];
		int v = 0;
		for (int i = 0; i < n; i++)
		{
			if (count[i] > max)
			{
				max = count[i];
				v = i;
			}
		}
		//add it to vertex cover
		vertex_coverVC1[indexVC1] = v;
		indexVC1++;

		//throw away all edges incident on that vertex
		for (int i = 0; i < n; i++)
		{
			mMatrix[v][i] = 0;
			mMatrix[i][v] = 0;
		}
	}

}

void *APPROX_VC_2(void*)
{
	vector <int> result_flag;

	for (int i = 0; i < edges; i++)
	{
		result_flag.push_back(0);
	}

	vector<int> result_vector;
	for (int i = 0; i < edges; i++)
	{
		int p = E[i][0];
		int q = E[i][1];
		if (result_flag[i] == 0) {
			for (int j = i + 1; j < edges; j++) {
				if (p == E[j][0] || q == E[j][0]
					|| q == E[j][1] || p == E[j][1]) {
					result_flag[j]++;
				}
			}
		}
	}

	for (int i = 0; i < edges; i++) {
		if (result_flag[i] == 0) {
			result_vector.push_back(E[i][0]);
			result_vector.push_back(E[i][1]);
		}
	}

	sort(result_vector.begin(), result_vector.end());
	vertex_coverVC2 = new int[n];
	indexVC2 = result_vector.size();
	for (int i = 0; i < result_vector.size(); i++)
	{
		vertex_coverVC2[i] = result_vector[i];
	}
}


void *IO(void*)
{
	//int** E_tmp;

	while (!cin.eof())
	{
		char command;
		cin.clear();
		cin.sync();
		cin.get(command);

		switch (command)
		{
		case 'V':
		{
			string content;
			cin >> content;
			n = stoi(content);
			break;
		}
		case 'E':
		{
			int** E_tmp;
			string content;
			cin >> content;
			int size = content.size();
			E_tmp = new int*[size];//At first, we don't know there are how many edges
			for (int i = 0;i < size;i++)
			{
				E_tmp[i] = new int[2];
				E_tmp[i][0] = 0;
				E_tmp[i][1] = 0;
			}

			int l = content.find('{');
			int r = content.find('}');
			string content2 = content.substr(l + 1, r - l - 1);

			int l1 = content2.find('<');
			int r1 = content2.find('>');
			int comma = content2.find(',', l1);

			int edge_tmp = 0;

			while (r1 < content2.length() + 1)
			{
				string n1 = content2.substr(l1 + 1, comma - l1 - 1);
				int n1_i = stoi(n1);
				string n2 = content2.substr(comma + 1, r1 - comma - 1);
				int n2_i = stoi(n2);

				if (n1_i >= n || n2_i >= n || n1_i < 0 || n2_i < 0)
				{
					cerr << "Error: index exceeds vertex number" << endl;
				}

				if (n1_i != n2_i)
				{
					E_tmp[edge_tmp][0] = n1_i;
					E_tmp[edge_tmp][1] = n2_i;
					edge_tmp++;
				}

				l1 = content2.find('<', r1 + 1);
				r1 = content2.find('>', r1 + 1);
				comma = content2.find(',', l1);
			}
			edges = edge_tmp;
			E = new int*[edges];
			for (int i = 0;i < edges;i++)
			{
				E[i] = new int[2];
				E[i][0] = E_tmp[i][0];
				E[i][1] = E_tmp[i][1];
			}
			sem_post(&semaphore);


			sem_wait(&semaphoreIO);
			//print out

			cout << "CNF-SAT-VC: ";
			for (int i = 0;i < indexSAT - 1;i++)
			{
				cout << vertex_coverSAT[i] << ",";
			}
			cout << vertex_coverSAT[indexSAT - 1] << endl;

			for(int a = 0; a < indexVC1 ; a++)
			{
				int value = vertex_coverVC1[a];
				VC1_Vect.push_back(value);
			}
			sort(VC1_Vect.begin(), VC1_Vect.end());

			cout << "APPROX-VC-1: ";
			for (int i = 0;i < indexVC1 - 1; i++)
			{
				cout << VC1_Vect[i] << "," ;
			}
			cout << VC1_Vect[indexVC1 - 1] << endl;
			VC1_Vect.clear();

			cout << "APPROX-VC-2: ";
			for (int i = 0;i < indexVC2 - 1;i++)
			{
				cout << vertex_coverVC2[i] << ",";
			}
			cout << vertex_coverVC2[indexVC2 - 1] << endl;

			break;
		}
		default:
		{
			break;
		}
		}
	}
	if (cin.eof())
	{
		flag = true;
		sem_post(&semaphore);
	}

}

int main(void)
{
	sem_init(&semaphoreIO, 0, 0);

	pthread_t threadIO;
	pthread_t threadSAT;
	pthread_t threadVC1;
	pthread_t threadVC2;

	pthread_create(&threadIO, NULL, &IO, NULL);

	while (true)
	{
		sem_init(&semaphore, 0, 0);
		sem_wait(&semaphore);

		if (flag == true)
		{
			return 0;
		}

		pthread_create(&threadSAT, NULL, &CNF_SAT_VC, NULL);
		pthread_create(&threadVC1, NULL, &APPROX_VC_1, NULL);
		pthread_create(&threadVC2, NULL, &APPROX_VC_2, NULL);

		pthread_join(threadSAT, NULL);
		pthread_join(threadVC1, NULL);
		pthread_join(threadVC2, NULL);

		sem_post(&semaphoreIO);
	}
	pthread_join(threadIO, NULL);
}
