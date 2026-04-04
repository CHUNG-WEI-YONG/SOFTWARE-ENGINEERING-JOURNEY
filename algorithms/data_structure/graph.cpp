#include <iostream>
#include <vector>
#include<stack>
#include<queue>
#include <climits>
using namespace std;

struct AdjNode{
    int vertex;
    int weight;
    AdjNode* next;
};

AdjNode* create_node(int v,int w){
    AdjNode* a=new AdjNode;
    a->vertex=v;
    a->weight=w;
    a->next=nullptr;
    return a;
}

class graph{
    private: 
    int vertex;
    vector<AdjNode*> adj;

    void to_bfs(int i,vector<bool> &v,queue<int> &s){
        AdjNode* curr=adj[i];
        while(curr){
            if(!v[curr->vertex]){
                s.push(curr->vertex);
                v[curr->vertex]=true;
            }
            curr=curr->next;
        }
        
    }

    void util_DFS(vector<bool> &visited,int point){
        if(visited[point])return;
        visited[point]=true;
        cout<<point<<"->";
        AdjNode* curr=adj[point];
        while(curr!=nullptr){

            util_DFS(visited,curr->vertex);
            curr=curr->next;
        }

    }

    public:
        graph(){
            
        };
        graph(int v):vertex(v), adj(v,nullptr){
            
        };

        void add_edge(int start,int end,int w=0){
            AdjNode* a= create_node(end,w);
            if(adj[start]==nullptr){
                adj[start]=a;
            }
            else{
                a->next=adj[start];
                adj[start]=a;
            }
        }

        void print_graph(){
            for(int i=0;i<vertex;i++){
                AdjNode* curr=adj[i];
                while(curr!=nullptr){
                    cout<<i<<" -> "<<curr->vertex<<endl;
                    curr=curr->next;
                }

            }
        }

        ~graph(){
            for(int i=0;i<vertex;i++){
                AdjNode* curr=adj[i];
                while(curr!=nullptr){
                   AdjNode* temp=curr; 
                   curr=curr->next;
                   delete temp;
                }
        }
    }

       

    void breath_first_search(int point){
            queue<int> s;
            vector<bool> visited(vertex,false);
            s.push(point);
            visited[point]=true;
        
        while(!s.empty()){
            cout<<s.front()<<",";
           to_bfs(s.front(),visited,s);
           s.pop();
        }
}

    void DFS(int point){
        vector<bool> v(vertex,false);
        util_DFS(v,point);
    }

    void bellmanFord(int point){
        vector <int> distance(vertex,INT_MAX);
        distance[point]=0;
        vector<bool> in_negative(vertex,false);
        for(int i=0;i<vertex-1;i++){
            for(int u=0;u<vertex;u++){
                AdjNode* curr=adj[u];
                while(curr!=nullptr){
                    int v=curr->vertex;
                    int w=curr->weight;
                    if(distance[u]!=INT_MAX&&distance[u]+w<distance[v]){
                        distance[v]=distance[u]+w;

                    }
                    curr=curr->next;
                }
            }
    }
    queue<int> q;
    for(int u=0;u<vertex;u++){
                AdjNode* curr=adj[u];
                while(curr!=nullptr){
                    int v=curr->vertex;
                    int w=curr->weight;
                    if(distance[u]!=INT_MAX&&distance[u]+w<distance[v]){
                        cout<<"There is negative cycle!";
                        if(!in_negative[v]){
                        q.push(v);
                        in_negative[v]=true;}
                        
                    }
                    curr=curr->next;
                }
            }

    while(!q.empty()){
        int a=q.front();
        q.pop();
        distance[a]=INT_MIN;
        AdjNode* curr=adj[a];
        while(curr!=nullptr){
            if (!in_negative[curr->vertex]){
                distance[curr->vertex]=INT_MIN;
                in_negative[curr->vertex]=false;
                q.push(curr->vertex);
            }
            curr=curr->next;
        }
    }    
    for (int i=0;i<vertex;i++){
        cout<<"Distance from point "<<i<<endl;
        cout<<i<<" :" <<distance[i]<<endl;

    }
    return ;
        
    }



    void dijkstra(int point){
    vector<int> dist(vertex,INT_MAX);
    dist[point]=0;
    priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>>pq;
    pq.push({0,point});
    while(!pq.empty()){
        int w=pq.top().first;
        int v=pq.top().second;
        pq.pop();
        AdjNode* curr=adj[v];

        while(curr!=nullptr){
            int a=curr->vertex;
            int d=curr->weight;
            if(dist[v]+d<dist[a]){
                dist[a]=dist[v]+d;
                pq.push({dist[a],a});
            }
        
        curr=curr->next;
    }
}
    for (int i=0;i<vertex;i++){
        cout<<"Distance from point "<<i<<endl;
        cout<<i<<" :" <<dist[i]<<endl;
    }
    return;
}
};


int main(){
    graph g=graph(5);
    g.add_edge(0,1,5);
    g.add_edge(0,1,10);
    g.add_edge(1,2,10);
    g.add_edge(0,4,15);
    g.add_edge(3,4,20);
    g.add_edge(3,4,10);
    g.add_edge(4,3,19);
    g.print_graph();
    g.breath_first_search(0);
    cout<<endl;
    g.DFS(0);
    cout<<endl;
    g.bellmanFord(0);
    g.dijkstra(0);



    return 0;
}
