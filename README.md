# CRI-Server
Chatting Remotely over the Internet Server

For my chat server, a user must identify themselves before they can do any other commands. The connection will be closed between the client if they do not first enter the USER command. The JOIN command allows users to join a channel, they will only recieve messages that they are virtual present for. The LIST command displays all channels or all people in a specified channel. The OPERATOR command allows clients to become operator with the correct password which in turn allows them to kick people from channels. The kick command removes the named individual from the named channel and notifies everyone in hte channel that someone was kicked. The part command allows users to leave a specified channel or leave all channels if no argument is specified. The PRIVMSG command allows users to send messages to an entire channel or to a specified user. The QUIT command removes user from all channels and notifies everyone in those channels. 
