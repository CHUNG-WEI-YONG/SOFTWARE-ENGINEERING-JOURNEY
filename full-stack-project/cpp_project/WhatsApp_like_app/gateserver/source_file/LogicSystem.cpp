#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"

void LogicSystem::RegGet(std::string s, HttpHandler handler){
    _get_handles.insert(make_pair(s,handler));
}

void LogicSystem::RegPost(std::string s, HttpHandler handler){
    _post_handles.insert(make_pair(s,handler));
}

LogicSystem::LogicSystem(){
    RegGet("/get_test",[](std::shared_ptr<HttpConnection> connection){
        beast::ostream(connection->_response.body())<<"Receive get_request response";
        int i=0;
        for(auto& elem:connection->_get_params){
            beast::ostream(connection->_response.body())<<"param "<<i<<" key is "<<elem.first;
            beast::ostream(connection->_response.body())<<"param "<<i<<" value is "<<elem.second<<std::endl;
        }
    });

    RegPost("/get_verifycode",[](std::shared_ptr<HttpConnection> connection){
        auto body_str=boost::beast::buffers_to_string(connection->_request.body().data());
        std::cout<<"receive body is "<<body_str<<std::endl;
        connection->_response.set(http::field::content_type,"text/json");
        // Json::Value root;
        // Json::Reader reader;
        // Json::Value source_root;
        json root;
        json src_root;
        try{
            src_root=json::parse(body_str);
            if (!src_root.contains("email")) {
                throw json::out_of_range::create(403, "Missing required parameter: key", &src_root);
            }

            std::string email=src_root["email"];
			GetVarifyRsp rsp= VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
            root["error"]=rsp.error();
            root["email"]=src_root["email"];


        }catch(const std::exception &e){
            root["error"]=ErrorCodes::Error_Json;
            root["msg"]="Failed to parse JSON data or missing core keys";
        }

        std::string jsonstr=root.dump();
        boost::beast::ostream(connection->_request.body())<<jsonstr;
        connection->WriteResponse();
        return true ;
    
    });
};


bool LogicSystem::HandleGet(std::string path,std::shared_ptr<HttpConnection> conn){
    if(_get_handles.find(path)==_get_handles.end()){
        return false;
    }
    _get_handles[path](conn);
    return true;

}

bool LogicSystem::HandlePost(std::string path,std::shared_ptr<HttpConnection> conn){
    if(_post_handles.find(path)==_post_handles.end()){
        return false;
    }
    _post_handles[path](conn);
    return true;
}
