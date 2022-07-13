CONTIKI_PROJECT = mon_node
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += routing.c
PROJECT_SOURCEFILES += buffer.c
PROJECT_SOURCEFILES += helpers.c
PROJECT_SOURCEFILES += cluster_head.c
PROJECT_SOURCEFILES += cluster_member.c

CONTIKI_WITH_RIME = 1
CONTIKI = $(HOME)/contiki

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
