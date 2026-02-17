'use strict';
// Set up express server.
const httpSocketAddr = process.env.DQM_HOST_ADDRESS; // get the DQM host address.
const express = require('express'); // import package for hosting local webserver.
const app = express(); // instantiate host app.
const http = require('http').Server(app); // instantiate host app using http protocol.

// Setup data ingest socket.
const zmq = require("zeromq"); // import package for hosting 
const zmqSocketAddr = "tcp://localhost:"+process.env.DQM_ZMQ_ADDRESS; // define the zmq ingest address.
const subSock = new zmq.socket('sub'); // setup socket for ingesting data from server.
subSock.connect(zmqSocketAddr); // connect to socket.
subSock.subscribe(""); // Selects a topic for subscription. Will only receive from publishers with that 

// Setup other package dependencies.
const path = require('path'); 

// Setup data constants.
const displayTime = 10; //seconds
const DQMSamplingRate = 100; //Hz. This is the display rate of data.
const displayingSamples = displayTime * DQMSamplingRate; //Amount of data to display
var data = []

const spillSampleRate = 50; // Number of data elements to take per spill.
var spillSampleCounter = 0;
var averageMessageCount = 0;

subSock.on('message', (message) => {
    spillSampleCounter = spillSampleCounter + 1;
    //if (!(spillSampleCounter%spillSampleRate)){
    //    data.push(parseInt(message.toString()));
    //};
    data.push(parseInt(message));
    averageMessageCount = averageMessageCount + 1;
});


app.get('/', (req, res) =>{
    res.sendFile(path.join(__dirname,"/ELBE_DQM.html"));
});

app.get('/call', (req, res)=>{
    res.send(data);
    data = [];
});

app.get('/average', (req, res)=>{
    res.send([averageMessageCount]);
    averageMessageCount = 0;
});

console.log();

//Start your server on a specified port
app.listen(httpSocketAddr, ()=>{
    console.log(`Server is runing on port ${httpSocketAddr}`);
});