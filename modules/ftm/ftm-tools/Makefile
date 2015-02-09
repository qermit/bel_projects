DESTPATH = .
FWPATH   = ../ftmfw
TESTPATH = ../ftmverification
TOOLPATH = ../ftmx86
LN       = ln -s 
  
all:	tool firmware testfw
	echo off
	#if test -d $(DESTPATH); \
     	#then echo exists; \
     	#else mkdir $(DESTPATH); \
     	#fi	
	$(LN) $(TOOLPATH)/ftm-ctl $(DESTPATH)/ftm-ctl
	$(LN) $(TOOLPATH)/heartbeat.xml   $(DESTPATH)/heartbeat.xml
	$(LN) $(TOOLPATH)/sourceLinac.xml   $(DESTPATH)/sourceLinac.xml
	$(LN) $(TOOLPATH)/ring.xml   $(DESTPATH)/ring.xml
	cp    $(TOOLPATH)/*manual*.txt   $(DESTPATH)
	$(LN) $(TESTPATH)/verifyftm.bin $(DESTPATH)/ftm-ver.bin 
	$(LN) $(TESTPATH)/load-verfw $(DESTPATH)/load-verfw
	$(LN) $(TESTPATH)/eca-tm-pex-dbg $(DESTPATH)/eca-tm-dbg 
	$(LN) $(FWPATH)/ftm.bin $(DESTPATH)/ftm.bin
	echo
	echo All files are ready in $(DESTPATH) 

tool:
	$(MAKE) -C $(TOOLPATH)
firmware:
	$(MAKE) -C $(FWPATH)
testfw:
	$(MAKE) -C $(TESTPATH)
         
clean:
	$(MAKE) clean -C $(TOOLPATH)
	$(MAKE) clean -C $(FWPATH)
	$(MAKE) clean -C $(TESTPATH)
	rm -f $(DESTPATH)/*.o $(DESTPATH)/*.elf $(DESTPATH)/*.bin $(DESTPATH)/ftm-ctl
	rm -f $(DESTPATH)/heartbeat.xml $(DESTPATH)/sourceLinac.xml $(DESTPATH)/ring.xml
	rm -f $(DESTPATH)/load-verfw $(DESTPATH)/eca-tm-dbg
	rm -f $(DESTPATH)/*manual*.txt
