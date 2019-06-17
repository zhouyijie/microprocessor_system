"use strict";

console.log('Loading function');
const aws = require('aws-sdk');
const s3 = new aws.S3({ apiVersion: '2006-03-01' });

exports.handler = (event, context, callback) => {
    //console.log('Received event:', JSON.stringify(event, null, 2));
    var kernel = [  0.0934157147604907, 
                    0.22184014839055305,
                    0.28764651885410586,
                    0.22184014839055305,
                    0.0934157147604907];
    var reqString = event.reqInfo;
    
    //retreive incomging data in numeric form
    reqString = reqString.replace(/(\r\n|\n|\r)/gm,",");
    var nums = reqString.split(',').map(function(item) {
    return parseFloat(item, 10);
    });
    
    //split numbers into dimensional data
    var data_x=[];
    var data_y=[];
    var data_z=[];
    for(let i=0;i<nums.length/3;i++){
        data_x[i]=nums[3*i];
        data_y[i]=nums[3*i+1];
        data_z[i]=nums[3*i+2];
    }
    
    //filter data
    var f_x=[];
    var f_y=[];
    var f_z=[];
    for(let i=0;i<data_x.length-kernel.length;i++){
        f_x[i] =    data_x[i]*kernel[0]+
                    data_x[i+1]*kernel[1]+
                    data_x[i+2]*kernel[2]+
                    data_x[i+3]*kernel[3]+
                    data_x[i+4]*kernel[4];
        f_y[i] =    data_y[i]*kernel[0]+
                    data_y[i+1]*kernel[1]+
                    data_y[i+2]*kernel[2]+
                    data_y[i+3]*kernel[3]+
                    data_y[i+4]*kernel[4];
        f_z[i] =    data_z[i]*kernel[0]+
                    data_z[i+1]*kernel[1]+
                    data_z[i+2]*kernel[2]+
                    data_z[i+3]*kernel[3]+
                    data_z[i+4]*kernel[4];
    }
    
    //stringify results back into csv form
    var outString= '';
    for(let i=0; i<f_x.length;i++){
        outString = outString+f_x[i].toString()+','+f_y[i].toString()+','+f_z[i].toString()+' \n';
    }
    var info = {resInfo:outString};
    
    //store in S3
    var bodybuffer= new Buffer(outString);
    var params ={Bucket: 'uploadsmicrop', Key: 'testdownload/android.txt', Body: bodybuffer};
    //params.Body=outString;
     s3.upload(params, function(err, data) {
         if (err) console.log(err, err.stack); // an error occurred
         else console.log('success');           // successful response
         //var info = {"data": 'some test string'};
         context.done();
     });
     
    console.log(info);
    console.log('done?');
    callback(null, info);
};
