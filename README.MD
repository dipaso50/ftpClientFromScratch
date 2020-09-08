# Simple FTP Client
 
Very simple FTP client with just a few commands.

## Usage 

```
$git clone https://github.com/dipaso50/ftpClientFromScratch.git
$cd ftpClientFromScratch/
$make 
$./ftpclient serverip port

220 (vsFTPd 3.0.3)

>>>USER anonymous
230 Login successful.

>>>HELP
*********************************************************************
LIST [remote directory] - List directory, default current.
RETR remoteFileName [localFileName] - Get remote file and storein current local directory, for rename file define localFileName.
CWD directoryName - Change working directory.
CDUP - Change to parent directory.
QUIT - Close connection and exit.
HELP - Print available commands information.
*********************************************************************
>>>

```

## Licence

MIT.

 