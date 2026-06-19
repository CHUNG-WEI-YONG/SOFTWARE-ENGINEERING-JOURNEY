const path=require('path');
const grpc=require('@grpc/grpc-js');
const protoloader=require('@grpc/proto-loader');

const proto_path=path.join(__dirname,'message.proto');
const packageDefinition =protoloader.loadSync(proto_path,{keepCase:true, longs:String,
    enums:String , defaults:true, oneofs:true,
})

const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)
const message_proto=protoDescriptor.message;

module.exports=message_proto