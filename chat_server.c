
#include <stdio.h>            // For stdout, stderr
#include <string.h>            // For strlen(), strcpy(), bzero()

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

struct Person
{
	char name[20];
	int membersOf;
	int sd;
	bool hasName;
};

struct Channel
{
	char name[19];
	char** people;
	int numberPeople;
};

void private_message_channel(char* chan_name, char* text, struct Person* allUsers, struct Channel* allChannels, int totalClients, int totalChans)
{
    char creator[20];
    //go through all channels and find the one with correct name
    for(int x=0; x<totalChans; x++)
    {
    	if(strcmp(allChannels[x].name, chan_name) == 0)
    	{
    		//all people currently in the channel need a message 
    		char** allpeeps = allChannels[x].people;
    		for(int r =0; r<allChannels[x].numberPeople;r++)
    		{
    		    //next perosn to get message find this persons socket id
    			char* tempname = allpeeps[r];
    			for(int h=0; h<totalClients;h++)
    			{
    				if(strcmp(tempname, allUsers[h].name) == 0)
    				{
                        //send message to socket id
    					send(allUsers[h].sd, text, strlen(text), 0);	
					break;
    				}
    			}
    		}
    	    //dont need to keep looking through channels bc found it
    		break;
    	}
    }
}


void private_message_individual(char* recipient, char* text, struct Person* allUsers, int totalClients, int i)
{
    int sendsd = 0;
    bool foundPers = false;
    //go through all connected clients
    for(int t=0; t< totalClients; t++)
	{
    	if(strcmp(allUsers[t].name, recipient) == 0)
    	{
            //found sd need to send pmessage tp
            sendsd = allUsers[t].sd;
            foundPers = true;
            break;
		}
    }  
    //send message
    if(foundPers)
    {
        send(sendsd, text, strlen(text),0);
    }  
    //person does not exist
    else
    {
        send(i, "Person not valid\n", 18, 0);
    }
}


int main (int argc, const char * argv[])
{
 	struct sockaddr_in addr;
 	fd_set fd_reads;
 	fd_set full_set;
 	int max_sd = 0;
    //create strcutures all to hold server data
 	struct Person allUsers[1024];
 	struct Channel allChannels[1024];
    char** operators = (char**) malloc(sizeof(char*) * 10);
    for(int i=0; i<10;i++)
    {
        operators[i] = (char*) malloc(sizeof(char) * 20);
    }
    int numOPs = 0;
 	int totalChannels = 0;
 	int totalClients = 0;
	//need a random port number
	uint16_t port = 0;
	//create a socket and assign properties
	int list_sd = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(port);
	addr.sin_family      = PF_INET;
	socklen_t sockLen = sizeof(addr);
	if (list_sd < 0)
	{
		perror("socket() failed");
		exit(-1);
	}
	//bind the socket
    if (bind(list_sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      	perror("bind() failed");
      	close(list_sd);
      	exit(-1);
    }
    //listen on the socket for incoming connections
    if (listen(list_sd, 1024) < 0)
 	{
    	perror("listen() failed");
    	close(list_sd);
    	exit(-1);
 	}
   	getsockname(list_sd, (struct sockaddr*) &addr, &sockLen);
   	printf("%d\n", ntohs(addr.sin_port));
    //prepare fd set to hold ready socket descriptors
   	FD_ZERO(&full_set);
    max_sd = list_sd;
    FD_SET(list_sd, &full_set);
    int new_sd;
    while(true)
    {
    	memcpy(&fd_reads, &full_set, sizeof(full_set));
        //wait until actiity on a sd
    	int rc = select(max_sd + 1, &fd_reads, NULL, NULL, NULL);
    	if (rc < 0)
      	{
         	perror("  select() failed");
         	exit(-1);
      	}
        //listening socket set - new connection
      	if (FD_ISSET(list_sd, &fd_reads))
        {
            rc--;
        	new_sd = accept(list_sd, (struct sockaddr *)&addr, (socklen_t*)&sockLen);
            if (new_sd <0)
            {
                perror("accept");
                exit(-1);
            }
            FD_SET(new_sd, &full_set);
            if (new_sd > max_sd)
		  	{
                max_sd = new_sd;
			}
            //create a new Person
			struct Person temp;
			temp.sd = new_sd;
			temp.hasName = false;
			temp.membersOf = 0;
			strcpy(temp.name,"");
			allUsers[totalClients] = temp;
			totalClients++;
    	}

        //for all other sds that are set - a client entered data
    	for(int i=0; i<=max_sd, rc>0; i++)
    	{
    		if(FD_ISSET(i, &fd_reads))
    		{
                rc--;
    			char buffer[512];
                //buffer = text user typed 
    			int bytes_read = read(i, buffer, 512);
    			buffer[bytes_read] = '\0';
                //if the client closed()with control c) - treat this as a quit command
    			if(bytes_read == 0)
    			{
    				strcpy(buffer, "QUIT");
    			}
                //loop to see if the current client already has a username entered
                char client_name[20];
                bool hasEnteredName = false;
                for(int h=0; h<totalClients;  h++)
                {
                    if(allUsers[h].sd == i)
                    {
                        if(allUsers[h].hasName)
                        {
                            hasEnteredName = true;
                        }
                        strcpy(client_name, allUsers[h].name);
                        break;
                    }
                }
                //if they dont have a name and didnt enter one - kill server
                if(!hasEnteredName && (buffer[0] != 'U' || buffer[1] != 'S' || buffer[2] != 'E' || buffer[3] != 'R'))
                {
                    send(i, "Invalid command, please identify yourself with USER\n", 53, 0);
                    strcpy(buffer, "QUIT");
                }
    			//USER COMMAND
    			if(bytes_read>=5 && buffer[0] == 'U' && buffer[1] == 'S' && buffer[2] == 'E' && buffer[3] == 'R')
    			{
                    //invalid command
                    if(buffer[4] != ' ')
                    {
                        send(i, "Invalid Command\n", 17, 0);
                    }
                    //max length = 20
    			    else if(bytes_read>25)
    				{
    					send(i, "Name too long\n", 14,0);
    				}
    				else
    				{
    					for(int j=0; j<totalClients; j++)
    					{
    						if(allUsers[j].sd == i)
    						{
                                //if they already have a name
    							if(allUsers[j].hasName == true)
    							{
    								send(i, "cannot change name\n", 19,0);
    							}
    							else
    							{
    								allUsers[j].hasName = true;
                                    //set the name
    								memcpy(allUsers[j].name, buffer + 5, bytes_read - 4);
                                    //send welcome message
    								char messag[30];
    								memset(messag, 0, 30);
    								strcat(messag, "Welcome, ");
    								strcat(messag, allUsers[j].name);
    								send(i, messag, strlen(messag), 0);
                                    memset(messag, 0, 30);
    								break;
    							}
    						}
    					}

    				}
    			}
                //LIST COMMAND
    			else if(bytes_read>=4 && buffer[0] == 'L' && buffer[1] == 'I' && buffer[2] == 'S' && buffer[3] == 'T' )
    			{
                    //A CHANNEL LIST COMMAND
    				if(buffer[4] == ' ' && buffer[5] == '#')
    				{
    					char channel_name[20];
    					char** current_members;
    					current_members = (char**) malloc(sizeof(char*) * 50);
    					int num_in_channel = 0;
    					for(int j=0; j<50; j++)
    		  		    {
    						current_members[j] = (char*) malloc(sizeof(char) * 20);
    					}
                        //extract the channel name
    					memcpy(channel_name, buffer+6, bytes_read-5);
                        //find the correct channel
                        for(int j=0; j<totalChannels; j++)
                        {
                            if(strcmp(allChannels[j].name, channel_name) == 0)
                            {
                                struct Channel current = allChannels[j];
                                //send the number of people
                                char message[35];
                                sprintf(message, "There are currently %d members.\n", current.numberPeople);
                                send(i, message, strlen(message),0);
                                //send all the peoples names
                                char total[300];
                                channel_name[strlen(channel_name) -1] = '\0';
                                sprintf(total, "#%s members:", channel_name);
                                send(i, total, strlen(total),0);
                                //loops through all people and sends their name without \n character
                                for(int k=0; k<current.numberPeople; k++)
                                {
                                    send(i, current.people[k], strlen(current.people[k])-1, 0);
                                    send(i, " ", 1, 0);
                                }
                                send(i, "\n", 1, 0);
                                break;
                              }
                        }
    				}
                    //LIST ALL CHANNLELS COMMAND
    				else if(bytes_read == 5)
    				{
                        //send total number and al names
                        char mess[40];
                        sprintf(mess, "There are currently %d channels.\n", totalChannels);
                        send(i, mess, strlen(mess),0);
						for(int y=0; y<totalChannels; y++)
                        {
                            send(i, "* ", 2,0);
                            send(i,allChannels[y].name, strlen(allChannels[y].name), 0);
                        }
    				}
                    //INVALID LIST COMMAND
    				else
					{    					
                    	send(i, "Invalid command\n", 16,0);
    				}
    			}
                //JOIN COMMAND
    			else if(bytes_read>=5 && buffer[0] == 'J' && buffer[1] == 'O' && buffer[2] == 'I' && buffer[3] == 'N' && buffer[4] == ' ')
    			{
    				if(buffer[5] != '#')
    				{
    					send(i, "Invalid command\n", 16,0);
    				}
    				else
    				{
    					//channel name and person who typed join
    					char channel_name[20];
    					char personadded[20];
    					memcpy(channel_name, buffer +6, bytes_read-5);
    					bool newChan = true;
    					//go through all channels and find the one with correct name
    					for(int x=0; x<totalChannels; x++)
    					{
    						if(strcmp(allChannels[x].name, channel_name) == 0)
    						{
    							newChan = false;
    							//find the the person that wants to join the channel
    							for(int t=0; t< totalClients; t++)
    							{
    								if(allUsers[t].sd == i)
    								{
    									//add name to correct channel
                                        strcpy(allChannels[x].people[allChannels[x].numberPeople], allUsers[t].name);
    									strcpy(personadded ,allUsers[t].name);
    									allChannels[x].numberPeople++;
                                       	allUsers[t].membersOf++;
    									//print message that we joined that channel
    									char message[40];
                                      	 memset(message, 0, 40);
                        				sprintf(message, "Joined Channel #%s", allChannels[x].name);
						      			message[strlen(message)] = '\0';
                          				//send message
                       					private_message_individual(allUsers[t].name, message, allUsers, totalClients, i);
									break;
    								}
    							}
    							//all people currently in the channel need a message that somone joined
    							char message2[40];
                                char chan_name[20];
                                char per_name[20];
                			    strcpy(per_name, personadded);
                               	per_name[strlen(per_name)-1] = '\0';
                		        strcpy(chan_name, allChannels[x].name);
                   				chan_name[strlen(chan_name) - 1] = '\0';
                                sprintf(message2,"#%s> %s joined the channel.\n", chan_name, per_name);
							     message2[strlen(message2)] = '\0';
    				    		private_message_channel(allChannels[x].name,message2, allUsers, allChannels, totalClients, totalChannels);
    						    //dont need to keep looking through channels bc found it
    						    break;
    						}
    					}
    					//Need to create a new channel
    					if(newChan)
    					{
    						struct Channel temp;
    						strcpy(temp.name,channel_name);
    						temp.numberPeople = 1;
                           			 //create space for new channel
    						temp.people = (char**) malloc(sizeof(char*) * 100);
    						for(int y=0; y<100; y++)
    						{
    							temp.people[y] = (char*) malloc(sizeof(char) * 20);
    						}
                           			 //add person to new channel
    						for(int t=0; t< totalClients; t++)
    						{
    							if(allUsers[t].sd == i)
    							{
    								strcpy(temp.people[0],allUsers[t].name);
                                    allUsers[t].membersOf++;
    								break;
    							}
    						}
    						allChannels[totalChannels] = temp;
    						totalChannels++;
    						send(i, "Created channel\n", 16, 0);
   						}
   					}
   				}
                //OPERATOR COMMAND
                else if(bytes_read >= 9 && buffer[0] == 'O' && buffer[1] == 'P' && buffer[2] == 'E' && buffer[3] == 'R'&& buffer[4] == 'A' && buffer[5] == 'T' && buffer[6] == 'O' && buffer[7] == 'R' && buffer[8] == ' ')
                {
                    //password user entered
                    char enteredPass[20];
                    memcpy(enteredPass, buffer+9, bytes_read-10);
                    enteredPass[bytes_read-10] = '\0';
                    //must have a password to compare against
                    if(argc == 2 && argv[1][0]=='-' && argv[1][1]=='-' && argv[1][2] == 'o' && argv[1][3] == 'p' && argv[1][4] == 't' && argv[1][5] == '-' && argv[1][6] == 'p' && argv[1][7] == 'a' && argv[1][8] == 's' && argv[1][9] == 's' && argv[1][10] == '=')
                    {
                        //password entered at runtime
                        char flag[31];
                        memcpy(flag, argv[1]+11, strlen(argv[1]) - 10);
                        //compare passwords
                        if(strcmp(flag, enteredPass) == 0)
                        {
                            //find out what person made command
                            char myName[20];
                            for(int h=0; h<totalClients;  h++)
                            {
                                if(allUsers[h].sd == i)
                                {
                                    strcpy(myName, allUsers[h].name);
                                    break;
                                }
                            }
                            strcpy(operators[numOPs],myName);
                            numOPs++;
                            //if correct give them op status
                            char* gotIT ="OPERATOR status bestowed.\n";
                            send(i, gotIT, strlen(gotIT), 0);
                        }
                        else
                        {
                            char* invalid = "Invalid Password\n";
                            send(i, invalid, strlen(invalid), 0);
                        }
                    }
                    //no operators allowed
                    else
                    {
                       char* noOPs = "NO operators permitted\n";
                       send(i, noOPs, strlen(noOPs),0);
                   }
               }
               //KICK COMMAND
                else if(bytes_read>= 5 && buffer[0] == 'K' && buffer[1] == 'I' && buffer[2] == 'C' && buffer[3] == 'K'&& buffer[4] == ' ')
                {
                    char info[41];
                    memcpy(info, buffer+5, bytes_read-4);
                    //are they an operator?
                    bool havePermission = false;
                    for(int h=0; h<numOPs;h++)
                    {
                        if(strcmp(operators[h], client_name) == 0)
                        {
                            havePermission = true;
                            break;
                        }
                    }
                    //if theyre an operator
                    if(havePermission)
                    {
                        if(info[0] == '#')
                        {
                            char relevant_channel[19];
                            char relevant_person[20];
                            const char space[2] = " ";
                            int count = 0;
                            //split up person from channel
                            for(int w=0; w<strlen(info); w++)
                            {
                                if(info[w] == ' ')
                                {
                                    break;
                                }
                                count++;
                            }
                            strncpy(relevant_channel, info+1, count-1);
                            strncpy(relevant_person, info + count + 1, strlen(info) - (count+2));
			                relevant_channel[count-1] = '\0';
                            strcat(relevant_channel, "\n");
			                relevant_channel[count] = '\0';
			                relevant_person[strlen(info)-(count+2)] = '\0';
                            strcat(relevant_person, "\n");
                            bool foundChannel = false;
                            bool foundPerson = false;
                           //go through all channnels
                            for(int g=0;g<totalChannels;g++)
                            {
                                if(strcmp(allChannels[g].name, relevant_channel) == 0)
                                {
                                    foundChannel = true;
                                    //go through all people in relevant channel
                                    for(int y=0; y<allChannels[g].numberPeople;y++)
                                    {
                                        if(strcmp(relevant_person, allChannels[g].people[y]) == 0)
                                        {
                                            foundPerson =true;
                                           //notify everyone about someone being booted                                              
                                            char message3[60];
                                            relevant_channel[strlen(relevant_channel)-1] = '\0';
                                            relevant_person[strlen(relevant_person) - 1] = '\0';
                                            sprintf(message3, "#%s> %s has been kicked from the channel.\n", relevant_channel, relevant_person);
                                            private_message_channel(allChannels[g].name, message3, allUsers, allChannels, totalClients, totalChannels);
                                            message3[0] = '\0';
                                            //remove the kicked person from channel
                                            for(int next = y; next<allChannels[g].numberPeople-1; next++)
                                            {
                                                allChannels[g].people[next] = allChannels[g].people[next+1];
                                            }
                                            allChannels[g].numberPeople--;
                                            break;
                                        }
                                    }
                                    //the person is not in the channel to kick
                                    if(!foundPerson)
                                    {
                                        char* notIN = "Person listed not in channel\n";
                                        send(i,notIN,strlen(notIN),0);
                                        break;
                                    }
                                }
                            }
                            if(!foundChannel){
                                char* notIN = "Invalid channel name\n";
                                send(i,notIN,strlen(notIN),0);
                                break;
                            }
                        }
                        else{
                            send(i,"Incorrect format\n",17,0);
                        }
                    }
                    else{
                        send(i,"You do not have permission to kick someone\n", 43, 0);                        }
                    }
                    //QUIT COMMAND
                    else if(buffer[0] == 'Q' && buffer[1] == 'U' && buffer[2] == 'I' && buffer[3] == 'T')
                    {
                        //client closed connection
                        char myName[20];
                        for(int j=0; j<totalClients;j++)
                        {
                            if(allUsers[j].sd == i)
                            {
                                strcpy(myName, allUsers[j].name);
                              break;
                            }
                        }                    
                        //remove the closed client from all channels
                        for(int k=0; k<totalChannels; k++)
                        {
                            char chan_name[20];
                            strcpy(chan_name, allChannels[k].name);
                            for(int f=0; f<allChannels[k].numberPeople; f++)
                            {
                                if(strcmp(allChannels[k].people[f], myName) == 0)
                               {
                                    //tell everyone in channel that someone left
                                    char message3[60];
                                    chan_name[strlen(chan_name) - 1] = '\0';
                                    myName[strlen(myName) - 1] = '\0';
                                    sprintf(message3, "#%s> %s left the channel.\n", chan_name, myName);
                                    private_message_channel(allChannels[k].name, message3, allUsers, allChannels, totalClients, totalChannels);
                                    message3[0] = '\0';
                                    //remove him/her
                                    for(int next = k; k<allChannels[k].numberPeople-1;k++)
                                    {
                                        allChannels[k].people[k] = allChannels[k].people[k+1];
                                    }
                                    allChannels[k].numberPeople--;
                                    break;                                       
                                }
                             }
                        }
                        //delete the closed persons person variable
                       for(int j=0; j<totalClients;j++)
                        {
                            if(allUsers[j].sd == i)
                            {
                                close(allUsers[j].sd);
                                FD_CLR(allUsers[j].sd, &full_set);
                                for(int next = j; next<totalClients-1;next++)
                                {
                                    allUsers[next] = allUsers[next+1];
                                }
                                break;
                            }
                        }                    
                    }
                    //PART COMMAND
                    else if(bytes_read>-4 && buffer[0] == 'P' && buffer[1] == 'A' && buffer[2] == 'R' && buffer[3] == 'T')
                    {
                        //find out what person made command
                        char myName[20];
                        for(int h=0; h<totalClients;  h++)
                        {
                            if(allUsers[h].sd == i)
                            {
                                strcpy(myName, allUsers[h].name);
                                break;
                            }
                        }
                        //leaving 1 channel or all?
                        if(buffer[4] == ' ' && buffer[5] == '#')
                        {
                            char chan_name[20];
                            memcpy(chan_name, buffer +6, bytes_read-5);
                            chan_name[strlen(chan_name)] = '\0';
                            bool foundInChan = false;
                            for(int k=0; k<totalChannels; k++)
                            {
                                if(strcmp(allChannels[k].name, chan_name) == 0)
                                {
                                    //found channel want to leave
                                    char** amIhere = allChannels[k].people;
                                    for(int f=0; f<allChannels[k].numberPeople; f++)
                                    {
                                        if(strcmp(amIhere[f], myName) == 0)
                                        {
                                            foundInChan = true;
                                            //notify everyone about someone leaving
                                            char message3[60];
                                            chan_name[strlen(chan_name) - 1] = '\0';
                                            myName[strlen(myName) - 1] = '\0';
                                            sprintf(message3, "#%s> %s left the channel.\n", chan_name, myName);
                                            private_message_channel(allChannels[k].name, message3, allUsers, allChannels, totalClients, totalChannels);
                                            message3[0] = '\0';
                                            //remove him
                                            for(int next = k; k<allChannels[k].numberPeople-1;k++)
                                            {
                                                amIhere[k] = amIhere[k+1];
                                            }
                                            allChannels[k].numberPeople--;
                                            break;
                                        }
                                    }
                                }
                            }
                            if(!foundInChan)
                            {
                                char mess[50];
                                char temp[20];
                                memcpy(temp, chan_name, strlen(chan_name)-1);
                                sprintf(mess, "You are not currently in #%s\n", temp);
                                send(i, mess, strlen(mess),0);
                            }

                        }
                        //part all cahnnels
                        else if(bytes_read == 5)
                        {
                            for(int k=0; k<totalChannels; k++)
                            {
                                char** amIhere = allChannels[k].people;
                                char chan_name[20];
                                strcpy(chan_name, allChannels[k].name);
                                for(int f=0; f<allChannels[k].numberPeople; f++)
                                {
                                    if(strcmp(amIhere[f], myName) == 0)
                                    {
                                        char message3[60];
                                        chan_name[strlen(chan_name) - 1] = '\0';
                                        myName[strlen(myName) - 1] = '\0';
                                        sprintf(message3, "#%s> %s left the channel.\n", chan_name, myName);
                                        private_message_channel(allChannels[k].name, message3, allUsers, allChannels, totalClients, totalChannels);
                                        message3[0] = '\0';
                                        //remove him
                                        for(int next = k; k<allChannels[k].numberPeople-1;k++)
                                        {
                                            amIhere[k] = amIhere[k+1];
                                        }
                                        allChannels[k].numberPeople--;
                                        break;                                       
                                    }
                                }
                            }

                        }
                        else
                        {
                             send(i, "Invalid command\n", 16,0);

                        }

                    }
                    //PRIVMSG COMMAND
                   else if(bytes_read>= 8 && buffer[0] == 'P' && buffer[1] == 'R' && buffer[2] == 'I' && buffer[3] == 'V'&& buffer[4] == 'M' && buffer[5] == 'S' && buffer[6] == 'G' && buffer[7] == ' ')
                    {
                        char creator[20];
                        char creator2[20];               
                        char text[512];
                        char totalMessage[550];
                        //find the person wants to send messaages name
                        for(int t=0; t< totalClients; t++)
    		            {
    			            if(allUsers[t].sd == i)
    		                {
                                strcpy(creator,allUsers[t].name);
                                break;
			                }
                        }    	
                        strncpy(creator2, creator, strlen(creator) -1);	   
                        creator2[strlen(creator)-1] = '\0';
                        //sending message to a channel
                        if(buffer[8] == '#')
                        {
                            char info2[530];
                            char chan_name[19];
                            strncpy(info2, buffer + 9, bytes_read-8);
                            info2[bytes_read-8] = '\0';
                            int count = 0;
                            //split up channel name and message
                            for(int w=0; w<strlen(info2); w++)
                            {
                                if(info2[w] == ' ')
                                {
                                    break;
                                }
                                count++;
                            }
                            //format channel name
                            memcpy(chan_name, info2, count);
                            char chan_name2[20];
                            strcpy(chan_name2, chan_name);
				            chan_name[strlen(chan_name)] = '\0';
                            chan_name2[strlen(chan_name)] = '\0';
                            bool inChannel = true;
                            //form the total message with correct \n characters
                            if(inChannel)
                            {
                                memcpy(text, info2+count+1, bytes_read-8-strlen(chan_name2));
                                text[strlen(text)] = '\0';
                                sprintf(totalMessage, "#%s> %s: %s", chan_name2, creator2, text);
                                totalMessage[strlen(totalMessage)] = '\0';
                                strcat(chan_name2, "\n");
                                //send message
                                private_message_channel(chan_name2, totalMessage, allUsers, allChannels, totalClients, totalChannels);   
                            }
                            else
                            {
                                send(i, "You are not a member of this channel\n",37, 0);
                            }

                        }
                        //send message 1 individual
                        else
                        {
                            char info2[530];
                            char recp[20];
                            strncpy(info2, buffer +8, bytes_read-8);
                            info2[bytes_read-8] = '\0';
                            int count = 0;
                            //split name and message
                            for(int w=0; w<strlen(info2); w++)
                            {
                                if(info2[w] == ' ')
                                {
                                    break;
                                }
                                count++;
                            }
                            //buil message
                            strncpy(recp, info2, count);
                            recp[count] = '\0';
                            memcpy(text, info2+count+1, bytes_read-8-strlen(recp));
                            text[strlen(text)] = '\0';
                            sprintf(totalMessage, "%s> %s",creator2, text);
                            totalMessage[strlen(totalMessage)] = '\0';
                            strcat(recp, "\n");
                            //send message
                            private_message_individual(recp, totalMessage, allUsers, totalClients,i);
                        }
                    }
                    else
                    {
                        send(i, "Invalid command\n", 17,0);
                    }

    			}
    	}
	}
}
