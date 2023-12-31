/* WebFuncs.cpp
    Functions for sending and processing web server data

    Web pages are stored as raw string literals in the web/webpage_name.h files for cleanliness. 
*/
#include <Arduino.h>
#include <WebServer.h>

void wi_sendStatPage(WebServer& server) { 
    // placeholder hello world for now
    // server.send(200, "text/html", R"=====(  
    // <!DOCTYPE html>
    // <html xmlns="http://www.w3.org/1999/xhtml">
    
    // <head>
    //     <meta charset="UTF-8">
    //     <meta name="viewport" content="width=device-width, initial-scale=1.0">
    //     <title>Hello World!</title>
    // </head>
	
    // <body>
    //     <p>This will be the status page when I'm all grow'd up</p>
    // </body>
	
    // </html>
    
    // )=====");
    debugMsg("[EVENT]: Web Server sent status page");
}

void wi_sendSetupPage(WebServer& server) { 
    
    // placeholder hello world for now
    server.send(200, "text/html", R"=====(  
    <!DOCTYPE html>
    <html xmlns="http://www.w3.org/1999/xhtml">
    
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Hello World!</title>
    </head>
	
    <body>
        <p>This will be the setup page when I'm all grow'd up</p>
    </body>
	
    </html>
    
    )=====");
    debugMsg("[EVENT]: Web Server sent setup page");
}

void wi_sendLogsPage(WebServer& server) { 
    
    // placeholder hello world for now
    server.send(200, "text/html", R"=====(  
    <!DOCTYPE html>
    <html xmlns="http://www.w3.org/1999/xhtml">
    
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Hello World!</title>
    </head>
	
    <body>
        <p>This will be the flight logs page when I'm all grow'd up</p>
    </body>
	
    </html>
    
    )=====");
    debugMsg("[EVENT]: Web Server sent logs page");
}

void wi_sendDocsPage(WebServer& server) { 
    
    // placeholder hello world for now
    server.send(200, "text/html", R"=====(  
    <!DOCTYPE html>
    <html xmlns="http://www.w3.org/1999/xhtml">
    
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Hello World!</title>
    </head>
	
    <body>
        <p>This will be the documentation page when I'm all grow'd up</p>
    </body>
	
    </html>
    
    )=====");
    debugMsg("[EVENT]: Web Server sent docs page");
}

void wi_NotFound(WebServer& server) {
    server.send(404, "text/plain", "Page / data not found");
    debugMsg("[EVENT]: Web Server sent 404 page");
}