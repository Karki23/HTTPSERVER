/*
The basic workflow of a http server is
>> Initially HTTP Client(i.e., web browser) sends a HTTP request to the HTTP Server.
>> Server processes the request received and sends HTTP response to the HTTP client.

Here the browser is the client and the stuff rendered on the browser given as address like
http://www.example.com:80/
is the server's index.html

When a request is given by a browser it has few http methods
GET
The GET method requests a representation of the specified resource. Requests using GET should only retrieve data and should have no other effect.

HEAD
The HEAD method asks for a response identical to that of a GET request, but without the response body.

POST
The POST method requests that the server accept the entity enclosed in the request as a new subordinate of the web resource identified by the URI.
The data POSTed might be, for example, an annotation for existing resources; a message for a bulletin board, newsgroup, mailing list, or comment thread;
a block of data that is the result of submitting a web form to a data-handling process; or an item to add to a database.[23]

PUT
The PUT method requests that the enclosed entity be stored under the supplied URI. If the URI refers to an already existing resource, it is modified;
if the URI does not point to an existing resource, then the server can create the resource with that URI.[24]

DELETE
The DELETE method deletes the specified resource.

A Basic structure of request (http)
GET /hello.html HTTP/1.1
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
Host: www.tutorialspoint.com
Accept-Language: en-us
Accept-Encoding: gzip, deflate
Connection: Keep-Alive

A basic structure of response (http)
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: 88
Content-Type: text/html
Connection: Closed

<html>
<body>
<h1>Hello, World!</h1>
</body>
</html>

For a response to be given and something meaningful to be rendered in the browser 3 headers are cumpolsory

HTTP/1.1 200 OK → This mentions what version of HTTP we are using, Status code and Status message.
Content-Type: text/plain → This says that I’m (server) sending a plain text. There are many Content-Types. For example, for images we use this.
Content-Length: 12 → It mentions how many bytes the server is sending to the client. The web-browser only reads how much we mention here.


THE FOLLOWING CODE IS AN EXTENSION OF THE TCP_SERVER.C
HERE WE LAEERN HOW TO SERVE WEB PAGES AND IMAGES ETC FROM A SERVER.
There are 3 main cases to consider:
The file(web page) is present
The file(web page) is absent
The client doesn’t have permissions to access the file (web page).
The list of status codes is as follows:
1xx informational response
2xx success
3xx Redirection
4xx Client errors
5xx Server errors

The content-types are
The two composite top-level media types are:

Multipart
Message
The five discrete top-level media types are:

Text
Image
Audio
Video
Application
*/

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#define PORT 8080

typedef struct HTTPREQ HTTPREQ;
typedef struct HTTPRES HTTPRES;

struct HTTPRES{
  char status[30];
  char conttype[30];
  char contlength[25];
  char content[30000];
};

struct HTTPREQ
{
  char METHOD[6];
  char file[25];
  char content[30000];;
};

int strend(const char *s, const char *t);
int search(char* pat, char* txt);
char* getResponseBody(char* filename);
int isFilePresent(char *fn);
HTTPRES* formresponse(HTTPREQ* parsed);
HTTPREQ* parserequest(const char* raw_req);
char* getcontent(HTTPRES* response);
char* handlehttpresp(const char* request);

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char* buffer = (char *)malloc(sizeof(char)*30000);
    char* final_response = (char *)malloc(sizeof(char)*30000);
    HTTPREQ* request = (HTTPREQ *)malloc(sizeof(HTTPREQ));
    HTTPRES* response = (HTTPRES *)malloc(sizeof(HTTPRES));
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket could not be created. Sorry"); //prints the descreptive error message to stderr, while your normal printf() prints it to stdout
        exit(EXIT_FAILURE); //Instead of exit(1) viz normally used to show unsuccessful termination, EXIT_FAILURE is used
        //as on different platform exit(1) might also mean success, it uses the standard EXIT_FAILURE
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //server doesn't need to have dest address just like in python we could keep it as ''
    address.sin_port = htons(PORT);
    //htons is a function called host-to-netwrk short
    //It works on 16 bit short integers or 2 bytes
    //What it does is it changes the endianess of the port number : viz from little to big endian
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    //used to set a block of memory in C, as sin_zero is an array it fills the whole array with \0

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("Socket could not be binded");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    //part wehre it checks for connection continously and sends and recieves a message
    while(1)
    {
        printf("\nChecking if any connection exists.......\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("Could not recieve client message");
            exit(EXIT_FAILURE);
        }
        valread = read( new_socket ,buffer,30000);
        printf("HTTP request recieved from client\n");
        request = parserequest(buffer);
        response = formresponse(request);
        strcpy(final_response,getcontent(response));
        write(new_socket , final_response , strlen(final_response));
        printf("HTTP response sent to client\n");
        close(new_socket);
    }
    return 0;
}

////*************************START OF HELPER FUNCTIONS***************************************///

HTTPREQ* parserequest(const char* raw_req){
  HTTPREQ* parsed = (HTTPREQ *)malloc(sizeof(HTTPREQ));
  int i = 0;
  int j = 0;
  char* temp = (char*)malloc(sizeof(char)*30000);
  while(raw_req[i]!=' '){
    temp[i] = raw_req[i];
    i+=1;
  }
  temp[i] = '\0';
  strcpy(parsed->METHOD,temp);
  strcpy(temp,"");
  i+=2;
  while(raw_req[i]!=' '){
    temp[j] = raw_req[i];
    i+=1;
    j+=1;
  }
  if(j<2){ //if it is directed at the root URL ie localhost:8080/ it redirects to index.html by default, basically checks if the file component is "" or not
    strcpy(parsed->file,"index.html");
  }
  else{ //otherwise redirect to the given path
    strcpy(parsed->file,temp);
  }
  strcpy(temp,"");
  j = 0;
  if(!(strcmp(parsed->METHOD,"POST"))){ //
    while(raw_req[i]!='\n' || raw_req[i+1]!='\n'){  //get to the content body
      i +=1;
      if(raw_req[i]=='\0'){ //if end is reached before that then return parsed
        strcpy(parsed->content,"");
        return parsed;
      }
    }
    i+=2;
    while(raw_req[i]!='\0'){ //extract content body
      temp[j] = raw_req[i];
      j+=1;
      i+=1;
    }
    temp[j] = '\0';
    strcpy(parsed->content,temp);
  }
  else{
    strcpy(parsed->content,"");
  }
  return parsed;
}

HTTPRES* formresponse(HTTPREQ* parsed){
  HTTPRES* response = (HTTPRES*)malloc(sizeof(HTTPRES));
  if(!(strcmp(parsed->METHOD,"GET"))){
      if(isFilePresent(parsed->file)){
        strcpy(response->status,"HTTP 1.1/ 200 OK");
        if(strend(parsed->file,".html")){
          strcpy(response->conttype, "Content-Type: text/html");
        }
        else if(strend(parsed->file,".txt")){
          strcpy(response->conttype,"Content-Type: text/plain");
        }
        strcpy(response->content,getResponseBody(parsed->file));
        int length = strlen(response->content);
        char* lengthsize = (char*)malloc(15*sizeof(char));
        snprintf(lengthsize, sizeof(lengthsize), "%d",length);
        strcpy(response->contlength, "Content-Length: ");
        strcat(response->contlength, lengthsize);
      }
      else{
        strcpy(response->status,"HTTP 1.1/ 404 NOT FOUND");
        strcpy(response->conttype,"text/plain");
        strcpy(response->contlength,"39");
        strcpy(response->content,"Sorry. The requested page is not found!");
      }
    }
  else if(!(strcmp(parsed->METHOD,"POST"))){
      time_t now;
      FILE *fp = NULL;
      if(!(fp = fopen(parsed->file,"a"))){
        fp = fopen(parsed->file,"w");
      }
      fprintf(fp,"%s\n", ctime(&now));
      fprintf(fp,"%s\n\n",parsed->content);
      fclose(fp);
      strcpy(response->status,"HTTP 1.1/ 200 OK");
      strcpy(response->conttype,"Content-Type: text/plain");
      strcpy(response->contlength,"Content-Length: 51");
      strcpy(response->content,"Information successfully posted and saved to server");
  }
  return response;
}

int isFilePresent(char *fn){
  FILE *fp;
  fp = popen("dir", "r");
  char c;
  int i=0;
  int d=0;
  char filename[1000];
  while((c = fgetc(fp)) != EOF)
  {
      filename[i]=c;
      i+=1;
  }
  pclose(fp);
  char cleanerfilename[1000]; //list of all files and directories after strpping out the spaces
  for(i =0,d= 0;i<strlen(filename);i++){
      if (!((filename[i] == ' ' && filename[i+1] == ' ') || filename[i]=='\t')) {
        cleanerfilename[d] = filename[i];
        d+=1;
      }
    }
  return(search(fn,cleanerfilename));
}

char* getcontent(HTTPRES* response){
  char* temp = (char*)malloc(sizeof(char)*30000);
  strcpy(temp,response->status);
  strcat(temp,"\n");
  strcat(temp,response->contlength);
  strcat(temp,"\n");
  strcat(temp,response->conttype);
  strcat(temp,"\n\n");
  strcat(temp,response->content);
  return temp;
}

char* getResponseBody(char* filename){
  FILE* filePointer = fopen(filename,"r");
  if(!filePointer)
    return NULL;
    char* buffer = 0;
    fseek(filePointer,0,SEEK_END);
    long int length = ftell(filePointer);
    fseek(filePointer,0,SEEK_SET);
    buffer = (char *)malloc(length);
    if (buffer)
    {
      fread (buffer, 1, length, filePointer);
    }
    buffer[length] = '\0';
    fclose(filePointer);
    return buffer;
}

int search(char* pat, char* txt){
  int M = strlen(pat);
  int N = strlen(txt);
  for (int i = 0; i <= N - M; i++){
      int j;
      for (j = 0; j < M; j++)
          if (txt[i + j] != pat[j])
              break;
      if (j == M)
          return 1;
  }
  return 0;
}

int strend(const char *s, const char *t) //function to check if a string ends with a something, used to check file extensions
{
  size_t ls = strlen(s);
  size_t lt = strlen(t);
  if (ls >= lt){
      return (0 == memcmp(t, s + (ls - lt), lt));
  }
  return 0;
}
