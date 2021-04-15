
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <set>
using namespace std;


// Parameters
float lambda = 0.3;	//Taubin parameter lambda
float mu = -0.2;	//Taubin parameter mu
const char* fileName = "2CMP_noise.off";	//input OFF mesh file


typedef struct {
	float x;
	float y;
	float z;
}FLTVECT;

typedef struct {
	int a;
	int b;
	int c;
}INT3VECT;

typedef struct {
	int nv;
	int nf;
	FLTVECT *vertex;
	INT3VECT *face;
}SurFacemesh;

// Surface mesh obtained from .off file
SurFacemesh* surfmesh;

//vector of vectors for the number of neighbors
vector<vector<int>> neighbors;
float** p_i;

void readPolygon()
{
	int num, n, m;
	int a, b, c, d;
	float x, y, z;
	char line[256];
	FILE *fin;


	if ((fin = fopen(fileName, "r")) == NULL) {
		printf("read error...\n");
		exit(0);
	};

	/* OFF format */
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'O' && line[1] == 'F' && line[2] == 'F')
			break;
	}
	fscanf(fin, "%d %d %d\n", &m, &n, &num);

	surfmesh = (SurFacemesh*)malloc(sizeof(SurFacemesh));
	surfmesh->nv = m;
	surfmesh->nf = n;
	surfmesh->vertex = (FLTVECT *)malloc(sizeof(FLTVECT)*surfmesh->nv);
	surfmesh->face = (INT3VECT *)malloc(sizeof(INT3VECT)*surfmesh->nf);

	for (n = 0; n < surfmesh->nv; n++) {
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		surfmesh->vertex[n].x = x;
		surfmesh->vertex[n].y = y;
		surfmesh->vertex[n].z = z;
	}

	for (n = 0; n < surfmesh->nf; n++) {
		fscanf(fin, "%d %d %d %d\n", &a, &b, &c, &d);
		surfmesh->face[n].a = b;
		surfmesh->face[n].b = c;
		surfmesh->face[n].c = d;
		if (a != 3)
			printf("Errors: reading surfmesh .... \n");
	}
	fclose(fin);

	//vector of vectors to get all neighbors of given vertices
	vector<int> tmp;
	for(int i = 0; i < surfmesh->nv - 1; ++i){
		neighbors.push_back(tmp);
	}
}


void writePolygon()
{
	FILE *fp = fopen("output.off", "wb");
	if (!fp)
	{
		printf("\nOpenning file was unsuccessful . . . \n");
	}
	fprintf(fp, "OFF\n");
	
	fprintf(fp, "%d %d %d\n", surfmesh->nv, surfmesh->nf, 0);

	for (int n = 0; n<surfmesh->nv; n++) 
	{
		fprintf(fp, "%f %f %f\n", surfmesh->vertex[n].x, surfmesh->vertex[n].y, surfmesh->vertex[n].z);
	}

	for (int n = 0; n<surfmesh->nf; n++) 
	{
		fprintf(fp, "3 %d %d %d\n", surfmesh->face[n].a, surfmesh->face[n].b, surfmesh->face[n].c);
	}

	fclose(fp);
}

//helpr method to find all neighbors for each vertex
void getNeighbors(){
	INT3VECT tmp;
	int containa = 0;
	int containb = 0;
	int containc = 0;
	int a,b,c;
	for(int i = 0; i < surfmesh->nf; i++){
		//load the ith face from the surface mesh
		tmp = surfmesh->face[i];

		a = tmp.a;
		b = tmp.b;
		c = tmp.c;

		neighbors[a].push_back(b);
		neighbors[a].push_back(c);

		neighbors[b].push_back(a);
		neighbors[b].push_back(c);

		neighbors[c].push_back(a);
		neighbors[c].push_back(b);
	}

	float xAvg = 0.0;
	float yAvg = 0.0;
	float zAvg = 0.0;
	int numVerts = 0;

	p_i = new float*[surfmesh->nv]; //FLTVECT[surfmesh->nv];
	for (int i = 0; i < surfmesh->nv; i++) {
		p_i[i] = new float[3];
	}

	for(int i = 0; i < surfmesh->nv; ++i){
		for(int j = 0; j < neighbors[i].size(); ++j){
			++numVerts;
			xAvg = xAvg + surfmesh->vertex[neighbors[i][j]].x;
			yAvg = yAvg + surfmesh->vertex[neighbors[i][j]].y;
			zAvg = zAvg + surfmesh->vertex[neighbors[i][j]].z;
		}

		//average of all neighbors
		p_i[i][0] = (xAvg/numVerts);
		p_i[i][1] = (yAvg/numVerts);
		p_i[i][2] = (zAvg/numVerts);

		p_i[i][0] = p_i[i][0] - surfmesh->vertex[i].x;
		p_i[i][1] = p_i[i][1] - surfmesh->vertex[i].y;
		p_i[i][2] = p_i[i][2] - surfmesh->vertex[i].z;

		xAvg = 0.0;
		yAvg = 0.0; 
		zAvg = 0.0;
		numVerts = 0;
	}
}

void smooth()
{
	getNeighbors();
	
	for(int i = 0; i < surfmesh->nv; ++i){
		// shrink step
		surfmesh->vertex[i].x = surfmesh->vertex[i].x + (lambda * p_i[i][0]);
		surfmesh->vertex[i].y = surfmesh->vertex[i].y + (lambda * p_i[i][1]);
		surfmesh->vertex[i].z = surfmesh->vertex[i].z + (lambda * p_i[i][2]);

		//inflate step
		surfmesh->vertex[i].x = surfmesh->vertex[i].x + (mu * p_i[i][0]);
		surfmesh->vertex[i].y = surfmesh->vertex[i].y + (mu * p_i[i][1]);
		surfmesh->vertex[i].z = surfmesh->vertex[i].z + (mu * p_i[i][2]);
	}
	
	
}


int main(int argc, char *argv[])
{
    readPolygon();
	
	for(int i = 0; i < 25; ++i){
		printf("smooth #%d\n", i+1);
		smooth();
	}

	writePolygon();

	return 0;
}
