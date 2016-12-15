# armadill
![logo](https://github.com/santomet/armadill/blob/master/docs/logo.png?raw=true)

What it is:
A school project - secure P2P IM with server as an authority. Well, not really secure. No deep testing, or fixes since we passed the subject.
How it works:
Basically, we tried to remake a (very) simplified version of Open Whisper Systems' axolotl protocol (see the docs)
What can i use it for?
If you have similar assingment at school, or you are trying to make client-server application in Qt, this can help

Known issues:
-s*itload of forgotten ones
-it does not work if client-to-client communication is going through NAT
-other ones we decided to call features

Easy to use:
1. clone
2. qmake (Qt 5.6)
3. make
4. replace cert.crt and cert.key in folder with server binary with yours (or keep the testing ones)
5. have fun and don't forget, don't trust any software at 100%, especially not this one ;)
