# MySQL User Defined Functions for MaxMind GeoIP API
#
# Copyright (c) 2010, Anchor Intelligence. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the
#   distribution.
#
# - Neither the name of Anchor Intelligence nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

APP	= udf_geoip.so

OS  = $(shell uname)
CC	= gcc
CFLAGS	= -I/usr/local/include -I/usr/local/mysql/include -I/usr/include/mysql -fPIC -g -O2 -Wall -Werror

ifeq ($(OS),Darwin)
    SHARED = -dynamiclib 
	LDLIBS = -lGeoIP -lstdc++
else
    SHARED = -shared
	LDLIBS = -lGeoIP
endif

CSRC	= $(wildcard *.c)
OBJS	= $(CSRC:.c=.o)

all: $(APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $(SHARED) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -MD -c $(CFLAGS) $< -o $@

clean:
	rm -f $(APP) $(OBJS) $(wildcard *.d)

install: $(APP)
	sudo rm -f /usr/lib/$(APP)
	sudo cp -p $(APP) /usr/lib
	mysql -v -v -v mysql < udf_geoip.sql

-include $(wildcard *.d)
