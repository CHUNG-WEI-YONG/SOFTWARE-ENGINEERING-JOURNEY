#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include<queue>
#include <limits>
#include <string>
#include <algorithm>
using namespace std;

struct AdjNode{
    int vertex;
    int weight;
    AdjNode* next;
};

struct edge{
    int u;
    int v;
    int weight;
};

struct Graph{
    int vertex;
    vector<AdjNode*> adj;
    unordered_map<string,int> id;
    vector<string> names;
    vector<edge> edges;
    vector<int> parent;
};

int getID(Graph* graph,string name){
    if (graph->id.count(name)){
        return graph->id[name];
    }
    int newId=graph->names.size();
    graph->id[name]=newId;
    graph->names.push_back(name);
    return newId;

}

Graph* create_graph(int vertex){
    Graph* graph=new Graph;
    graph->vertex=vertex;
    graph->adj.resize(vertex,nullptr);
    return graph;
}

AdjNode* create_node(int value,int weight){
    AdjNode* new_node=new AdjNode;
    new_node->vertex=value;
    new_node->weight=weight;
    new_node->next=nullptr;
    return new_node;
}

int find(Graph* graph , int vertex){
    if(graph->parent[vertex]!=vertex){
        graph->parent[vertex]=find(graph,graph->parent[vertex]);
        
    }
    return graph->parent[vertex];
}

void Union(Graph* graph,int v,int u){
    int rootA=find(graph,v);
    int rootB=find(graph,u);
    if(rootA!=rootB){
        graph->parent[rootB]=rootA;
    }
}

Graph* add_edge(Graph* graph,string origin,string destination,int weight){
    int ori=getID(graph,origin);
    int final=getID(graph,destination);
    AdjNode* first=create_node(final,weight);
    first->next=graph->adj[ori];
    graph->adj[ori]=first;

    AdjNode* second=create_node(ori,weight);
    second->next=graph->adj[final];
    graph->adj[final]=second;

    graph->edges.push_back({ori,final,weight});
    return graph;
}

void BFS(Graph* graph,int start){
    queue<int> q;
    bool* visited=new bool[graph->vertex]();
//写出bool是否已经visited
    visited[start]=true;
    q.push(start);
    //把第一个放进去
    cout<<"BFS:"<<endl;
    while(!q.empty()){
        int v=q.front();
        q.pop();
        cout<<graph->names[v]<<endl;
        //放出第一个
        AdjNode* curr=graph->adj[v];
        
        while (curr!=nullptr){
            int neighbour=curr->vertex;
            if (!visited[neighbour]){
                visited[neighbour]=true;
                q.push(neighbour);
            }
            curr=curr->next;
        }//把第一个的所有邻居都弄出来；
        cout<<endl;}
        delete[] visited;
}

bool DCYCLE(int v,bool visited[],int parent,Graph* graph){
    visited[v]=true;
    AdjNode* curr=graph->adj[v];

    while(curr!=nullptr){
        int neighbour=curr->vertex;

        if (!visited[neighbour]){
            if(DCYCLE(neighbour,visited,v,graph)){
                return true;
            }

        }
        else if(neighbour!=parent){
            return true;
        }

        curr=curr->next;
    }
    return false;
}

bool hasCycle(Graph* graph,int v){
    bool* visited=new bool[graph->vertex]();
    for (int i=0;i<graph->vertex;i++){
        if (!visited[i]){
            if (DCYCLE(i,visited,-1,graph)){
                return true;
            }
        }

    }
    return false;
}

void printDjkstra(int target , vector<int>& parent,Graph* graph){
    if (target==-1){
        cout<<endl;
        return;
    }
    printDjkstra(parent[target],parent,graph);
    cout<<graph->names[target]<<"->";

}

void djkstra(int start , Graph* graph ){
    vector <int> distance(graph->vertex,INT_MAX);
    distance[start]=0;
    vector <int> parent(graph->vertex,-1);


    vector <bool> visited(graph->vertex,false);

    priority_queue<
        pair<int , int>,
        vector<pair<int,int>>,
        greater<pair<int,int>> 
        >pq;

    pq.push({0,start});

    while(!pq.empty()){
        int u=pq.top().second;
        int d=pq.top().first;
        pq.pop();
        if(visited[u]){continue;}
        visited[u]=true;

        AdjNode* curr=graph->adj[u];
        while (curr!=nullptr){
            int v=curr->vertex;
            int w=curr->weight;
            if (distance[u]+w<distance[v]){
                distance[v]=distance[u]+w;
                parent[v]=u;
                pq.push({distance[v],v});
            }
            
            curr=curr->next;
        }

    }
    for (int i=0;i<graph->vertex;i++){
        if (distance[i]==INT_MAX){
            continue;
        }
        cout<<graph->names[i]<<" : "<<distance[i]<<endl;
        cout<<"To: "<<graph->names[i]<<endl;
        cout<<"distance "<<distance[i]<<endl;
        cout<<"Path: ";
        printDjkstra(i,parent,graph);
    }
}

void Bellman_Ford(int start,Graph* graph){
    int v=graph->vertex;
    vector<int> dist(v,INT_MAX);
    dist[start]=0;
    for (int k=0;k<v-1;k++){
        for (int i=0;i<v;i++){/*this allow the v-1 time iteration so that all point meet , 
            and i every iteration the point is visited , if the point is not visited in the first time , 
            then it will not chang ein first iteration*/
        AdjNode* curr=graph->adj[i];
        while(curr!=nullptr){
            int vertex=curr->vertex;
            int w=curr->weight;
            if(dist[i]!=INT_MAX&&dist[i]+w<dist[vertex]){
                dist[vertex]=dist[i]+w;
            }
            curr=curr->next;

        }
        }
}
        for (int i=0;i<v;i++){
        AdjNode* curr=graph->adj[i];
        while(curr!=nullptr){
            int vertex=curr->vertex;
            int w=curr->weight;
            if(dist[i]!=INT_MAX&&dist[i]+w<dist[vertex]){
                cout<<"Negative cycle detected"<<endl;
                return;
            }
            curr=curr->next;

        }
        
}
}

void prim(Graph* graph ,int start){
    vector<int> dist(graph->vertex,INT_MAX);
    vector<bool> inMST(graph->vertex,false);
    vector<int> parent(graph->vertex,-1);

    priority_queue<
        pair<int,int>,
        vector<pair<int,int>>,
        greater<pair<int,int>>
    >pq;

    dist[start]=0;
    pq.push({dist[start],start});

    while(!pq.empty()){
        int v=pq.top().second;
        int w=pq.top().first;

        pq.pop();
        if(inMST[v]){continue;}
        inMST[v]=true;
        AdjNode* curr=graph->adj[v];
        while(curr!=nullptr){
            int l=curr->weight;
            int vertex=curr->vertex;
            if(!inMST[vertex]&& l<dist[vertex]){
                dist[vertex]=l;
                parent[vertex]=v;
                pq.push({dist[vertex],vertex});
            }


            curr=curr->next;
        }
    }

    cout<<"----Prim Algorithms----"<<endl;
    for(int i=0;i<graph->vertex;i++){
        if (parent[i]!=-1){
            cout << graph->names[parent[i]]
                 << " - "
                 << graph->names[i]
                 << " : "
                 << dist[i] << endl;
        }
    }
}

void Kruskal(Graph* graph){
    graph->parent.resize(graph->vertex);
    for (int i=0;i<graph->vertex;i++){
        graph->parent[i]=i;
    }
    std::sort(graph->edges.begin(),graph->edges.end(),[](edge a, edge b){
        return a.weight<b.weight;
    });
    for (edge e:graph->edges){
        int rootA=find(graph,e.u);
        int rootB=find(graph,e.v);

        if (rootA!=rootB){
            cout << graph->names[e.u] << " - "
             << graph->names[e.v] << " : "
             << e.weight << endl;
             Union(graph,rootA,rootB);
        
        }
    }
}
    




/*int main(){
    Graph* graph=create_graph(5);
    int edge
    for (int i=0;i<5;i++){
        string place1;
        cout<<"Place1: ";
        cin>>place1;
        string place2;
        cout<<"Place2: ";
        cin>>place2;
        int dist;
        cout<<"Distance: ";
        cin>>dist;

        graph=add_edge(graph,place1,place2,dist);
    }
    string origin;
    cout<<"sTART FROM: "
    cin>>origin;
    int num=getID(graph,origin);
    djkstra(num,graph);

    
return 0;}*/

   int main(){

    Graph* graph = create_graph(6);

    add_edge(graph, "A", "B", 4);
    add_edge(graph, "A", "C", 2);

    add_edge(graph, "B", "C", 1);
    add_edge(graph, "B", "D", 5);

    add_edge(graph, "C", "D", 8);
    add_edge(graph, "C", "E", 10);

    add_edge(graph, "D", "E", 2);
    add_edge(graph, "D", "F", -6);

    add_edge(graph, "E", "F", 3);

    // 起点 A
    int start = getID(graph, "A");

    cout << "===== DIJKSTRA TEST =====\n";

    Bellman_Ford(start, graph);
    prim(graph,3);

    return 0;
}

    


    






