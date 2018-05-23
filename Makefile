
SRC_DIR	= .
OBJ_DIR	= ./OBJS

SRCS	= $(SRC_DIR)/linkstat.c $(SRC_DIR)/version.c

OBJS	= $(OBJ_DIR)/linkstat.o $(OBJ_DIR)/version.o

WALL	= -W -Wreturn-type -Wunused -Wswitch -Wcomment -Wtrigraphs -pedantic
CC	= gcc
CFLAGS	= -g $(DEFS)
DEFS	= -DUNAME="\"`uname -srvm`\"" -DLONG_OPTIONS -DCHECK_MAC_ADDR
SHELL	= /bin/sh

#LINT	= lint -abchx
LINT	= splint -warnposixheaders -exitarg

PROFILE	= -pg

ECHO	= echo -e

all: linkstat

info:
	@$(ECHO) "COMPILER COMMAND LINE:"
	@$(ECHO) "$(CC) $(CFLAGS) $(INCL)"
	@$(ECHO) " "
	@$(ECHO) "INCLUDED LIBRARIES:"
	@$(ECHO) "$(LIBS)"

install: linkstat
	@cp linkstat /opt/LinkStat/bin
	@$(ECHO) "linkstat 	: installing    /opt/LinkStat/bin/linkstat"

warnings: $(SRCS)
	@make clean
	@$(ECHO) 'char datecompiled[] = "'`date`'";' >datecompiled.c
	@gcc $(WALL) $(CFLAGS) $(INCL) -o linkstat datecompiled.c $(SRCS) $(LIBS)
	@rm datecompiled.?

profile: $(SRCS)
	@make clean
	@$(ECHO) 'char datecompiled[] = "'`date`'";' >datecompiled.c
	@$(CC) $(CFLAGS) $(PROFILE) -O $(INCL) -o linkstat datecompiled.c $(SRCS) $(LIBS)
	@rm datecompiled.?

lint: $(SRCS)
	$(LINT) $(INCL) $(SRCS) | less

linkstat: $(OBJS)
	@$(ECHO) "linkstat 	: linking"
	@$(ECHO) 'char datecompiled[] = "'`date`'";' >datecompiled.c
	@$(CC) $(CFLAGS) $(INCL) -o $@ datecompiled.c $(OBJS) $(LIBS)
	@$(ECHO) " "
	@size $@; $(ECHO) " "
	@ls -l $@; $(ECHO) " "
	@strings linkstat | awk '/@\(#\)/ {sub(/@\(#\)/,"	"); print}'
	@rm datecompiled.?
	@$(ECHO) ".done." | tr . '\07'

$(OBJ_DIR)/linkstat.o: $(SRC_DIR)/linkstat.c $(SRC_DIR)/version.h
	@$(ECHO) "linkstat 	: compiling	C code       -->  object code"
	@$(CC) $(CFLAGS) $(INCL) -c $(SRC_DIR)/linkstat.c -o $(OBJ_DIR)/linkstat.o

$(OBJ_DIR)/version.o: $(SRC_DIR)/version.c $(SRC_DIR)/version.h
	@$(ECHO) "version		: compiling	C code       -->  object code"
	@$(CC) $(CFLAGS) $(INCL) -c $(SRC_DIR)/version.c -o $(OBJ_DIR)/version.o

clean:
	@/bin/rm -f mon.out $(OBJS) *~ core

