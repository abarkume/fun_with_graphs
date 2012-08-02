#include "graph.h"
#include <stdbool.h>

void print_graph(graph_info g)
{
	for (int i = 0; i < g.n; i++)
	{
		for (int j = 0; j < g.n; j++)
			printf("%d\t", g.distances[g.n*i + j]);
		printf("\n");
	}

	for (int i = 0; i < g.n; i++)
		printf("%d ", g.k[i]);
	printf("\n");

	unsigned m = (g.n + WORDSIZE - 1) / WORDSIZE;
	for(int i = 0; i < g.n; i++)
	{
		for(int j = 0; j < g.n; j++)
		{
			if(ISELEMENT(GRAPHROW(g.nauty_graph, i, m), j))
				printf("1, ");
			else
				printf("0, ");
		}
		printf("\n");
	}
	printf("\n");
	printf("K: %d, D: %d, S: %d, M: %d\n", g.max_k, g.diameter, g.sum_of_distances, g.m);
}


void floyd_warshall(graph_info g) {
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

void fill_dist_matrix(graph_info g)
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
	for (int i = 0; i < g.n-1; i++) {
		for (int j = i+1; j < g.n-1; j++) {
			int dist = g.distances[g.n*i + g.n-1] + g.distances[g.n*(g.n-1) + j];
			if(dist < g.distances[g.n*i + j]) {
				g.distances[g.n*i + j] = g.distances[g.n*j+i] = dist;
			}
		}
	}

}

void test_fill_dist_matrix(void)
{
	graph_info g;
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

int calc_sum(graph_info g)
{
	int sum = 0;
	for(int i = 0; i < g.n; i++)
		for(int j = i+1; j < g.n; j++)
		sum += g.distances[g.n*i+j];
	return sum;
}

int calc_diameter(graph_info g)
{
	int diameter = 0;
	for(int i = 0; i < g.n; i++)
		for(int j = i+1; j < g.n; j++)
			if(diameter < g.distances[g.n*i + j])
				diameter = g.distances[g.n*i + j];
	return diameter;
}

graph_info *graph_info_from_nauty(graph *g, int n)
{
	graph_info *ret = malloc(sizeof(graph_info));
	ret->n = n;
	ret->distances = malloc(n * n * sizeof(*ret->distances));
	ret->k = malloc(n * sizeof(*ret->k));

	int m = (n + WORDSIZE - 1) / WORDSIZE;
	ret->m = 0; //total number of edges
	for (int i = 0; i < n; i++) {
		ret->k[i] = 0;
		for (int j = 0; j < n; j++) {
			if(i == j)
				ret->distances[n*i + j] = 0;
			else if(ISELEMENT(g + i*m, j))
			{
				ret->distances[n*i + j] = 1;
				ret->k[i]++;
				ret->m++;
			}
			else
				ret->distances[n*i + j] = GRAPH_INFINITY;
		}
	}
	ret->m /= 2;
	
	ret->max_k = 0;
	for (int i = 0; i < n; i++)
		if (ret->k[i] > ret->max_k)
			ret->max_k = ret->k[i];
	
	floyd_warshall(*ret);
	ret->sum_of_distances = calc_sum(*ret);
	ret->diameter = calc_diameter(*ret);
	ret->nauty_graph = malloc(n * m * sizeof(graph));
	memcpy(ret->nauty_graph, g, n * m * sizeof(graph));
	
	return ret;
}

void graph_info_destroy(graph_info *g)
{
	free(g->distances);
	free(g->k);
	free(g->nauty_graph);
	free(g);
}


static void init_extended(graph_info input, graph_info *extended)
{
	extended->n = (input.n+1);
	
	int m = (input.n + WORDSIZE - 1) / WORDSIZE;
	int extended_m = (input.n + WORDSIZE)/WORDSIZE;
	
	extended->nauty_graph = malloc(extended->n * extended_m * sizeof(setword));
	
	for(int i = 0; i < input.n; i++)
	{
		for(int j = 0; j < m; j++)
			extended->nauty_graph[i*extended_m + j] = input.nauty_graph[i*m + j];
		if(extended_m > m)
			extended->nauty_graph[i*extended_m + m] = 0;
	}
	for(int i = 0; i < extended_m; i++)
		extended->nauty_graph[input.n*extended_m + i] = 0;

	extended->distances = malloc(extended->n*extended->n*sizeof(*extended->distances));
	for(int i = 0; i < input.n; i++)
		for(int j = 0; j < input.n; j++)
			extended->distances[(extended->n)*i + j] = input.distances[(input.n)*i + j];
	for(int i = 0; i < extended->n - 1; i++)
		extended->distances[(extended->n)*i+extended->n-1] =
		extended->distances[(extended->n)*(extended->n-1)+i] = GRAPH_INFINITY;
	extended->distances[extended->n*extended->n - 1] = 0;

	extended->k = (int*) malloc(extended->n*sizeof(int));
	for(int i = 0; i < input.n; i++)
		extended->k[i] = input.k[i];
	extended->k[input.n] = 0;

	extended->m = input.m;
	extended->max_k = input.max_k;
}

static graph_info *new_graph_info(graph_info src)
{
	graph_info *ret = (graph_info*) malloc(sizeof(graph_info));
	int m = (src.n + WORDSIZE - 1) / WORDSIZE;
	ret->n = src.n;
	ret->distances = malloc(ret->n * ret->n * sizeof(*ret->distances));
	ret->nauty_graph = malloc(ret->n * m * sizeof(setword));
	ret->k = malloc(ret->n * sizeof(*ret->k));
	ret->k = src.k;
	ret->m = src.m;
	ret->max_k = src.max_k;
	memcpy(ret->distances, src.distances, src.n * src.n * sizeof(*src.distances));
	memcpy(ret->nauty_graph, src.nauty_graph, src.n * m * sizeof(setword));
	return ret;
}

static void destroy_extended(graph_info extended)
{
	free(extended.distances);
	free(extended.nauty_graph);
	free(extended.k);
}

static void add_edges(graph_info *g, unsigned start, int extended_m)
{
	//setup m and k[n] for the children
	//note that these values will not change b/w each child
	//of this node in the search tree
	g->m++;
	g->k[g->n - 1]++;
	unsigned old_max_k = g->max_k;
	if(g->k[g->n - 1] > g->max_k)
		g->max_k = g->k[g->n - 1];
	
	//if the child has a node of degree greater than MAX_K,
	//don't search it
	if(g->k[g->n - 1] <= MAX_K)
	{
		for(unsigned i = start; i < g->n - 1; i++)
		{
			g->k[i]++;
			
			//same as comment above
			if(g->k[i] <= MAX_K)
			{
				unsigned old_max_k = g->max_k;
				if(g->k[i] > g->max_k)
					g->max_k = g->k[i];
				
				g->distances[g->n*i + (g->n-1)] = g->distances[g->n*(g->n-1) + i] = 1;
				ADDELEMENT(GRAPHROW(g->nauty_graph, i, extended_m), g->n-1);
				ADDELEMENT(GRAPHROW(g->nauty_graph, g->n-1, extended_m), i);
				
				add_edges(g, i + 1, extended_m);
				
				DELELEMENT(GRAPHROW(g->nauty_graph, i, extended_m), g->n-1);
				DELELEMENT(GRAPHROW(g->nauty_graph, g->n-1, extended_m), i);
				g->distances[g->n*i + (g->n-1)] = g->distances[g->n*(g->n-1) + i] = GRAPH_INFINITY;
				g->max_k = old_max_k;
			}
			g->k[i]--;
		}
	}
	
	//tear down values we created in the beginning
	g->max_k = old_max_k;
	g->m--;
	g->k[g->n - 1]--;


	if(g->k[g->n - 1] > 0)
	{
		graph_info *temporary = new_graph_info(*g);
		fill_dist_matrix(*temporary); 
		temporary->diameter = calc_diameter(*temporary); 
		temporary->sum_of_distances = calc_sum(*temporary); 
		print_graph(*temporary);
	}
}

void add_edges_and_transfer_to_queue(graph_info input)
{
	graph_info extended;
	init_extended(input, &extended);
	
	add_edges(&extended, 0, (extended.n + WORDSIZE - 1) / WORDSIZE);
}

void test_add_edges(void)
{
	graph_info g;
	int distances [25] = {
		0, 1, 1, 2, 2,
		1, 0, 2, 1, 3,
		1, 2, 0, 3, 1,
		2, 1, 3, 0, 4,
		2, 3, 1, 4, 0,
	};
	int m = (4 + WORDSIZE) / WORDSIZE;
	graph nauty_graph[m * 5];
	for (int i = 0; i < m * 5; i++)
		nauty_graph[i] = 0;
	ADDELEMENT(GRAPHROW(nauty_graph, 0, m), 1);
	ADDELEMENT(GRAPHROW(nauty_graph, 0, m), 2);
	ADDELEMENT(GRAPHROW(nauty_graph, 1, m), 0);
	ADDELEMENT(GRAPHROW(nauty_graph, 1, m), 3);
	ADDELEMENT(GRAPHROW(nauty_graph, 2, m), 0);
	ADDELEMENT(GRAPHROW(nauty_graph, 2, m), 4);
	ADDELEMENT(GRAPHROW(nauty_graph, 3, m), 1);
	ADDELEMENT(GRAPHROW(nauty_graph, 4, m), 2);
	
	g.distances = distances;
	g.nauty_graph = nauty_graph;
	g.n = 5;
	int g_k[5] = {2, 2, 2, 1 ,1};
	g.k = g_k;
	g.m = 4;
	g.max_k = 2;
	
	print_graph(g);

	add_edges_and_transfer_to_queue(g);
}
