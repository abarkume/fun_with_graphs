#include "graph.h"


void floyd_warshall(distance_matrix g) {
	for (int k = 0; k < g.n; k++) {
		for (int i = 0; i < g.n; i++) {
			for (int j = 0; j < g.n; j++) {
				int dist = g.distances[g.n*i + k] + g.distances[g.n*k + j];
				if(dist < g.distances[g.n*i + j]) {
					g.distances[g.n*i + j] = dist;
				}
			}
		}
	}
}

void fill_dist_matrix(distance_matrix g)
{
	//Figure out distance from new node to each other node
	for(int i = 0; i < g.n-1; i++)
	{
		if(g.distances[g.n*i + g.n-1] == GRAPH_INFINITY)
		{
			int min_dist = GRAPH_INFINITY;
			for(int j = 0; j < g.n-1; j++)
				if(g.distances[g.n*(g.n-1) + j] == 1 
				   && g.distances[g.n*j + i] + 1 < min_dist)
					 min_dist = g.distances[g.n*j + i] + 1;
			g.distances[g.n*(g.n-1) + i] = g.distances[g.n*i + g.n-1] = min_dist;
		}
	}
	
	//One iteration of Floyd-Warshall with k = g.n - 1
	for (int i = 0; i < g.n; i++) {
		for (int j = 0; j < g.n; j++) {
			int dist = g.distances[g.n*i + g.n-1] + g.distances[g.n*g.n-1 + j];
			if(dist < g.distances[g.n*i + j]) {
				g.distances[g.n*i + j] = dist;
			}
		}
	}
		
}

void test_fill_dist_matrix(void)
{
	distance_matrix g;
	int distances[9] = {
		GRAPH_INFINITY, 1, 1,
		1, GRAPH_INFINITY, GRAPH_INFINITY,
		1, GRAPH_INFINITY, GRAPH_INFINITY
	};
	g.distances = distances;
	g.n = 3;
	
	fill_dist_matrix(g);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			printf("%d, ", g.distances[i*3 + j]);
		printf("\n");
	}
}

int Binomial[MAXN][MAXN*(MAXN-1)/2];
void initiate_Binomial()
{
	for(int i = 1; i <= MAXN; i++)
	Binomial[i][0] = Binomial[i][i] = 1;

	for(int i = 1; i <= MAXN; i++)
		for(int j = 1;  j <= (MAXN-i); j++)
			Binomial[i][j] = Binomial[i-1][j-1] + Binomial[i-1][j];
}

void put_into_queue(distance_matrix g, int m);
void next_combination(distance_matrix g, distance_matrix replacement);

void add_edges_and_transfer_to_queue(distance_matrix input, int original_edges, int edges_added)
{
	int total_edges = original_edges + edges_added;
	distance_matrix extended;
	extended.n = (input.n+1);
	extended.distances = (int*) malloc(extended.n*extended.n*sizeof(*extended.distances));
	for(int i = 0; i < input.n; i++)
		for(int j = 0; j < input.n; j++)
			extended.distances[(extended.n)*i + j] = input.distances[(input.n)*i + j];
	for(int i = edges_added; i < extended.n; i++)
		extended.distances[(extended.n)*i+extended.n-1] = extended.distances[(extended.n)*(extended.n-1)+i] = GRAPH_INFINITY;
	

	for(int i = 0; i < edges_added; i++)
		extended.distances[extended.n*(extended.n-1)+i] = extended.distances[extended.n*i + extended.n - 1] = 1;

	int count = 0;
	while(count < Binomial[extended.n - 1][edges_added])
	{
		fill_dist_matrix(extended);
//		put_into_queue(extended,total_edges);
		next_combination(extended,input);
		count++;
	}
}

void next_combination(distance_matrix g, distance_matrix replacement)
{
	for(int i = 0; i < g.n; i++)
		if((g.distances[g.n*i+g.n-1] == 1) && (g.distances[g.n*(i+1)+g.n-1] != 1))
		{
			int previous_edges = 0;
			for(int j = 0; j < i; j++)
				if(g.distances[g.n*j + g.n-1] == 1)
					previous_edges++;

			for(int j = 0; j < previous_edges; j++)
				g.distances[g.n*j + g.n - 1] = g.distances[g.n*(g.n-1) + j] = 1;
			for(int j = previous_edges; j <= i; j++)
				g.distances[g.n*j + g.n - 1] = g.distances[g.n*(g.n-1) + j] = GRAPH_INFINITY;
			g.distances[g.n*(i+1) + g.n-1] = g.distances[g.n*(g.n-1) + i + 1] = 1;
			for(int j = i+2; j < g.n - 1; j++)
				if(g.distances[g.n*j + g.n-1] != 1)
					g.distances[g.n*j + g.n-1] = g.distances[g.n*(g.n-1) + j] = GRAPH_INFINITY;

			for(int i = 0; i < replacement.n; i++)
				for(int j = 0; j < replacement.n; j++)
					g.distances[g.n*i+j] = replacement.distances[replacement.n*i+j];
			break;
		}
}
