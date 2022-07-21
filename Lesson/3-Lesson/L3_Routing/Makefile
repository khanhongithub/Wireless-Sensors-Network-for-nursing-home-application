CONTIKI_PROJECT = first_app
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += routing.c
PROJECT_SOURCEFILES += buffer.c

CONTIKI_WITH_RIME = 1
CONTIKI = $(HOME)/contiki

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.include
