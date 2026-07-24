#include <iostream>
struct anode{
    int many;
};

struct Node{
    int value;
    Node* left;
    Node* right;
};
int find_min(Node* root){
    Node* curr=root;
    while (curr->left!=nullptr){
        curr=curr->left;
    }
    return curr->value;
}
Node* create_node(int value){
    Node* new_node=new Node;
    new_node->value=value;
    new_node->left=nullptr;
    new_node->right=nullptr;
    return new_node;
};

Node* insert(Node* root,int value){
    if (root==nullptr){
        root =create_node(value);
        return root;

    }
    else if (value<=root->value){
        root->left=insert(root->left,value);
        return root;
    }

    else {
        root->right=insert(root->right,value);
        return root;
    };

};

Node* delete_node(Node* root,int value){
    if (root==nullptr){
        return root;
    }
    else if (root->value>value){
        root->left=delete_node(root->left,value);
    }
    else if (root->value<value){
        root->right=delete_node(root->right,value);
    }
    
    else{
        if (root->left==nullptr){
                Node* temp=root->right;
                delete root;
                return temp;
        }
        if (root->right==nullptr){
                Node* temp=root->left;
                delete root;
                return temp;
        }
        root->value=find_min(root->right);
        root->right=delete_node(root->right,root->value);
        
        
    }
    return root;
}

Node* Delete(Node* root,int value){
    if (root==nullptr){
        return root;
    }
    else if(value<root->value){
        root->left=Delete(root->left,value);
    }
    else if(value>root->value){
        root->right=Delete(root->right,value);
    }
    else{
        if (root->left==nullptr&&root->right==nullptr){
            delete root;
            root=nullptr;
            return root;
        }
        else if (root->right==nullptr){
            Node* temp=root;
            temp=temp->left;
            delete root;
            return temp;
        }
        else if (root->left==nullptr){
            Node* temp=root;
            temp=root->right;
            delete root;
            return temp;
        }
        else{
            root->value=find_min(root->right);
            root=Delete(root->right,root->value);
            return root;
        }
    }
    return root;
}

int find_height(Node* root){
    if (root==nullptr){
        return -1;
    }
    int left=find_height(root->left);
    int right=find_height(root->right);
    if (left>right){
        return 1+left;
    }
    else{
        return 1+right;
    }
}

int main(){
    Node* root;
    anode b;
    b.many;
    root=nullptr;
    root = insert(root,5);
    root = insert(root,2);
    root = insert(root,3);
    root = insert(root,4);
    root = insert(root,1);
    root = insert(root,6);
    root = insert(root,7);
    root = insert(root,8);
    root = insert(root,9);
    root = insert(root,10);
    delete_node(root,2);
    Node* temp=root;
    while(temp!=nullptr){
        std::cout<<temp->value<<">";
        temp=temp->left;
        
    }
    int h=find_height(root);
    std::cout<<"\n"<<h;
    return 0;

}

