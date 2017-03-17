#阶段四
CPPFLAGS= -Iinclude
CFLAGS= -g -Wall
LDFLAGS= 
CC=gcc
#CC=arm-linux-gcc

src = $(wildcard *.c)
obj = $(patsubst %.c,%.o,$(src))
target = app

$(target):$(obj)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

.PHONY:clean

#彻底清除生生过程文件
clean:
	-rm -f *.o
	-rm -f app

#彻底清除生生过程文件和生成配置文件
distclean:
	rm /usr/bin/app
install:
	cp app  /usr/bin


test:
	@echo $(src)
	@echo $(obj)
