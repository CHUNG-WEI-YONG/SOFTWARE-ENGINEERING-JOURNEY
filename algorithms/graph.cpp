#include <iostream>

struct AdjNode{
    int vertex;
    AdjNode* next;
};

struct Graph{
    int num_of_vertex;
    AdjNode** adj;
};

Graph* createGraph(int v){
    Graph* graph=new Graph;
    graph->num_of_vertex=v;
    graph->adj=new AdjNode*[v];

    for (int i=0;i<v;i++){
        graph->adj[i]=nullptr;
    }
    return graph;

}

void addEdge(Graph* graph,int src,int desk){
    //add new node on the head of the elemnt in list , for example 1:a->b->c
    AdjNode* newNode=new AdjNode;
    newNode->vertex=desk;
    newNode->next=graph->adj[src];
    graph->adj[src]=newNode;

    AdjNode* sNode=new AdjNode;
    sNode->vertex=src;
    sNode->next=graph->adj[desk];
    graph->adj[desk]=sNode;
   
}

void printGraph(Graph* graph){
    for (int i=0;i<graph->num_of_vertex;i++){
        std::cout<<i<<" : ";
        AdjNode* curr=graph->adj[i];
        while (curr!=nullptr){
            std::cout<<curr->vertex<<" , ";
            curr=curr->next;
        }
        std::cout<<"Null\n";
    }
}

int main() {
    Graph* graph = createGraph(5);

    addEdge(graph, 0, 1);
    addEdge(graph, 0, 4);
    addEdge(graph, 1, 2);
    addEdge(graph, 1, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 3);
    addEdge(graph, 3, 4);

    printGraph(graph);
    return 0;
}