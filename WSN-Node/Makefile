CONTIKI_PROJECT = mon_node
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += cluster_head.c
PROJECT_SOURCEFILES += cluster_member.c

CONTIKI_WITH_RIME = 1
CONTIKI = $(HOME)/contiki

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
