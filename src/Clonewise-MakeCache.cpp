#include "Clonewise.h"
#include <mpi.h>
#include <cstdio>
#include <cstdlib>

#define R_TAG		100
#define RESULTS_TAG	101

std::vector<std::string> vPackages;

void
DoWork(const char *name, int index, int me)
{
	int c = 1;
	char myName[strlen(name) + 1];
	char *v[] = { myName, NULL };
	char s[1024];

	strcpy(myName, name);


	snprintf(s, sizeof(s), "/var/lib/Clonewise/distros/%s/cache/%s", distroString, name);
	outFd = fopen(s, "w+");

	RunClonewise(c, v);
	printf("# scanned %s\n", name);

	if (me != 0) {
		long size;
		char *result;
		int r[2];

		size = ftell(outFd);
		fseek(outFd, 0, SEEK_SET);
		result = new char[size];
		fread(result, 1, size, outFd);
		r[0] = index;
		r[1] = size;
		MPI_Send(r, 1, MPI_INT, 0, R_TAG, MPI_COMM_WORLD); 
		MPI_Send(result, size, MPI_CHAR, 0, RESULTS_TAG, MPI_COMM_WORLD); 
		delete [] result;
	}

	fclose(outFd), outFd = NULL;
}

int
main(int argc, char* argv[])
{ 
	int np, me; 
	const int root = 0;
	MPI_Status status;
 	std::map<std::string, std::list<std::string> >::const_iterator pIter;
	int *x, *y, xi, ysize;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);
   
	LoadEverything();
	outputFormat = CLONEWISE_OUTPUT_XML;

	x = new int[packages.size()];
	for (	pIter  = packages.begin(), xi = 0;
		pIter != packages.end();
		pIter++)
	{
		vPackages.push_back(pIter->first);
		x[xi++] = xi;
	}
	ysize = xi / np;
	y = new int[ysize];
	MPI_Scatter(x, ysize, MPI_INT, y, ysize, MPI_INT, root, MPI_COMM_WORLD); 
	if (me == 0) {
		for (int i = 0; i < ysize; i++) {
			DoWork(vPackages[y[i]].c_str(), y[i], me);
		}
		for (int i = ysize * np; i < xi; i++) {
			DoWork(vPackages[i].c_str(), x[i], me);
		}

		for (int yi = 0; yi < ysize; yi++) {
			for (int i = 1; i < np; i++) { 
				int r[2], size;
				char *result;
				FILE *f;
				char s[1024];

				MPI_Recv(r, 2, MPI_INT, i, R_TAG, MPI_COMM_WORLD, &status); 

				size = r[1];
				result = new char[size];

				MPI_Recv(result, size, MPI_CHAR, i, RESULTS_TAG, MPI_COMM_WORLD, &status); 

				snprintf(s, sizeof(s), "/var/lib/Clonewise/distros/%s/cache/%s", distroString, vPackages[x[r[0]]].c_str());
				f = fopen(s, "w");
				fwrite(result, 1, size, f);
				fclose(f), f = NULL;

				delete [] result;
			}
		}

		delete [] x; 
	} else {
		for (int i = 0; i < ysize; i++) {
			DoWork(vPackages[y[i]].c_str(), y[i], me);
		}
	} 
 
	delete [] y;
	MPI_Finalize(); 
	exit(0); 
}
