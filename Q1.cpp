#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <string.h>
#include <fcntl.h>
#include <cctype>
#include <sys/wait.h>
#include <sstream>
using namespace std;

string cptrtos(char* toConvert)         //Converts char array to string
{
	string str;
	for (int i = 0; toConvert[i]; i++)
		str += toConvert[i];

	return str;
}

string readFile(string filename)        //Will read the file and store it into string
{
    string read;
    char c;

    fstream obj (filename + ".txt", ios::in);       //Opening file

    if (obj)
    {
        c = obj.get();
        while (!obj.eof())
        {
           if (isupper(c))
            c = tolower(c);

           read += c; 
           c = obj.get();
        }
        obj.close();
    }
    else
    {
        cout<<"File not opened!"<<endl;
    }

    return read;

}

int countChar(string s, char a)
{
    int count=0;
    for (int i=0; i< s.length(); i++)
    {
        if (s[i] == a)
        {
            count++;
        }
    }
    return count;
}

int main()
{
    char buff[100];
    string filename;
    int size,size1;
    string F1,F2,F3;
    char letter = 'a';
    cout<<"Enter the name of the file = ";
    cin>>filename;

    string fileText = readFile(filename);       //Stores the contents of file in "fileText"

//----------------------------------------------------------------------------------------------------
    if (fileText.length()%3 == 0)
    {
        size = fileText.length()/3;             //Size of each chunk
    
        F1 = fileText.substr(0, size);
        F2 = fileText.substr(size, size);
        F3 = fileText.substr(size * 2, size);

        cout << "F1 Chunk = "<<F1 <<endl<<"F2 Chunk = "<< F2 <<endl<<"F3 Chunk = "<< F3 << endl;
    }
    else
    {
        size = fileText.length()/3;         //Size of first two chunks
        int temp = fileText.length()%3;     //Remainder
        size1 = size+temp;                  //Size of last chunk + remaining characters

        F1 = fileText.substr(0, size);
        F2 = fileText.substr(size, size);
        F3 = fileText.substr(size * 2, size1);
        cout << "F1 Chunk = "<<F1 <<endl<<"F2 Chunk = "<< F2 <<endl<<"F3 Chunk = "<< F3 << endl;
    }
//------------------------------------------------------------------------------------------------------

    string p1_str,p2_str, p3_str;

    int P1_to_parent_pipe[2];           //Pipe between P1 and parent process
    pipe(P1_to_parent_pipe);

    pid_t P1 = fork();

    if (P1 > 0)     //Parent
    {
        wait(NULL);
        close(P1_to_parent_pipe[1]);
        read(P1_to_parent_pipe[0],buff,sizeof(buff));

        p1_str = cptrtos(buff);
        ////////////////////// P2

        int P2_to_parent_pipe[2];           //Pipe between P2 and parent process
        pipe(P2_to_parent_pipe);

        pid_t P2 = fork();

        if (P2 > 0)     //Parent
        {
            wait(NULL);
            close(P2_to_parent_pipe[1]);
            read(P2_to_parent_pipe[0],buff,sizeof(buff));
            p2_str = cptrtos(buff);
            ///////////////////////// P3

            int P3_to_parent_pipe[2];           //Pipe between P3 and parent process
            pipe(P3_to_parent_pipe);

            pid_t P3 = fork();

            if (P3 > 0)         //Parent
            {
                wait(NULL);
                close(P3_to_parent_pipe[1]);
                read(P3_to_parent_pipe[0],buff,sizeof(buff));
                p3_str = cptrtos(buff);
                /////////////////////

                int characterCounts[26] = {0};

                int index = 0;
                string number;

                for (int i = 0; i < p1_str.length(); i++) {
                    if (p1_str[i] == '.') 
                    {
                        stringstream obj(number);
                        int x = 0; 
                        obj >> x; 

                        characterCounts[index] += x;

                        index++;
                        number.clear();
                    }

                    else {
                        number += p1_str[i];
                    }
                }

                index = 0;

                for (int i = 0; i < p2_str.length(); i++) {
                    if (p2_str[i] == '.') 
                    {
                        stringstream obj(number);
                        int x = 0; 
                        obj >> x; 

                        characterCounts[index] += x;

                        index++;
                        number.clear();
                    }

                    else {
                        number += p2_str[i];
                    }
                }

                index = 0;

                for (int i = 0; i < p3_str.length(); i++) {
                    if (p3_str[i] == '.') 
                    {
                        stringstream obj(number);
                        int x = 0; 
                        obj >> x; 

                        characterCounts[index] += x;

                        index++;
                        number.clear();
                    }

                    else {
                        number += p3_str[i];
                    }
                }

                cout << "Total number of each character in file: " << endl;

                for (char letter = 'a',index = 0; letter <= 'z'; letter++, index++)
                    cout << letter << " = "<<characterCounts[index] <<endl;

                cout << endl;
            }

            else if (P3 == 0)
            {
                letter = 'a';
                close(P3_to_parent_pipe[0]);
                for (int i = 0; i < 26; i++,letter++)        //loop to create 26 processes
                {
                    int P3_child_pipe[2];               //Pipe between P3 Child and P3
                    pipe(P3_child_pipe);

                    pid_t pid = fork();

                    if (pid > 0)        // P3
                    {
                        wait(NULL);

                        close(P3_child_pipe[1]);        //Writing end of pipe 
                        read(P3_child_pipe[0],buff,sizeof(buff));
                        //cout<<"Parent "<<letter<<" = "<<buff<<endl;
            
                        p3_str += cptrtos(buff);
                        p3_str += ".";

                        if (letter == 'z') 
                        {
                            strcpy(buff, p3_str.c_str());
                            write(P3_to_parent_pipe[1],buff,strlen(buff)+1);
                        }
                    }

                    else if (pid == 0)      //Sub-Children
                    {
                        close(P3_child_pipe[0]);        //Reading end of pipe

                        int count = countChar(F3, letter);
                        strcpy(buff, to_string(count).c_str());

                        write(P3_child_pipe[1],buff,strlen(buff)+1);
                        kill(getpid(), SIGTERM);        //child kills itself (so it can't fork itself)
                    }

                    else 
                    {
                        cout << "Alphabet child fork failed." << endl;
                    }
                }
            }

            else
            {
                cout<<"P3 Failed"<<endl;
            }
            
        }

        else if (P2 == 0)
        {
            letter = 'a';
            close(P2_to_parent_pipe[0]);
            for (int i = 0; i < 26; i++,letter++)        //loop to create 26 processes
            {
                int P2_child_pipe[2];               //Pipe between P2 Child and P2
                pipe (P2_child_pipe);

                pid_t pid = fork();

                if (pid > 0)        // P2
                {
                    wait(NULL);

                    close(P2_child_pipe[1]);        //Writing end of pipe 
                    read(P2_child_pipe[0],buff,sizeof(buff));
                    //cout<<"Parent "<<letter<<" = "<<buff<<endl;
        
                    p2_str += cptrtos(buff);
                    p2_str += ".";

                    if (letter == 'z') {
                        strcpy(buff, p2_str.c_str());
                        write(P2_to_parent_pipe[1],buff,strlen(buff)+1);
                    }
                }

                else if (pid == 0)      //Sub-Children
                {
                    close(P2_child_pipe[0]);        //Reading end of pipe

                    int count = countChar(F2, letter);
                    strcpy(buff, to_string(count).c_str());

                    write(P2_child_pipe[1],buff,strlen(buff)+1);
                    kill(getpid(), SIGTERM);        //child kills itself (so it can't fork itself)
                }

                else 
                {
                    cout << "Alphabet child fork failed." << endl;
                }
            }
        }

        else
        {
            cout<<"P2 failed"<<endl;
        }
        
        
    }

    else if (P1 == 0)       //P1 (first child)
    {
        close(P1_to_parent_pipe[0]);
        for (int i=0; i<26; i++,letter++)        //loop to create 26 processes
        {
            int P1_child_pipe[2];               //Pipe between P1 Child and P1
            pipe (P1_child_pipe);

            pid_t pid = fork();

            if (pid > 0)        // P1
            {
                wait(NULL);

                close(P1_child_pipe[1]);        //Writing end of pipe 
                read(P1_child_pipe[0],buff,sizeof(buff));
                //cout<<"Parent "<<letter<<" = "<<buff<<endl;
    
                p1_str += cptrtos(buff);
                p1_str += ".";

                if (letter == 'z') {
                    strcpy(buff, p1_str.c_str());
                    write(P1_to_parent_pipe[1],buff,strlen(buff)+1);
                }
            }

            else if (pid == 0)      //Sub-Children
            {
                close(P1_child_pipe[0]);        //Reading end of pipe

                int count = countChar(F1, letter);
                strcpy(buff, to_string(count).c_str());

                write(P1_child_pipe[1],buff,strlen(buff)+1);
                kill(getpid(), SIGTERM);        //child kills itself (so it can't fork itself)
            }

            else 
            {
                cout << "Alphabet child fork failed." << endl;
            }
        }
    }

    else
    {
        cout<<"P1 process failed"<<endl;
    }
    

}
