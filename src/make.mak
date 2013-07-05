#author: yangshengcheng@gzcss.net
#create: 20130425


CFLAGS=/W3 /nologo

SYS_LIB=advapi32.lib wsock32.lib ws2_32.lib User32.lib libssh2.lib Shlwapi.lib
OFILES=stat.obj getopt.obj sock.obj log.obj loadtask.obj util.obj ssh.obj exec.obj

all:		gather.exe

gather.exe:	$(OFILES) 
		link /out:gather.exe $(OFILES) $(SYS_LIB)

getopt.obj:	getopt.cpp
		cl $(CFLAGS) /c /MD getopt.cpp
		
stat.obj:	stat.cpp
		cl $(CFLAGS) /c /MD stat.cpp
		
sock.obj:	sock.cpp
		cl $(CFLAGS) /c /MD sock.cpp
		
log.obj:	log.cpp
		cl $(CFLAGS) /c /MD log.cpp
		
loadtask.obj:	loadtask.cpp
		cl $(CFLAGS) /c /MD loadtask.cpp
		
util.obj:	util.cpp
		cl $(CFLAGS) /c /MD util.cpp
		
ssh.obj:	ssh.cpp
		cl $(CFLAGS) /c /MD ssh.cpp
		
exec.obj:	exec.cpp
		cl $(CFLAGS) /c /MD exec.cpp		
				
		
clean:
		-del gather.exe
		-del *.obj
